/// @file LocalPlaylistDetailViewModel.cpp
/// @brief Implementation of LocalPlaylistDetailViewModel

#include "viewmodel/LocalPlaylistDetailViewModel.h"

#include "core/logger/Logger.h"

namespace QeriPlayerQt {

LocalPlaylistDetailViewModel::LocalPlaylistDetailViewModel(IPlaylistRepository *playlistRepo, ISongRepository *songRepo,
                                                           QObject *parent)
    : QObject(parent)
    , m_playlistRepo(playlistRepo)
    , m_songRepo(songRepo)
    , m_songs(new SongListModel(this))
{
}

LocalPlaylistDetailViewModel::~LocalPlaylistDetailViewModel() = default;

QString LocalPlaylistDetailViewModel::playlistId() const
{
    return m_playlistId;
}
QString LocalPlaylistDetailViewModel::playlistName() const
{
    return m_playlistName;
}
SongListModel *LocalPlaylistDetailViewModel::songs() const
{
    return m_songs;
}
bool LocalPlaylistDetailViewModel::isLoading() const
{
    return m_isLoading;
}
bool LocalPlaylistDetailViewModel::hasError() const
{
    return m_hasError;
}
ViewModelError LocalPlaylistDetailViewModel::error() const
{
    return m_error;
}

void LocalPlaylistDetailViewModel::loadPlaylist(const QString &id)
{
    m_playlistId = id;
    Q_EMIT playlistIdChanged();

    try {
        auto playlist = m_playlistRepo->findById(id);
        if (!playlist.has_value()) {
            m_playlistName.clear();
            Q_EMIT playlistNameChanged();
            m_songs->setSongs({});

            m_error = ViewModelError(ViewModelError::ErrorType::NotFound, "Playlist not found");
            m_hasError = true;
            Q_EMIT errorChanged();
            return;
        }

        m_playlistName = playlist->name;
        Q_EMIT playlistNameChanged();

        m_songs->setSongs(playlist->songs);
        m_hasError = false;
        Q_EMIT errorChanged();
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to load playlist: {}", ex.what());
        m_error = ViewModelError(ViewModelError::ErrorType::Database, QString::fromUtf8(ex.what()));
        m_hasError = true;
        Q_EMIT errorChanged();
    }
}

void LocalPlaylistDetailViewModel::addSong(const QString &songId)
{
    try {
        m_playlistRepo->addSong(m_playlistId, songId);
        loadPlaylist(m_playlistId); // Refresh
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to add song to playlist: {}", ex.what());
    }
}

void LocalPlaylistDetailViewModel::removeSong(const QString &songId)
{
    try {
        m_playlistRepo->removeSong(m_playlistId, songId);
        loadPlaylist(m_playlistId); // Refresh
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to remove song from playlist: {}", ex.what());
    }
}

void LocalPlaylistDetailViewModel::reorderSongs(const QStringList &songIds)
{
    try {
        m_playlistRepo->reorderSongs(m_playlistId, songIds);
        loadPlaylist(m_playlistId); // Refresh
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to reorder songs: {}", ex.what());
    }
}

void LocalPlaylistDetailViewModel::rename(const QString &newName)
{
    try {
        auto playlist = m_playlistRepo->findById(m_playlistId);
        if (playlist.has_value()) {
            m_playlistRepo->updateMetadata(m_playlistId, newName, playlist->description, playlist->coverUrl.toString());
            m_playlistName = newName;
            Q_EMIT playlistNameChanged();
        }
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to rename playlist: {}", ex.what());
    }
}

void LocalPlaylistDetailViewModel::deletePlaylist()
{
    try {
        m_playlistRepo->remove(m_playlistId);
        Q_EMIT playlistDeleted();
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to delete playlist: {}", ex.what());
    }
}

void LocalPlaylistDetailViewModel::playSong(int index)
{
    Song song = m_songs->songAt(index);
    if (!song.id.isEmpty()) {
        Q_EMIT requestPlay(song);
    }
}

void LocalPlaylistDetailViewModel::playAll()
{
    Q_EMIT requestPlayPlaylist(m_songs->songs(), 0);
}

} // namespace QeriPlayerQt
