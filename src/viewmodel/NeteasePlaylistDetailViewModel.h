/// @file NeteasePlaylistDetailViewModel.h
/// @brief ViewModel for NetEase playlist/album detail view

#ifndef QERIPLAYERQT_NETEASEPLAYLISTDETAILVIEWMODEL_H
#define QERIPLAYERQT_NETEASEPLAYLISTDETAILVIEWMODEL_H

#include "api/netease/NeteaseClient.h"
#include "domain/Song.h"
#include "repo/IPlaylistRepository.h"
#include "repo/ISongRepository.h"
#include "viewmodel/SongListModel.h"
#include "viewmodel/ViewModelError.h"

#include <QCoroTask>
#include <QObject>
#include <QString>
#include <QVector>

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

    Q_INVOKABLE void loadPlaylist(const QString &playlistId);
    Q_INVOKABLE void loadAlbum(const QString &albumId);
    Q_INVOKABLE void retry();
    Q_INVOKABLE void saveToLocal();
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
    QCoro::Task<void> loadPlaylistImpl(const QString &playlistId);
    QCoro::Task<void> loadAlbumImpl(const QString &albumId);

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
};

} // namespace QeriPlayerQt

#endif
