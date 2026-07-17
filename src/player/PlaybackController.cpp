/// @file PlaybackController.cpp
/// @brief High-level orchestrator coordinating backend, queue, URL resolution, and persistence

#include "player/PlaybackController.h"

#include "core/logger/Logger.h"
#include "domain/SongUrlResult.h"

#include <QCoroTask>
#include <QDateTime>
#include <QDebug>
#include <QPointer>
#include <QTimer>

#include <algorithm>
#include <stdexcept>

namespace {

bool isPlayableRemoteUrl(const QString &value)
{
    if (value.trimmed().isEmpty()) {
        return false;
    }

    const QUrl url(value, QUrl::StrictMode);
    const QString scheme = url.scheme().toLower();
    return url.isValid() && !url.isRelative() && !url.host().isEmpty()
           && (scheme == QStringLiteral("http") || scheme == QStringLiteral("https"));
}

QString songUrlStatusMessage(const QeriPlayerQt::Song &song, const QeriPlayerQt::SongUrlResult &result)
{
    using Status = QeriPlayerQt::SongUrlResult::Status;

    if (!result.noticeMessage.trimmed().isEmpty()) {
        return result.noticeMessage.trimmed();
    }

    switch (result.status) {
        case Status::RequiresLogin:
            return QStringLiteral("Login required or track unavailable for: %1").arg(song.name);
        case Status::WaitingForAuthoritativeStream:
            return QStringLiteral("Playback stream is not ready yet for: %1").arg(song.name);
        case Status::Failure:
            return QStringLiteral("Failed to resolve playback URL for: %1").arg(song.name);
        case Status::Success:
            break;
    }

    return QStringLiteral("Failed to resolve playback URL for: %1").arg(song.name);
}

} // namespace

namespace QeriPlayerQt {

PlaybackController::PlaybackController(std::unique_ptr<IPlayerBackend> backend, IMusicPlatformPlugin *plugin,
                                       IPlayerStateRepository *playerStateRepo, ISettingsRepository *settingsRepo,
                                       QObject *parent)
    : QObject(parent)
    , m_backend(std::move(backend))
    , m_plugin(plugin)
    , m_playerStateRepo(playerStateRepo)
    , m_settingsRepo(settingsRepo)
    , m_queue(new PlayQueue(this))
    , m_seekSaveTimer(new QTimer(this))
{
    m_seekSaveTimer->setSingleShot(true);
    m_seekSaveTimer->setInterval(500);

    connectBackendSignals();
    connectQueueSignals();

    // Restore volume/mute from settings
    if (m_settingsRepo) {
        auto volStr = m_settingsRepo->get(QStringLiteral("player/volume"));
        if (volStr.has_value()) {
            bool ok = false;
            double vol = volStr.value().toDouble(&ok);
            if (ok) {
                m_backend->setVolume(qBound(0.0, vol, 1.0));
            }
        }

        bool muted = m_settingsRepo->getBool(QStringLiteral("player/muted"), false);
        m_backend->setMuted(muted);
    }

    // Debounced seek save
    connect(m_seekSaveTimer, &QTimer::timeout, this, &PlaybackController::persistState);
}

PlaybackController::~PlaybackController() = default;

// --- Playback control ---

QCoro::Task<void> PlaybackController::play(Song song)
{
    m_currentSong = song;
    Q_EMIT currentSongChanged(song);

    // If song already has a mediaUri, use it directly
    if (song.mediaUri.isValid()) {
        try {
            co_await m_backend->load(song.mediaUri);
            m_backend->play();
        } catch (const std::exception &ex) {
            Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
        }
        co_return;
    }

    // Resolve URL from cache or platform plugin
    QString url;
    try {
        url = co_await resolveUrl(song);
    } catch (const std::exception &ex) {
        Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
        co_return;
    }

    if (url.isEmpty()) {
        Q_EMIT errorOccurred(QStringLiteral("Failed to resolve playback URL for: %1").arg(song.name));
        co_return;
    }

    // Load and play. A CDN URL can be revoked before its advertised expiry,
    // so evict and resolve once more if loading the cached/fresh URL fails.
    QString firstLoadError;
    try {
        co_await m_backend->load(QUrl(url));
        m_backend->play();
        co_return;
    } catch (const std::exception &ex) {
        firstLoadError = QString::fromUtf8(ex.what());
    }

    Logger::get("player")->warn("Playback URL failed for {}, refreshing once: {}", song.name.toStdString(),
                                firstLoadError.toStdString());
    evictCachedUrl(song);
    try {
        url = co_await resolveUrl(song, true);
        co_await m_backend->load(QUrl(url));
        m_backend->play();
    } catch (const std::exception &retryError) {
        Q_EMIT errorOccurred(QString::fromUtf8(retryError.what()));
    }
}

void PlaybackController::pause()
{
    m_backend->pause();
}

void PlaybackController::resume()
{
    m_backend->play();
}

void PlaybackController::stop()
{
    m_backend->stop();
}

void PlaybackController::seek(qint64 positionMs)
{
    m_backend->seek(positionMs);
    // Debounced save for seek operations
    m_seekSaveTimer->start();
}

// --- Queue access ---

PlayQueue *PlaybackController::queue()
{
    return m_queue;
}

// --- Volume ---

void PlaybackController::setVolume(double normalized)
{
    m_backend->setVolume(qBound(0.0, normalized, 1.0));
    if (m_settingsRepo) {
        try {
            m_settingsRepo->set(QStringLiteral("player/volume"), QString::number(m_backend->volume(), 'f', 2));
        } catch (const std::exception &ex) {
            Logger::get("player")->warn("Failed to persist volume setting: {}", ex.what());
        }
    }
}

double PlaybackController::volume() const
{
    return m_backend->volume();
}

void PlaybackController::setMuted(bool muted)
{
    m_backend->setMuted(muted);
    if (m_settingsRepo) {
        try {
            m_settingsRepo->set(QStringLiteral("player/muted"),
                                muted ? QStringLiteral("true") : QStringLiteral("false"));
        } catch (const std::exception &ex) {
            Logger::get("player")->warn("Failed to persist muted setting: {}", ex.what());
        }
    }
}

bool PlaybackController::isMuted() const
{
    return m_backend->isMuted();
}

// --- State queries ---

PlaybackState PlaybackController::playbackState() const
{
    return m_state;
}

const Song &PlaybackController::currentSong() const
{
    return m_currentSong;
}

QString PlaybackController::backendName() const
{
    return m_backend->backendName();
}

// --- Pre-resolve ---

void PlaybackController::preResolveUrl(const Song &song)
{
    const QString key = songCacheKey(song);
    const auto cacheIt = m_urlCache.constFind(key);
    if (cacheIt != m_urlCache.cend() && cacheIt->expiresAtMs > QDateTime::currentMSecsSinceEpoch()
        && isPlayableRemoteUrl(cacheIt->url)) {
        return;
    }

    if (!m_plugin || m_preResolveInFlight.contains(key)) {
        return;
    }

    pruneCompletedPreResolveTasks();

    constexpr qsizetype MAX_BACKGROUND_TASKS = 32;
    if (m_preResolveTasks.size() >= MAX_BACKGROUND_TASKS) {
        return;
    }

    m_preResolveInFlight.insert(key);

    // Background resolution with safe lifetime via QPointer.
    // Keep the task object alive; destroying a suspended QCoro::Task can corrupt
    // the coroutine frame that resumes when the network request finishes.
    auto task = [](QPointer<PlaybackController> self, Song s, QString cacheKey) -> QCoro::Task<void> {
        if (!self || !self->m_plugin) {
            co_return;
        }
        try {
            auto result = co_await self->m_plugin->getSongUrl(s.id);
            if (!self) {
                co_return;
            }
            if (result.isSuccess()) {
                const auto &songUrl = result.data();
                if (songUrl.status == SongUrlResult::Status::Success && isPlayableRemoteUrl(songUrl.url)) {
                    self->cacheResolvedUrl(s, songUrl);
                } else {
                    Logger::get("player")->warn("Pre-resolve returned non-playable URL for {}: {}",
                                                s.name.toStdString(), songUrlStatusMessage(s, songUrl).toStdString());
                }
            } else {
                Logger::get("player")->warn("Pre-resolve failed for {}: {}", s.name.toStdString(),
                                            result.error().message().toStdString());
            }
        } catch (const std::exception &ex) {
            if (self) {
                Logger::get("player")->warn("Pre-resolve exception for {}: {}", s.name.toStdString(), ex.what());
            }
        }
        if (self) {
            self->m_preResolveInFlight.remove(cacheKey);
        }
    }(QPointer<PlaybackController>(this), song, key);
    m_preResolveTasks.push_back(std::move(task));
}

// --- Private ---

void PlaybackController::pruneCompletedPreResolveTasks()
{
    auto isCompleted = [](const QCoro::Task<void> &task) { return task.isReady(); };
    m_preResolveTasks.erase(std::remove_if(m_preResolveTasks.begin(), m_preResolveTasks.end(), isCompleted),
                            m_preResolveTasks.end());
}

void PlaybackController::connectBackendSignals()
{
    connect(m_backend.get(), &IPlayerBackend::stateChanged, this, [this](PlaybackState newState) {
        m_state = newState;
        Q_EMIT playbackStateChanged(newState);

        if (newState == PlaybackState::Stopped || newState == PlaybackState::Paused) {
            persistState();
        }
    });

    connect(m_backend.get(), &IPlayerBackend::positionChanged, this, &PlaybackController::positionChanged);
    connect(m_backend.get(), &IPlayerBackend::durationChanged, this, &PlaybackController::durationChanged);

    connect(m_backend.get(), &IPlayerBackend::mediaFinished, this, [this]() {
        // Auto-advance to next song (stored to prevent premature destruction)
        auto nextSong = m_queue->next();
        if (nextSong.has_value()) {
            m_autoAdvanceTask = play(nextSong.value());
        } else {
            m_state = PlaybackState::Stopped;
            Q_EMIT playbackStateChanged(PlaybackState::Stopped);
            Q_EMIT playbackFinished();
            persistState();
        }
    });

    connect(m_backend.get(), &IPlayerBackend::errorOccurred, this, [this](const QString &msg) {
        m_state = PlaybackState::Error;
        Q_EMIT playbackStateChanged(PlaybackState::Error);
        Q_EMIT errorOccurred(msg);
    });
}

void PlaybackController::connectQueueSignals()
{
    connect(m_queue, &PlayQueue::currentChanged, this, [this]() {
        auto song = m_queue->currentSong();
        if (song.has_value()) {
            m_currentSong = song.value();
            Q_EMIT currentSongChanged(song.value());
        }
    });

    // Pre-resolve URLs when songs are added to the queue
    connect(m_queue, &PlayQueue::queueChanged, this, [this]() {
        // Pre-resolve the next few songs
        const auto &songs = m_queue->songs();
        int currentIdx = m_queue->currentIndex();
        for (int i = 0; i < 3 && currentIdx + i + 1 < songs.size(); ++i) {
            preResolveUrl(songs.at(currentIdx + i + 1));
        }
    });
}

void PlaybackController::persistState()
{
    if (!m_playerStateRepo) {
        return;
    }

    try {
        PersistedPlayerState state = m_queue->toPersistedState();
        state.positionMs = m_backend->positionMs();
        state.shouldResumePlayback = (m_state == PlaybackState::Playing || m_state == PlaybackState::Paused);
        m_playerStateRepo->save(state);
    } catch (const std::exception &ex) {
        Logger::get("player")->warn("Failed to persist player state: {}", ex.what());
    }
}

void PlaybackController::restoreState()
{
    m_restoreState = [](QPointer<PlaybackController> self) -> QCoro::Task<void> {
        if (!self || !self->m_playerStateRepo) {
            co_return;
        }

        auto savedState = self->m_playerStateRepo->load();
        if (!savedState.has_value() || !savedState->shouldResumePlayback) {
            co_return;
        }

        if (savedState->playlist.isEmpty()) {
            co_return;
        }

        if (!self) {
            co_return;
        }
        self->m_queue->loadFromState(savedState.value());

        auto currentSongOpt = self->m_queue->currentSong();
        if (!currentSongOpt.has_value()) {
            co_return;
        }

        if (!self) {
            co_return;
        }
        self->m_currentSong = currentSongOpt.value();
        Q_EMIT self->currentSongChanged(self->m_currentSong);

        // Restore playback position
        try {
            if (self->m_currentSong.mediaUri.isValid()) {
                co_await self->m_backend->load(self->m_currentSong.mediaUri);
            } else {
                QString url = co_await self->resolveUrl(self->m_currentSong);
                if (!self) {
                    co_return;
                }
                if (!url.isEmpty()) {
                    co_await self->m_backend->load(QUrl(url));
                }
            }
            if (self && savedState->positionMs > 0) {
                self->m_backend->seek(savedState->positionMs);
            }
        } catch (const std::exception &ex) {
            if (self) {
                Logger::get("player")->warn("Failed to restore playback state: {}", ex.what());
            }
        }
    }(QPointer<PlaybackController>(this));
}

void PlaybackController::cacheResolvedUrl(const Song &song, const SongUrlResult &result)
{
    const qint64 serverTtlMs = result.expiresInMs > 0 ? result.expiresInMs : DEFAULT_URL_CACHE_TTL_MS;
    const qint64 usableTtlMs = std::max<qint64>(1000, serverTtlMs - URL_EXPIRY_SAFETY_MARGIN_MS);
    m_urlCache.insert(songCacheKey(song),
                      CachedUrl {result.url.trimmed(), QDateTime::currentMSecsSinceEpoch() + usableTtlMs});
}

void PlaybackController::evictCachedUrl(const Song &song)
{
    m_urlCache.remove(songCacheKey(song));
}

QString PlaybackController::songCacheKey(const Song &song) const
{
    return QStringLiteral("%1:%2").arg(static_cast<int>(song.platform)).arg(song.id);
}

QCoro::Task<QString> PlaybackController::resolveUrl(Song song, bool forceRefresh)
{
    const QString key = songCacheKey(song);
    if (!forceRefresh) {
        auto cacheIt = m_urlCache.find(key);
        if (cacheIt != m_urlCache.end()) {
            const qint64 now = QDateTime::currentMSecsSinceEpoch();
            if (cacheIt->expiresAtMs > now && isPlayableRemoteUrl(cacheIt->url)) {
                Logger::get("player")->debug("Using cached playback URL for {} (remainingMs={})",
                                             song.name.toStdString(), cacheIt->expiresAtMs - now);
                const QString cachedUrl = cacheIt->url;
                co_return cachedUrl;
            }

            const char *reason = cacheIt->expiresAtMs <= now ? "expired" : "invalid";
            Logger::get("player")->warn("Discarding {} cached playback URL for {}: {}", reason, song.name.toStdString(),
                                        cacheIt->url.toStdString());
            m_urlCache.erase(cacheIt);
        }
    } else {
        evictCachedUrl(song);
    }

    if (!m_plugin) {
        throw std::runtime_error("No platform plugin available for URL resolution");
    }

    auto result = co_await m_plugin->getSongUrl(song.id);
    if (!result.isSuccess()) {
        throw std::runtime_error(result.error().message().toStdString());
    }

    const auto &songUrl = result.data();
    if (songUrl.status != SongUrlResult::Status::Success) {
        throw std::runtime_error(songUrlStatusMessage(song, songUrl).toStdString());
    }
    if (!isPlayableRemoteUrl(songUrl.url)) {
        throw std::runtime_error(
            QStringLiteral("Platform returned an invalid playback URL for: %1").arg(song.name).toStdString());
    }

    cacheResolvedUrl(song, songUrl);
    co_return songUrl.url.trimmed();
}

} // namespace QeriPlayerQt
