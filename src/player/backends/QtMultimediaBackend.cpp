/// @file QtMultimediaBackend.cpp
/// @brief Qt6 Multimedia implementation of IPlayerBackend

#include "player/backends/QtMultimediaBackend.h"

#include <QCoroSignal>

#include <QDebug>
#include <stdexcept>

namespace NeriPlayerQt {

QtMultimediaBackend::QtMultimediaBackend(QObject *parent)
    : IPlayerBackend(parent)
    , m_player(std::make_unique<QMediaPlayer>())
    , m_audioOutput(std::make_unique<QAudioOutput>())
{
    m_player->setAudioOutput(m_audioOutput.get());
    connectSignals();
}

QtMultimediaBackend::~QtMultimediaBackend() = default;

QCoro::Task<void> QtMultimediaBackend::load(const QUrl &url)
{
    m_player->setSource(url);

    // Check if already loaded (signal won't fire again)
    auto currentStatus = m_player->mediaStatus();
    if (currentStatus == QMediaPlayer::LoadedMedia || currentStatus == QMediaPlayer::BufferedMedia) {
        co_return;
    }
    if (currentStatus == QMediaPlayer::InvalidMedia) {
        throw std::runtime_error(QStringLiteral("Failed to load media: %1").arg(m_player->errorString()).toStdString());
    }

    // Wait for the media to reach a terminal status with timeout
    constexpr auto kLoadTimeout = std::chrono::seconds(30);
    while (true) {
        auto status = co_await qCoro(m_player.get(), &QMediaPlayer::mediaStatusChanged, kLoadTimeout);
        if (!status.has_value()) {
            throw std::runtime_error("Timed out waiting for media to load");
        }
        if (*status == QMediaPlayer::LoadedMedia || *status == QMediaPlayer::BufferedMedia) {
            break;
        }
        if (*status == QMediaPlayer::InvalidMedia) {
            throw std::runtime_error(
                QStringLiteral("Failed to load media: %1").arg(m_player->errorString()).toStdString());
        }
    }
}

void QtMultimediaBackend::play()
{
    m_player->play();
}

void QtMultimediaBackend::pause()
{
    m_player->pause();
}

void QtMultimediaBackend::stop()
{
    m_player->stop();
}

void QtMultimediaBackend::seek(qint64 positionMs)
{
    m_player->setPosition(positionMs);
}

PlaybackState QtMultimediaBackend::state() const
{
    return m_cachedState;
}

qint64 QtMultimediaBackend::positionMs() const
{
    return m_player->position();
}

qint64 QtMultimediaBackend::durationMs() const
{
    return m_player->duration();
}

bool QtMultimediaBackend::isSeekable() const
{
    return m_player->isSeekable();
}

void QtMultimediaBackend::setVolume(double normalized)
{
    m_audioOutput->setVolume(static_cast<float>(normalized));
}

double QtMultimediaBackend::volume() const
{
    return static_cast<double>(m_audioOutput->volume());
}

void QtMultimediaBackend::setMuted(bool muted)
{
    m_audioOutput->setMuted(muted);
}

bool QtMultimediaBackend::isMuted() const
{
    return m_audioOutput->isMuted();
}

QString QtMultimediaBackend::backendName() const
{
    return QStringLiteral("Qt Multimedia");
}

void QtMultimediaBackend::connectSignals()
{
    // State mapping
    connect(m_player.get(), &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState qsState) {
        PlaybackState newState;
        switch (qsState) {
            case QMediaPlayer::PlayingState:
                newState = PlaybackState::Playing;
                break;
            case QMediaPlayer::PausedState:
                newState = PlaybackState::Paused;
                break;
            case QMediaPlayer::StoppedState:
                newState = PlaybackState::Stopped;
                break;
            default:
                newState = PlaybackState::Stopped;
                break;
        }
        if (m_cachedState != newState) {
            m_cachedState = newState;
            Q_EMIT stateChanged(newState);
        }
    });

    // Position forwarding
    connect(m_player.get(), &QMediaPlayer::positionChanged, this, &IPlayerBackend::positionChanged);

    // Duration forwarding
    connect(m_player.get(), &QMediaPlayer::durationChanged, this, &IPlayerBackend::durationChanged);

    // Media finished detection
    connect(m_player.get(), &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            m_cachedState = PlaybackState::Stopped;
            Q_EMIT stateChanged(PlaybackState::Stopped);
            Q_EMIT mediaFinished();
        }
    });

    // Error forwarding
    connect(m_player.get(), &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString &errorStr) {
        m_cachedState = PlaybackState::Error;
        Q_EMIT stateChanged(PlaybackState::Error);
        Q_EMIT errorOccurred(errorStr);
    });
}

void QtMultimediaBackend::updateCachedState()
{
    switch (m_player->playbackState()) {
        case QMediaPlayer::PlayingState:
            m_cachedState = PlaybackState::Playing;
            break;
        case QMediaPlayer::PausedState:
            m_cachedState = PlaybackState::Paused;
            break;
        case QMediaPlayer::StoppedState:
        default:
            m_cachedState = PlaybackState::Stopped;
            break;
    }
}

} // namespace NeriPlayerQt
