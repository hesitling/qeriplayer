/// @file PlaybackController.h
/// @brief High-level orchestrator coordinating backend, queue, URL resolution, and persistence

#ifndef QERIPLAYERQT_PLAYBACKCONTROLLER_H
#define QERIPLAYERQT_PLAYBACKCONTROLLER_H

#include "api/common/IMusicPlatformPlugin.h"
#include "domain/Enums.h"
#include "domain/Song.h"
#include "player/IPlayerBackend.h"
#include "player/PlayQueue.h"
#include "repo/IPlayerStateRepository.h"
#include "repo/ISettingsRepository.h"

#include <QCoroTask>
#include <QHash>
#include <QObject>
#include <QString>
#include <QTimer>

#include <memory>

namespace QeriPlayerQt {

/**
 * @brief High-level playback orchestrator
 *
 * Coordinates the backend, play queue, URL resolution, and state persistence.
 * Manages pre-resolving URLs, auto-advance on track finish, and volume/mute
 * settings persistence.
 */
class PlaybackController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Construct a PlaybackController
     * @param backend The audio playback backend (ownership transferred)
     * @param plugin Platform plugin for URL resolution
     * @param playerStateRepo Repository for player state persistence
     * @param settingsRepo Repository for settings persistence
     * @param parent QObject parent
     */
    PlaybackController(std::unique_ptr<IPlayerBackend> backend, IMusicPlatformPlugin *plugin,
                       IPlayerStateRepository *playerStateRepo, ISettingsRepository *settingsRepo,
                       QObject *parent = nullptr);
    ~PlaybackController() override;

    // --- Playback control ---

    /**
     * @brief Play a song (resolves URL if needed, loads into backend, starts playback)
     * @param song The song to play
     */
    QCoro::Task<void> play(const Song &song);

    /// @brief Pause playback
    void pause();

    /// @brief Resume playback
    void resume();

    /// @brief Stop playback
    void stop();

    /// @brief Seek to position in milliseconds
    void seek(qint64 positionMs);

    // --- Queue access ---

    /// @brief Access the play queue
    [[nodiscard]] PlayQueue *queue();

    // --- Volume ---

    void setVolume(double normalized);
    [[nodiscard]] double volume() const;

    void setMuted(bool muted);
    [[nodiscard]] bool isMuted() const;

    // --- State queries ---

    [[nodiscard]] PlaybackState playbackState() const;
    [[nodiscard]] const Song &currentSong() const;
    [[nodiscard]] QString backendName() const;

    // --- Pre-resolve ---

    /**
     * @brief Schedule background URL resolution for a song
     * @param song The song whose URL should be pre-resolved
     */
    void preResolveUrl(const Song &song);

    /**
     * @brief Restore saved playback state from repository (non-blocking)
     *
     * Kicks off an async restore if PersistedPlayerState::shouldResumePlayback is true.
     * Safe to call from non-coroutine contexts.
     */
    void restoreState();

Q_SIGNALS:
    void currentSongChanged(const QeriPlayerQt::Song &song);
    void playbackStateChanged(QeriPlayerQt::PlaybackState state);
    void positionChanged(qint64 positionMs);
    void durationChanged(qint64 durationMs);
    void errorOccurred(const QString &message);
    void playbackFinished();

private:
    void connectBackendSignals();
    void connectQueueSignals();
    void persistState();
    QCoro::Task<QString> resolveUrl(const Song &song);

    std::unique_ptr<IPlayerBackend> m_backend;
    IMusicPlatformPlugin *m_plugin;
    IPlayerStateRepository *m_playerStateRepo;
    ISettingsRepository *m_settingsRepo;
    PlayQueue *m_queue;

    Song m_currentSong;
    PlaybackState m_state = PlaybackState::Stopped;

    // URL cache: songId -> url
    QHash<QString, QString> m_urlCache;
    // TTL timestamps: songId -> expiry time (epoch ms)
    QHash<QString, qint64> m_urlCacheExpiry;
    static constexpr qint64 URL_CACHE_TTL_MS = 30 * 60 * 1000; // 30 minutes

    // Debounced save for seek operations
    QTimer *m_seekSaveTimer;

    // Running async tasks (prevent premature destruction)
    QCoro::Task<void> m_restoreState;
    QCoro::Task<void> m_autoAdvanceTask;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_PLAYBACKCONTROLLER_H
