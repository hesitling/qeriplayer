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

QCoro::QmlTask LocalPlaylistDetailViewModel::loadPlaylist(const QString &id)
{
    return QCoro::QmlTask(loadPlaylistImpl(id));
}

QCoro::QmlTask LocalPlaylistDetailViewModel::addSong(const QString &songId)
{
    return QCoro::QmlTask([this, songId]() -> QCoro::Task<void> {
        try {
            m_playlistRepo->addSong(m_playlistId, songId);
            co_await loadPlaylistImpl(m_playlistId); // Refresh
        } catch (const std::exception &ex) {
            Logger::get("viewmodel")->warn("Failed to add song to playlist: {}", ex.what());
            m_error = ViewModelError(ViewModelError::ErrorType::Database, QString::fromUtf8(ex.what()));
            m_hasError = true;
            Q_EMIT errorChanged();
        }
    }());
}

QCoro::QmlTask LocalPlaylistDetailViewModel::removeSong(const QString &songId)
{
    return QCoro::QmlTask([this, songId]() -> QCoro::Task<void> {
        try {
            m_playlistRepo->removeSong(m_playlistId, songId);
            co_await loadPlaylistImpl(m_playlistId); // Refresh
        } catch (const std::exception &ex) {
            Logger::get("viewmodel")->warn("Failed to remove song from playlist: {}", ex.what());
            m_error = ViewModelError(ViewModelError::ErrorType::Database, QString::fromUtf8(ex.what()));
            m_hasError = true;
            Q_EMIT errorChanged();
        }
    }());
}

QCoro::QmlTask LocalPlaylistDetailViewModel::reorderSongs(const QStringList &songIds)
{
    return QCoro::QmlTask([this, songIds]() -> QCoro::Task<void> {
        try {
            m_playlistRepo->reorderSongs(m_playlistId, songIds);
            co_await loadPlaylistImpl(m_playlistId); // Refresh
        } catch (const std::exception &ex) {
            Logger::get("viewmodel")->warn("Failed to reorder songs: {}", ex.what());
            m_error = ViewModelError(ViewModelError::ErrorType::Database, QString::fromUtf8(ex.what()));
            m_hasError = true;
            Q_EMIT errorChanged();
        }
    }());
}

QCoro::QmlTask LocalPlaylistDetailViewModel::rename(const QString &newName)
{
    return QCoro::QmlTask([this, newName]() -> QCoro::Task<void> {
        try {
            auto playlist = m_playlistRepo->findById(m_playlistId);
            if (playlist.has_value()) {
                m_playlistRepo->updateMetadata(m_playlistId, newName, playlist->description,
                                               playlist->coverUrl.toString());
                m_playlistName = newName;
                Q_EMIT playlistNameChanged();
            }
        } catch (const std::exception &ex) {
            Logger::get("viewmodel")->warn("Failed to rename playlist: {}", ex.what());
            m_error = ViewModelError(ViewModelError::ErrorType::Database, QString::fromUtf8(ex.what()));
            m_hasError = true;
            Q_EMIT errorChanged();
        }
        co_return;
    }());
}

QCoro::QmlTask LocalPlaylistDetailViewModel::deletePlaylist()
{
    return QCoro::QmlTask([this]() -> QCoro::Task<void> {
        try {
            m_playlistRepo->remove(m_playlistId);
            Q_EMIT playlistDeleted();
        } catch (const std::exception &ex) {
            Logger::get("viewmodel")->warn("Failed to delete playlist: {}", ex.what());
            m_error = ViewModelError(ViewModelError::ErrorType::Database, QString::fromUtf8(ex.what()));
            m_hasError = true;
            Q_EMIT errorChanged();
        }
        co_return;
    }());
}

// --- Private ---

QCoro::Task<void> LocalPlaylistDetailViewModel::loadPlaylistImpl(const QString &id)
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
            co_return;
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
    co_return;
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
