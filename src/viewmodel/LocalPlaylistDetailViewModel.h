/// @file LocalPlaylistDetailViewModel.h
/// @brief ViewModel for local playlist detail view

#ifndef QERIPLAYERQT_LOCALPLAYLISTDETAILVIEWMODEL_H
#define QERIPLAYERQT_LOCALPLAYLISTDETAILVIEWMODEL_H

#include "domain/Playlist.h"
#include "domain/Song.h"
#include "repo/IPlaylistRepository.h"
#include "repo/ISongRepository.h"
#include "viewmodel/SongListModel.h"
#include "viewmodel/ViewModelError.h"

#include <QObject>
#include <QStringList>
#include <QVector>

namespace QeriPlayerQt {

class LocalPlaylistDetailViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString playlistId READ playlistId NOTIFY playlistIdChanged)
    Q_PROPERTY(QString playlistName READ playlistName NOTIFY playlistNameChanged)
    Q_PROPERTY(SongListModel *songs READ songs CONSTANT)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(bool hasError READ hasError NOTIFY errorChanged)
    Q_PROPERTY(ViewModelError error READ error NOTIFY errorChanged)

public:
    explicit LocalPlaylistDetailViewModel(IPlaylistRepository *playlistRepo, ISongRepository *songRepo,
                                          QObject *parent = nullptr);
    ~LocalPlaylistDetailViewModel() override;

    QString playlistId() const;
    QString playlistName() const;
    SongListModel *songs() const;
    bool isLoading() const;
    bool hasError() const;
    ViewModelError error() const;

    Q_INVOKABLE void loadPlaylist(const QString &id);
    Q_INVOKABLE void addSong(const QString &songId);
    Q_INVOKABLE void removeSong(const QString &songId);
    Q_INVOKABLE void reorderSongs(const QStringList &songIds);
    Q_INVOKABLE void rename(const QString &newName);
    Q_INVOKABLE void deletePlaylist();
    Q_INVOKABLE void playSong(int index);
    Q_INVOKABLE void playAll();

Q_SIGNALS:
    void playlistIdChanged();
    void playlistNameChanged();
    void isLoadingChanged();
    void errorChanged();
    void requestPlay(const QeriPlayerQt::Song &song);
    void requestPlayPlaylist(const QVector<QeriPlayerQt::Song> &songs, int startIndex);
    void playlistDeleted();

private:
    IPlaylistRepository *m_playlistRepo;
    ISongRepository *m_songRepo;
    SongListModel *m_songs;
    QString m_playlistId;
    QString m_playlistName;
    bool m_isLoading = false;
    ViewModelError m_error;
    bool m_hasError = false;
};

} // namespace QeriPlayerQt

#endif
