/// @file MainViewModel.h
/// @brief Main ViewModel coordinating child VMs and navigation

#ifndef NERIPLAYERQT_MAINVIEWMODEL_H
#define NERIPLAYERQT_MAINVIEWMODEL_H

#include "domain/PlaylistSummary.h"
#include "viewmodel/LocalPlaylistDetailViewModel.h"
#include "viewmodel/NeteasePlaylistDetailViewModel.h"
#include "viewmodel/PlayerViewModel.h"
#include "viewmodel/PlaylistViewModel.h"
#include "viewmodel/SearchViewModel.h"
#include "viewmodel/SettingsViewModel.h"

#include <QObject>

namespace NeriPlayerQt {

class MainViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(View currentView READ currentView NOTIFY currentViewChanged)
    Q_PROPERTY(PlayerViewModel *playerViewModel READ playerViewModel CONSTANT)
    Q_PROPERTY(SearchViewModel *searchViewModel READ searchViewModel CONSTANT)
    Q_PROPERTY(PlaylistViewModel *playlistViewModel READ playlistViewModel CONSTANT)
    Q_PROPERTY(SettingsViewModel *settingsViewModel READ settingsViewModel CONSTANT)
    Q_PROPERTY(
        LocalPlaylistDetailViewModel *localPlaylistDetail READ localPlaylistDetail NOTIFY localPlaylistDetailChanged)
    Q_PROPERTY(NeteasePlaylistDetailViewModel *neteasePlaylistDetail READ neteasePlaylistDetail NOTIFY
                   neteasePlaylistDetailChanged)

public:
    enum class View : quint8 { Home, Search, Library, LocalPlaylist, NeteasePlaylist, Settings };
    Q_ENUM(View)

    MainViewModel(PlayerViewModel *playerVm, SearchViewModel *searchVm, PlaylistViewModel *playlistVm,
                  SettingsViewModel *settingsVm, QObject *parent = nullptr);
    ~MainViewModel() override;

    View currentView() const;
    PlayerViewModel *playerViewModel() const;
    SearchViewModel *searchViewModel() const;
    PlaylistViewModel *playlistViewModel() const;
    SettingsViewModel *settingsViewModel() const;
    LocalPlaylistDetailViewModel *localPlaylistDetail() const;
    NeteasePlaylistDetailViewModel *neteasePlaylistDetail() const;

    Q_INVOKABLE void navigateTo(NeriPlayerQt::MainViewModel::View view);
    Q_INVOKABLE void openLocalPlaylist(const QString &id);
    Q_INVOKABLE void openNeteasePlaylist(const NeriPlayerQt::PlaylistSummary &summary);
    Q_INVOKABLE void openNeteaseAlbum(const NeriPlayerQt::AlbumSummary &summary);
    Q_INVOKABLE void initialize();

Q_SIGNALS:
    void currentViewChanged();
    void localPlaylistDetailChanged();
    void neteasePlaylistDetailChanged();

private:
    void deleteDetailViewModels();
    void connectSignals();
    void wireDetailVmSignals();

    View m_currentView = View::Home;
    PlayerViewModel *m_playerVm;
    SearchViewModel *m_searchVm;
    PlaylistViewModel *m_playlistVm;
    SettingsViewModel *m_settingsVm;
    LocalPlaylistDetailViewModel *m_localPlaylistDetail = nullptr;
    NeteasePlaylistDetailViewModel *m_neteasePlaylistDetail = nullptr;
};

} // namespace NeriPlayerQt

#endif
