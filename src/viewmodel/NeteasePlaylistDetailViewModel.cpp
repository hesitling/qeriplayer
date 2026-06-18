/// @file NeteasePlaylistDetailViewModel.cpp
/// @brief Implementation of NeteasePlaylistDetailViewModel

#include "viewmodel/NeteasePlaylistDetailViewModel.h"

#include "core/logger/Logger.h"

namespace QeriPlayerQt {

NeteasePlaylistDetailViewModel::NeteasePlaylistDetailViewModel(NeteaseClient *neteaseClient, ISongRepository *songRepo,
                                                               IPlaylistRepository *playlistRepo, QObject *parent)
    : QObject(parent)
    , m_neteaseClient(neteaseClient)
    , m_songRepo(songRepo)
    , m_playlistRepo(playlistRepo)
    , m_songs(new SongListModel(this))
{
}

NeteasePlaylistDetailViewModel::~NeteasePlaylistDetailViewModel() = default;

QString NeteasePlaylistDetailViewModel::headerName() const
{
    return m_headerName;
}
QString NeteasePlaylistDetailViewModel::headerCoverUrl() const
{
    return m_headerCoverUrl;
}
int NeteasePlaylistDetailViewModel::headerTrackCount() const
{
    return m_headerTrackCount;
}
SongListModel *NeteasePlaylistDetailViewModel::songs() const
{
    return m_songs;
}
bool NeteasePlaylistDetailViewModel::isLoading() const
{
    return m_isLoading;
}
bool NeteasePlaylistDetailViewModel::hasError() const
{
    return m_hasError;
}
ViewModelError NeteasePlaylistDetailViewModel::error() const
{
    return m_error;
}

QCoro::QmlTask NeteasePlaylistDetailViewModel::loadPlaylist(const QString &playlistId)
{
    m_lastPlaylistId = playlistId;
    m_lastAlbumId.clear();
    m_isAlbum = false;
    return QCoro::QmlTask(loadPlaylistImpl(playlistId));
}

QCoro::QmlTask NeteasePlaylistDetailViewModel::loadAlbum(const QString &albumId)
{
    m_lastAlbumId = albumId;
    m_lastPlaylistId.clear();
    m_isAlbum = true;
    return QCoro::QmlTask(loadAlbumImpl(albumId));
}

namespace {
QCoro::Task<void> completedTask()
{
    co_return;
}
} // namespace

QCoro::QmlTask NeteasePlaylistDetailViewModel::retry()
{
    if (m_isAlbum) {
        if (m_lastAlbumId.isEmpty()) {
            return QCoro::QmlTask(completedTask());
        }
        return QCoro::QmlTask(loadAlbumImpl(m_lastAlbumId));
    } else {
        if (m_lastPlaylistId.isEmpty()) {
            return QCoro::QmlTask(completedTask());
        }
        return QCoro::QmlTask(loadPlaylistImpl(m_lastPlaylistId));
    }
}

QCoro::QmlTask NeteasePlaylistDetailViewModel::saveToLocal()
{
    return QCoro::QmlTask([this]() -> QCoro::Task<void> {
        if (m_headerName.isEmpty())
            co_return;

        try {
            // Create local playlist
            Playlist localPlaylist = m_playlistRepo->create(m_headerName);

            // Cache all songs
            m_songRepo->saveBatch(m_songs->songs());

            // Add songs to playlist
            for (const Song &song : m_songs->songs()) {
                m_playlistRepo->addSong(localPlaylist.id, song.id);
            }
        } catch (const std::exception &ex) {
            Logger::get("viewmodel")->warn("Failed to save playlist to local: {}", ex.what());
            m_error = ViewModelError(ViewModelError::ErrorType::Unknown, QString::fromUtf8(ex.what()));
            m_hasError = true;
            Q_EMIT errorChanged();
        }
    }());
}

void NeteasePlaylistDetailViewModel::playSong(int index)
{
    Song song = m_songs->songAt(index);
    if (!song.id.isEmpty()) {
        Q_EMIT requestPlay(song);
    }
}

void NeteasePlaylistDetailViewModel::playAll()
{
    Q_EMIT requestPlayPlaylist(m_songs->songs(), 0);
}

QCoro::Task<void> NeteasePlaylistDetailViewModel::loadPlaylistImpl(const QString &playlistId)
{
    m_isLoading = true;
    Q_EMIT isLoadingChanged();
    m_hasError = false;
    Q_EMIT errorChanged();

    try {
        auto result = co_await m_neteaseClient->getPlaylistDetail(playlistId);

        m_isLoading = false;
        Q_EMIT isLoadingChanged();

        if (result.isError()) {
            m_error = ViewModelError::fromApiError(result.error());
            m_hasError = true;
            Q_EMIT errorChanged();
            co_return;
        }

        const Playlist &playlist = result.data();
        m_headerName = playlist.name;
        m_headerCoverUrl = playlist.coverUrl.toString();
        m_headerTrackCount = playlist.songCount;

        Q_EMIT headerNameChanged();
        Q_EMIT headerCoverUrlChanged();
        Q_EMIT headerTrackCountChanged();

        m_songs->setSongs(playlist.songs);

        // Cache songs
        m_songRepo->saveBatch(playlist.songs);
    } catch (const std::exception &ex) {
        m_isLoading = false;
        Q_EMIT isLoadingChanged();
        m_error = ViewModelError(ViewModelError::ErrorType::Unknown, QString::fromUtf8(ex.what()));
        m_hasError = true;
        Q_EMIT errorChanged();
        Logger::get("viewmodel")->warn("Failed to load NetEase playlist: {}", ex.what());
    }
}

QCoro::Task<void> NeteasePlaylistDetailViewModel::loadAlbumImpl(const QString &albumId)
{
    m_isLoading = true;
    Q_EMIT isLoadingChanged();
    m_hasError = false;
    Q_EMIT errorChanged();

    try {
        auto result = co_await m_neteaseClient->getAlbumDetail(albumId);

        m_isLoading = false;
        Q_EMIT isLoadingChanged();

        if (result.isError()) {
            m_error = ViewModelError::fromApiError(result.error());
            m_hasError = true;
            Q_EMIT errorChanged();
            co_return;
        }

        const QVector<Song> &songs = result.data();

        // Extract album metadata from first song if available
        if (!songs.isEmpty()) {
            m_headerName = songs.first().album;
            m_headerCoverUrl = songs.first().coverUrl.toString();
            Q_EMIT headerNameChanged();
            Q_EMIT headerCoverUrlChanged();
        }

        m_headerTrackCount = songs.size();
        Q_EMIT headerTrackCountChanged();

        m_songs->setSongs(songs);

        // Cache songs
        m_songRepo->saveBatch(songs);
    } catch (const std::exception &ex) {
        m_isLoading = false;
        Q_EMIT isLoadingChanged();
        m_error = ViewModelError(ViewModelError::ErrorType::Unknown, QString::fromUtf8(ex.what()));
        m_hasError = true;
        Q_EMIT errorChanged();
        Logger::get("viewmodel")->warn("Failed to load NetEase album: {}", ex.what());
    }
}

} // namespace QeriPlayerQt
