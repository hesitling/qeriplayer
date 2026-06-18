/// @file PlayerViewModel.cpp
/// @brief Implementation of PlayerViewModel

#include "viewmodel/PlayerViewModel.h"

#include "core/logger/Logger.h"

namespace QeriPlayerQt {

PlayerViewModel::PlayerViewModel(PlaybackController *controller, IPlayHistoryRepository *historyRepo, QObject *parent)
    : QObject(parent)
    , m_controller(controller)
    , m_historyRepo(historyRepo)
    , m_queueModel(new SongListModel(this))
{
    connectControllerSignals();
    connectQueueSignals();
    updateQueueModel();
}

PlayerViewModel::~PlayerViewModel() = default;

// --- Getters ---

Song PlayerViewModel::currentSong() const
{
    return m_controller->currentSong();
}

PlaybackState PlayerViewModel::playbackState() const
{
    return m_controller->playbackState();
}

bool PlayerViewModel::isPlaying() const
{
    return m_controller->playbackState() == PlaybackState::Playing;
}

bool PlayerViewModel::isPaused() const
{
    return m_controller->playbackState() == PlaybackState::Paused;
}

bool PlayerViewModel::isLoading() const
{
    return m_controller->playbackState() == PlaybackState::Loading;
}

qint64 PlayerViewModel::positionMs() const
{
    return m_positionMs;
}

qint64 PlayerViewModel::durationMs() const
{
    return m_durationMs;
}

double PlayerViewModel::volume() const
{
    return m_controller->volume();
}

bool PlayerViewModel::isMuted() const
{
    return m_controller->isMuted();
}

RepeatMode PlayerViewModel::repeatMode() const
{
    return m_controller->queue()->repeatMode();
}

bool PlayerViewModel::isShuffleEnabled() const
{
    return m_controller->queue()->isShuffleEnabled();
}

SongListModel *PlayerViewModel::queue() const
{
    return m_queueModel;
}

bool PlayerViewModel::hasError() const
{
    return m_hasError;
}

ViewModelError PlayerViewModel::error() const
{
    return m_error;
}

// --- Playback control ---

QCoro::Task<void> PlayerViewModel::play(const Song &song)
{
    try {
        co_await m_controller->play(song);
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("play() failed: {}", ex.what());
    }
}

void PlayerViewModel::loadQueueAndPlay(const QVector<Song> &songs, int startIndex)
{
    m_controller->queue()->clear();
    for (const Song &song : songs) {
        m_controller->queue()->addSong(song);
    }
    m_controller->queue()->setCurrentIndex(startIndex);
    updateQueueModel();

    if (startIndex >= 0 && startIndex < songs.size()) {
        play(songs.at(startIndex));
    }
}

void PlayerViewModel::pause()
{
    m_controller->pause();
}

void PlayerViewModel::resume()
{
    m_controller->resume();
}

void PlayerViewModel::stop()
{
    m_controller->stop();
}

void PlayerViewModel::seek(qint64 positionMs)
{
    m_controller->seek(positionMs);
}

void PlayerViewModel::next()
{
    auto nextSong = m_controller->queue()->next();
    if (nextSong.has_value()) {
        // Fire and forget coroutine
        play(nextSong.value());
    }
}

void PlayerViewModel::prev()
{
    auto prevSong = m_controller->queue()->prev();
    if (prevSong.has_value()) {
        // Fire and forget coroutine
        play(prevSong.value());
    }
}

// --- Volume / Mute ---

void PlayerViewModel::setVolume(double normalized)
{
    m_controller->setVolume(normalized);
    Q_EMIT volumeChanged();
}

void PlayerViewModel::setMuted(bool muted)
{
    m_controller->setMuted(muted);
    Q_EMIT mutedChanged();
}

void PlayerViewModel::toggleMute()
{
    setMuted(!isMuted());
}

// --- Repeat / Shuffle ---

void PlayerViewModel::cycleRepeatMode()
{
    PlayQueue *q = m_controller->queue();
    switch (q->repeatMode()) {
        case RepeatMode::Off:
            q->setRepeatMode(RepeatMode::One);
            break;
        case RepeatMode::One:
            q->setRepeatMode(RepeatMode::All);
            break;
        case RepeatMode::All:
            q->setRepeatMode(RepeatMode::Off);
            break;
    }
    // Signal emitted via PlayQueue::repeatChanged connection
}

void PlayerViewModel::toggleShuffle()
{
    PlayQueue *q = m_controller->queue();
    q->setShuffleEnabled(!q->isShuffleEnabled());
    // Signal emitted via PlayQueue::shuffleChanged connection
}

// --- Queue management ---

void PlayerViewModel::addToQueue(const Song &song)
{
    m_controller->queue()->addSong(song);
    updateQueueModel();
}

void PlayerViewModel::removeFromQueue(int index)
{
    m_controller->queue()->removeAt(index);
    updateQueueModel();
}

void PlayerViewModel::moveInQueue(int from, int to)
{
    m_controller->queue()->moveSong(from, to);
    updateQueueModel();
}

void PlayerViewModel::clearQueue()
{
    m_controller->queue()->clear();
    updateQueueModel();
}

// --- Private ---

void PlayerViewModel::connectControllerSignals()
{
    connect(m_controller, &PlaybackController::currentSongChanged, this, [this](const Song &) {
        Q_EMIT currentSongChanged();

        // Record play history
        const Song &song = m_controller->currentSong();
        if (!song.id.isEmpty() && m_historyRepo) {
            try {
                m_historyRepo->record(song.id);
            } catch (const std::exception &ex) {
                Logger::get("viewmodel")->warn("Failed to record play history: {}", ex.what());
            }
        }
    });

    connect(m_controller, &PlaybackController::playbackStateChanged, this,
            [this](PlaybackState) { Q_EMIT playbackStateChanged(); });

    connect(m_controller, &PlaybackController::positionChanged, this, [this](qint64 positionMs) {
        m_positionMs = positionMs;
        Q_EMIT positionChanged();
    });

    connect(m_controller, &PlaybackController::durationChanged, this, [this](qint64 durationMs) {
        m_durationMs = durationMs;
        Q_EMIT durationChanged();
    });

    connect(m_controller, &PlaybackController::errorOccurred, this, [this](const QString &message) {
        m_error = ViewModelError(ViewModelError::ErrorType::Api, message);
        m_hasError = true;
        Q_EMIT errorChanged();
    });
}

void PlayerViewModel::connectQueueSignals()
{
    PlayQueue *q = m_controller->queue();

    connect(q, &PlayQueue::queueChanged, this, [this]() { updateQueueModel(); });

    connect(q, &PlayQueue::currentChanged, this, [this]() {
        // Update playing index in the model
        m_queueModel->setPlayingIndex(m_controller->queue()->currentIndex());
    });

    connect(q, &PlayQueue::repeatChanged, this, [this](RepeatMode) { Q_EMIT repeatModeChanged(); });

    connect(q, &PlayQueue::shuffleChanged, this, [this](bool) { Q_EMIT shuffleChanged(); });
}

void PlayerViewModel::updateQueueModel()
{
    m_queueModel->setSongs(m_controller->queue()->songs());
    m_queueModel->setPlayingIndex(m_controller->queue()->currentIndex());
}

} // namespace QeriPlayerQt
