/// @file PlaylistViewModel.h
/// @brief ViewModel for library overview listing playlists from all sources

#ifndef QERIPLAYERQT_PLAYLISTVIEWMODEL_H
#define QERIPLAYERQT_PLAYLISTVIEWMODEL_H

#include "api/netease/NeteaseClient.h"
#include "domain/PlaylistSummary.h"
#include "repo/IPlaylistRepository.h"
#include "viewmodel/ViewModelError.h"

#include <QCoroTask>
#include <QObject>
#include <QString>
#include <QVariantList>

namespace QeriPlayerQt {

/**
 * @brief ViewModel managing the library overview
 *
 * Lists playlists from local and NetEase sources.
 * Selection signals navigate to detail screens.
 */
class PlaylistViewModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(QVariantList localPlaylists READ localPlaylists NOTIFY localPlaylistsChanged)
    Q_PROPERTY(QVariantList neteasePlaylists READ neteasePlaylists NOTIFY neteasePlaylistsChanged)
    Q_PROPERTY(QVariantList neteaseAlbums READ neteaseAlbums NOTIFY neteaseAlbumsChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(bool hasError READ hasError NOTIFY errorChanged)
    Q_PROPERTY(ViewModelError error READ error NOTIFY errorChanged)

public:
    explicit PlaylistViewModel(IPlaylistRepository *playlistRepo, NeteaseClient *neteaseClient,
                               QObject *parent = nullptr);
    ~PlaylistViewModel() override;

    // --- Getters ---
    QVariantList localPlaylists() const;
    QVariantList neteasePlaylists() const;
    QVariantList neteaseAlbums() const;
    bool isLoading() const;
    bool hasError() const;
    ViewModelError error() const;

    // --- Loading ---
    Q_INVOKABLE void loadLocalPlaylists();
    Q_INVOKABLE void loadNeteasePlaylists();
    Q_INVOKABLE void loadNeteaseAlbums();

    // --- Local playlist CRUD ---
    Q_INVOKABLE void createLocalPlaylist(const QString &name);
    Q_INVOKABLE void deleteLocalPlaylist(const QString &id);
    Q_INVOKABLE void renameLocalPlaylist(const QString &id, const QString &name);

    // --- Error ---
    Q_INVOKABLE void clearError();

Q_SIGNALS:
    void localPlaylistsChanged();
    void neteasePlaylistsChanged();
    void neteaseAlbumsChanged();
    void isLoadingChanged();
    void errorChanged();

    /// @brief User selected a local playlist for detail view
    void localPlaylistSelected(const QString &id);

    /// @brief User selected a NetEase playlist for detail view
    void neteasePlaylistSelected(const QeriPlayerQt::PlaylistSummary &summary);

    /// @brief User selected a NetEase album for detail view
    void neteaseAlbumSelected(const QeriPlayerQt::AlbumSummary &summary);

private:
    QCoro::Task<void> loadNeteasePlaylistsImpl();
    QCoro::Task<void> loadNeteaseAlbumsImpl();

    IPlaylistRepository *m_playlistRepo;
    NeteaseClient *m_neteaseClient;

    QVariantList m_localPlaylists;
    QVariantList m_neteasePlaylists;
    QVariantList m_neteaseAlbums;
    bool m_isLoading = false;
    ViewModelError m_error;
    bool m_hasError = false;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_PLAYLISTVIEWMODEL_H
