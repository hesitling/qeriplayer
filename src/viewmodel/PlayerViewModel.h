/// @file PlayerViewModel.h
/// @brief ViewModel wrapping PlaybackController for QML binding

#ifndef QERIPLAYERQT_PLAYERVIEWMODEL_H
#define QERIPLAYERQT_PLAYERVIEWMODEL_H

#include "domain/Enums.h"
#include "domain/Song.h"
#include "player/PlaybackController.h"
#include "repo/IPlayHistoryRepository.h"
#include "viewmodel/SongListModel.h"
#include "viewmodel/ViewModelError.h"

#include <QCoroTask>
#include <QObject>

namespace QeriPlayerQt {

/**
 * @brief ViewModel exposing PlaybackController state to QML
 *
 * Wraps PlaybackController signals into Q_PROPERTY-based UI contract.
 * Records play history via IPlayHistoryRepository on song change.
 */
class PlayerViewModel : public QObject {
    Q_OBJECT

    // --- Properties ---
    Q_PROPERTY(Song currentSong READ currentSong NOTIFY currentSongChanged)
    Q_PROPERTY(PlaybackState playbackState READ playbackState NOTIFY playbackStateChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playbackStateChanged)
    Q_PROPERTY(bool isPaused READ isPaused NOTIFY playbackStateChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY playbackStateChanged)
    Q_PROPERTY(qint64 positionMs READ positionMs NOTIFY positionChanged)
    Q_PROPERTY(qint64 durationMs READ durationMs NOTIFY durationChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool isMuted READ isMuted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(RepeatMode repeatMode READ repeatMode NOTIFY repeatModeChanged)
    Q_PROPERTY(bool isShuffleEnabled READ isShuffleEnabled NOTIFY shuffleChanged)
    Q_PROPERTY(SongListModel *queue READ queue CONSTANT)
    Q_PROPERTY(bool hasError READ hasError NOTIFY errorChanged)
    Q_PROPERTY(ViewModelError error READ error NOTIFY errorChanged)

public:
    explicit PlayerViewModel(PlaybackController *controller, IPlayHistoryRepository *historyRepo,
                             QObject *parent = nullptr);
    ~PlayerViewModel() override;

    // --- Getters ---
    Song currentSong() const;
    PlaybackState playbackState() const;
    bool isPlaying() const;
    bool isPaused() const;
    bool isLoading() const;
    qint64 positionMs() const;
    qint64 durationMs() const;
    double volume() const;
    bool isMuted() const;
    RepeatMode repeatMode() const;
    bool isShuffleEnabled() const;
    SongListModel *queue() const;
    bool hasError() const;
    ViewModelError error() const;

    // --- Playback control ---
    Q_INVOKABLE QCoro::Task<void> play(const QeriPlayerQt::Song &song);
    Q_INVOKABLE void loadQueueAndPlay(const QVector<QeriPlayerQt::Song> &songs, int startIndex);
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(qint64 positionMs);
    Q_INVOKABLE void next();
    Q_INVOKABLE void prev();

    // --- Volume / Mute ---
    void setVolume(double normalized);
    void setMuted(bool muted);
    Q_INVOKABLE void toggleMute();

    // --- Repeat / Shuffle ---
    Q_INVOKABLE void cycleRepeatMode();
    Q_INVOKABLE void toggleShuffle();

    // --- Queue management ---
    Q_INVOKABLE void addToQueue(const QeriPlayerQt::Song &song);
    Q_INVOKABLE void removeFromQueue(int index);
    Q_INVOKABLE void moveInQueue(int from, int to);
    Q_INVOKABLE void clearQueue();

Q_SIGNALS:
    void currentSongChanged();
    void playbackStateChanged();
    void positionChanged();
    void durationChanged();
    void volumeChanged();
    void mutedChanged();
    void repeatModeChanged();
    void shuffleChanged();
    void errorChanged();

private:
    void connectControllerSignals();
    void connectQueueSignals();
    void updateQueueModel();

    PlaybackController *m_controller;
    IPlayHistoryRepository *m_historyRepo;
    SongListModel *m_queueModel;
    ViewModelError m_error;
    bool m_hasError = false;
    qint64 m_positionMs = 0;
    qint64 m_durationMs = 0;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_PLAYERVIEWMODEL_H
