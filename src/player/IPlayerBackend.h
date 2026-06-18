/// @file IPlayerBackend.h
/// @brief Abstract interface for audio playback backends

#ifndef QERIPLAYERQT_IPLAYERBACKEND_H
#define QERIPLAYERQT_IPLAYERBACKEND_H

#include "domain/Enums.h"

#include <QCoroTask>
#include <QObject>
#include <QString>
#include <QUrl>

namespace QeriPlayerQt {

/**
 * @brief Abstract interface for audio playback backends
 *
 * Defines the contract that all audio backends (Qt Multimedia, mpv, etc.)
 * must implement. Uses Qt signals for state notifications to integrate
 * naturally with ViewModel property binding.
 */
class IPlayerBackend : public QObject {
    Q_OBJECT

public:
    explicit IPlayerBackend(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    ~IPlayerBackend() override = default;

    /**
     * @brief Load media from URL, preparing it for playback
     * @param url The media URL to load
     * @throws std::runtime_error if loading fails
     */
    virtual QCoro::Task<void> load(const QUrl &url) = 0;

    /// @brief Start or resume playback
    virtual void play() = 0;

    /// @brief Suspend playback at current position
    virtual void pause() = 0;

    /// @brief Halt playback and reset position to zero
    virtual void stop() = 0;

    /**
     * @brief Seek to position in milliseconds
     * @param positionMs Target position in milliseconds
     */
    virtual void seek(qint64 positionMs) = 0;

    /// @brief Current playback state
    [[nodiscard]] virtual PlaybackState state() const = 0;

    /// @brief Current position in milliseconds
    [[nodiscard]] virtual qint64 positionMs() const = 0;

    /// @brief Media duration in milliseconds
    [[nodiscard]] virtual qint64 durationMs() const = 0;

    /// @brief Whether the current media is seekable
    [[nodiscard]] virtual bool isSeekable() const = 0;

    /**
     * @brief Set volume (0.0 to 1.0)
     * @param normalized Volume level, clamped to [0.0, 1.0]
     */
    virtual void setVolume(double normalized) = 0;

    /// @brief Current volume level (0.0 to 1.0)
    [[nodiscard]] virtual double volume() const = 0;

    /// @brief Set muted state
    virtual void setMuted(bool muted) = 0;

    /// @brief Whether audio is currently muted
    [[nodiscard]] virtual bool isMuted() const = 0;

    /// @brief Human-readable backend name (e.g., "Qt Multimedia", "mpv")
    [[nodiscard]] virtual QString backendName() const = 0;

Q_SIGNALS:
    void stateChanged(QeriPlayerQt::PlaybackState newState);
    void positionChanged(qint64 positionMs);
    void durationChanged(qint64 durationMs);
    void mediaFinished();
    void errorOccurred(const QString &message);
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_IPLAYERBACKEND_H
