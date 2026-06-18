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

#include <stdexcept>

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

QCoro::Task<void> PlaybackController::play(const Song &song)
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

    // Load and play
    try {
        co_await m_backend->load(QUrl(url));
        m_backend->play();
    } catch (const std::exception &ex) {
        Q_EMIT errorOccurred(QString::fromUtf8(ex.what()));
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
    // Check if already cached and valid
    auto it = m_urlCacheExpiry.find(song.id);
    if (it != m_urlCacheExpiry.end() && it.value() > QDateTime::currentMSecsSinceEpoch()) {
        return; // Already cached and not expired
    }

    if (!m_plugin) {
        return; // No platform plugin available
    }

    // Background resolution with safe lifetime via QPointer
    auto task = [](QPointer<PlaybackController> self, Song s) -> QCoro::Task<void> {
        if (!self || !self->m_plugin) {
            co_return;
        }
        try {
            auto result = co_await self->m_plugin->getSongUrl(s.id);
            if (!self) {
                co_return;
            }
            if (result.isSuccess()) {
                self->m_urlCache.insert(s.id, result.data().url);
                self->m_urlCacheExpiry.insert(s.id, QDateTime::currentMSecsSinceEpoch() + URL_CACHE_TTL_MS);
            } else {
                Logger::get("player")->warn("Pre-resolve failed for {}: {}", s.name.toStdString(),
                                            result.error().message().toStdString());
            }
        } catch (const std::exception &ex) {
            Logger::get("player")->warn("Pre-resolve exception for {}: {}", s.name.toStdString(), ex.what());
        }
    }(QPointer<PlaybackController>(this), song);
    Q_UNUSED(task);
}

// --- Private ---

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

QCoro::Task<QString> PlaybackController::resolveUrl(const Song &song)
{
    // Check cache first
    auto cacheIt = m_urlCache.find(song.id);
    if (cacheIt != m_urlCache.end()) {
        auto expiryIt = m_urlCacheExpiry.find(song.id);
        if (expiryIt != m_urlCacheExpiry.end() && expiryIt.value() > QDateTime::currentMSecsSinceEpoch()) {
            co_return cacheIt.value();
        }
        // Expired — remove from cache
        m_urlCache.erase(cacheIt);
        m_urlCacheExpiry.erase(expiryIt);
    }

    // Resolve via platform plugin
    if (!m_plugin) {
        throw std::runtime_error("No platform plugin available for URL resolution");
    }

    auto result = co_await m_plugin->getSongUrl(song.id);
    if (!result.isSuccess()) {
        throw std::runtime_error(result.error().message().toStdString());
    }

    // Cache the result
    m_urlCache.insert(song.id, result.data().url);
    m_urlCacheExpiry.insert(song.id, QDateTime::currentMSecsSinceEpoch() + URL_CACHE_TTL_MS);

    co_return result.data().url;
}

} // namespace QeriPlayerQt
