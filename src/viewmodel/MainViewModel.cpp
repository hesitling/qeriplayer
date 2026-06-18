/// @file MainViewModel.cpp
/// @brief Implementation of MainViewModel

#include "viewmodel/MainViewModel.h"

namespace QeriPlayerQt {

MainViewModel::MainViewModel(PlayerViewModel *playerVm, SearchViewModel *searchVm, PlaylistViewModel *playlistVm,
                             SettingsViewModel *settingsVm, QObject *parent)
    : QObject(parent)
    , m_playerVm(playerVm)
    , m_searchVm(searchVm)
    , m_playlistVm(playlistVm)
    , m_settingsVm(settingsVm)
{
    connectSignals();
}

MainViewModel::~MainViewModel() = default;

MainViewModel::View MainViewModel::currentView() const
{
    return m_currentView;
}
PlayerViewModel *MainViewModel::playerViewModel() const
{
    return m_playerVm;
}
SearchViewModel *MainViewModel::searchViewModel() const
{
    return m_searchVm;
}
PlaylistViewModel *MainViewModel::playlistViewModel() const
{
    return m_playlistVm;
}
SettingsViewModel *MainViewModel::settingsViewModel() const
{
    return m_settingsVm;
}
LocalPlaylistDetailViewModel *MainViewModel::localPlaylistDetail() const
{
    return m_localPlaylistDetail;
}
NeteasePlaylistDetailViewModel *MainViewModel::neteasePlaylistDetail() const
{
    return m_neteasePlaylistDetail;
}

void MainViewModel::navigateTo(View view)
{
    if (m_currentView == view)
        return;

    // Delete detail VMs when navigating away
    if (m_currentView == View::LocalPlaylist || m_currentView == View::NeteasePlaylist) {
        deleteDetailViewModels();
    }

    m_currentView = view;
    Q_EMIT currentViewChanged();
}

void MainViewModel::openLocalPlaylist(const QString &id)
{
    deleteDetailViewModels();

    // TODO: get repos from ServiceLocator or inject
    // For now, create with nullptr repos (will be wired properly in integration)
    m_localPlaylistDetail = new LocalPlaylistDetailViewModel(nullptr, nullptr, this);
    wireDetailVmSignals();
    m_localPlaylistDetail->loadPlaylist(id);
    Q_EMIT localPlaylistDetailChanged();

    navigateTo(View::LocalPlaylist);
}

void MainViewModel::openNeteasePlaylist(const PlaylistSummary &summary)
{
    deleteDetailViewModels();

    // TODO: get repos from ServiceLocator or inject
    m_neteasePlaylistDetail = new NeteasePlaylistDetailViewModel(nullptr, nullptr, nullptr, this);
    wireDetailVmSignals();
    m_neteasePlaylistDetail->loadPlaylist(summary.id);
    Q_EMIT neteasePlaylistDetailChanged();

    navigateTo(View::NeteasePlaylist);
}

void MainViewModel::openNeteaseAlbum(const AlbumSummary &summary)
{
    deleteDetailViewModels();

    m_neteasePlaylistDetail = new NeteasePlaylistDetailViewModel(nullptr, nullptr, nullptr, this);
    wireDetailVmSignals();
    m_neteasePlaylistDetail->loadAlbum(summary.id);
    Q_EMIT neteasePlaylistDetailChanged();

    navigateTo(View::NeteasePlaylist);
}

void MainViewModel::initialize()
{
    m_settingsVm->loadSettings();
    m_playlistVm->loadLocalPlaylists();
}

void MainViewModel::deleteDetailViewModels()
{
    if (m_localPlaylistDetail) {
        m_localPlaylistDetail->deleteLater();
        m_localPlaylistDetail = nullptr;
        Q_EMIT localPlaylistDetailChanged();
    }
    if (m_neteasePlaylistDetail) {
        m_neteasePlaylistDetail->deleteLater();
        m_neteasePlaylistDetail = nullptr;
        Q_EMIT neteasePlaylistDetailChanged();
    }
}

void MainViewModel::connectSignals()
{
    // Search → Player
    connect(m_searchVm, &SearchViewModel::requestPlay, m_playerVm,
            [this](const Song &song) { m_playerVm->play(song); });

    // Playlist → Navigation
    connect(m_playlistVm, &PlaylistViewModel::localPlaylistSelected, this, &MainViewModel::openLocalPlaylist);
    connect(m_playlistVm, &PlaylistViewModel::neteasePlaylistSelected, this, &MainViewModel::openNeteasePlaylist);
    connect(m_playlistVm, &PlaylistViewModel::neteaseAlbumSelected, this, &MainViewModel::openNeteaseAlbum);
}

void MainViewModel::wireDetailVmSignals()
{
    if (m_localPlaylistDetail) {
        connect(m_localPlaylistDetail, &LocalPlaylistDetailViewModel::requestPlay, m_playerVm,
                [this](const Song &song) { m_playerVm->play(song); });
        connect(
            m_localPlaylistDetail, &LocalPlaylistDetailViewModel::requestPlayPlaylist, m_playerVm,
            [this](const QVector<Song> &songs, int startIndex) { m_playerVm->loadQueueAndPlay(songs, startIndex); });
    }
    if (m_neteasePlaylistDetail) {
        connect(m_neteasePlaylistDetail, &NeteasePlaylistDetailViewModel::requestPlay, m_playerVm,
                [this](const Song &song) { m_playerVm->play(song); });
        connect(
            m_neteasePlaylistDetail, &NeteasePlaylistDetailViewModel::requestPlayPlaylist, m_playerVm,
            [this](const QVector<Song> &songs, int startIndex) { m_playerVm->loadQueueAndPlay(songs, startIndex); });
    }
}

} // namespace QeriPlayerQt
