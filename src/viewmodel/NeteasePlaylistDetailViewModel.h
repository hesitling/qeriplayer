/// @file NeteasePlaylistDetailViewModel.h
/// @brief ViewModel for NetEase playlist/album detail view

#ifndef QERIPLAYERQT_NETEASEPLAYLISTDETAILVIEWMODEL_H
#define QERIPLAYERQT_NETEASEPLAYLISTDETAILVIEWMODEL_H

#include "api/common/ApiResult.h"
#include "api/netease/NeteaseClient.h"
#include "domain/Song.h"
#include "repo/IPlaylistRepository.h"
#include "repo/ISongRepository.h"
#include "viewmodel/SongListModel.h"
#include "viewmodel/ViewModelError.h"

#include <QCoroQmlTask>
#include <QCoroTask>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVector>

#include <exception>

namespace QeriPlayerQt {

class NeteasePlaylistDetailViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString headerName READ headerName NOTIFY headerNameChanged)
    Q_PROPERTY(QString headerCoverUrl READ headerCoverUrl NOTIFY headerCoverUrlChanged)
    Q_PROPERTY(int headerTrackCount READ headerTrackCount NOTIFY headerTrackCountChanged)
    Q_PROPERTY(SongListModel *songs READ songs CONSTANT)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(bool hasError READ hasError NOTIFY errorChanged)
    Q_PROPERTY(ViewModelError error READ error NOTIFY errorChanged)

public:
    explicit NeteasePlaylistDetailViewModel(NeteaseClient *neteaseClient, ISongRepository *songRepo,
                                            IPlaylistRepository *playlistRepo, QObject *parent = nullptr);
    ~NeteasePlaylistDetailViewModel() override;

    QString headerName() const;
    QString headerCoverUrl() const;
    int headerTrackCount() const;
    SongListModel *songs() const;
    bool isLoading() const;
    bool hasError() const;
    ViewModelError error() const;

    Q_INVOKABLE QCoro::QmlTask loadPlaylist(const QString &playlistId);
    Q_INVOKABLE QCoro::QmlTask loadAlbum(const QString &albumId);
    Q_INVOKABLE QCoro::QmlTask retry();
    Q_INVOKABLE QCoro::QmlTask saveToLocal();
    Q_INVOKABLE void playSong(int index);
    Q_INVOKABLE void playAll();

Q_SIGNALS:
    void headerNameChanged();
    void headerCoverUrlChanged();
    void headerTrackCountChanged();
    void isLoadingChanged();
    void errorChanged();
    void requestPlay(const QeriPlayerQt::Song &song);
    void requestPlayPlaylist(const QVector<QeriPlayerQt::Song> &songs, int startIndex);

private:
    void beginLoad();
    static QCoro::Task<void> loadPlaylistTask(QPointer<NeteasePlaylistDetailViewModel> self, QString playlistId);
    static QCoro::Task<void> loadAlbumTask(QPointer<NeteasePlaylistDetailViewModel> self, QString albumId);
    static QCoro::Task<void> saveToLocalTask(QPointer<NeteasePlaylistDetailViewModel> self);

    template <typename T> bool finalizeLoad(const ApiResult<T> &result)
    {
        m_isLoading = false;
        Q_EMIT isLoadingChanged();

        if (result.isError()) {
            setErrorState(ViewModelError::fromApiError(result.error()));
            return false;
        }

        return true;
    }

    bool ensureClientAvailable();
    void setErrorState(const ViewModelError &error);
    void handleLoadException(const std::exception &ex, const char *entityName);
    void applyPlaylist(const Playlist &playlist);
    void applyAlbumSongs(const QVector<Song> &songs);

    NeteaseClient *m_neteaseClient;
    ISongRepository *m_songRepo;
    IPlaylistRepository *m_playlistRepo;
    SongListModel *m_songs;

    QString m_headerName;
    QString m_headerCoverUrl;
    int m_headerTrackCount = 0;
    bool m_isLoading = false;
    ViewModelError m_error;
    bool m_hasError = false;

    // For retry
    QString m_lastPlaylistId;
    QString m_lastAlbumId;
    bool m_isAlbum = false;
    QCoro::QmlTask m_pendingTask;
};

} // namespace QeriPlayerQt

#endif
