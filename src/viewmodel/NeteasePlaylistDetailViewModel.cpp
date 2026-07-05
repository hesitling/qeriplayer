/// @file NeteasePlaylistDetailViewModel.cpp
/// @brief Implementation of NeteasePlaylistDetailViewModel

#include "viewmodel/NeteasePlaylistDetailViewModel.h"

#include "core/logger/Logger.h"

#include <QPointer>

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
    m_pendingTask = QCoro::QmlTask(loadPlaylistTask(QPointer<NeteasePlaylistDetailViewModel>(this), playlistId));
    return m_pendingTask;
}

QCoro::QmlTask NeteasePlaylistDetailViewModel::loadAlbum(const QString &albumId)
{
    m_lastAlbumId = albumId;
    m_lastPlaylistId.clear();
    m_isAlbum = true;
    m_pendingTask = QCoro::QmlTask(loadAlbumTask(QPointer<NeteasePlaylistDetailViewModel>(this), albumId));
    return m_pendingTask;
}

QCoro::Task<void> NeteasePlaylistDetailViewModel::loadPlaylistTask(QPointer<NeteasePlaylistDetailViewModel> self,
                                                                   QString playlistId)
{
    if (!self) {
        co_return;
    }

    self->beginLoad();

    try {
        if (!self->ensureClientAvailable()) {
            co_return;
        }

        NeteaseClient *client = self->m_neteaseClient;
        auto result = co_await client->getPlaylistDetail(playlistId);
        if (!self || !self->finalizeLoad(result)) {
            co_return;
        }

        self->applyPlaylist(result.data());
    } catch (const std::exception &ex) {
        if (!self) {
            co_return;
        }
        self->handleLoadException(ex, "playlist");
    }
}

QCoro::Task<void> NeteasePlaylistDetailViewModel::loadAlbumTask(QPointer<NeteasePlaylistDetailViewModel> self,
                                                                QString albumId)
{
    if (!self) {
        co_return;
    }

    self->beginLoad();

    try {
        if (!self->ensureClientAvailable()) {
            co_return;
        }

        NeteaseClient *client = self->m_neteaseClient;
        auto result = co_await client->getAlbumDetail(albumId);
        if (!self || !self->finalizeLoad(result)) {
            co_return;
        }

        self->applyAlbumSongs(result.data());
    } catch (const std::exception &ex) {
        if (!self) {
            co_return;
        }
        self->handleLoadException(ex, "album");
    }
}

QCoro::Task<void> NeteasePlaylistDetailViewModel::saveToLocalTask(QPointer<NeteasePlaylistDetailViewModel> self)
{
    if (!self || self->m_headerName.isEmpty()) {
        co_return;
    }

    try {
        Playlist localPlaylist = self->m_playlistRepo->create(self->m_headerName);
        self->m_songRepo->saveBatch(self->m_songs->songs());

        for (const Song &song : self->m_songs->songs()) {
            if (!self) {
                co_return;
            }
            self->m_playlistRepo->addSong(localPlaylist.id, song.id);
        }
    } catch (const std::exception &ex) {
        if (!self) {
            co_return;
        }
        Logger::get("viewmodel")->warn("Failed to save playlist to local: {}", ex.what());
        self->setErrorState(ViewModelError(ViewModelError::ErrorType::Unknown, QString::fromUtf8(ex.what())));
    }
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
        return loadAlbum(m_lastAlbumId);
    }

    if (m_lastPlaylistId.isEmpty()) {
        return QCoro::QmlTask(completedTask());
    }
    return loadPlaylist(m_lastPlaylistId);
}

QCoro::QmlTask NeteasePlaylistDetailViewModel::saveToLocal()
{
    m_pendingTask = QCoro::QmlTask(saveToLocalTask(QPointer<NeteasePlaylistDetailViewModel>(this)));
    return m_pendingTask;
}

void NeteasePlaylistDetailViewModel::beginLoad()
{
    m_isLoading = true;
    Q_EMIT isLoadingChanged();
    m_hasError = false;
    m_error = ViewModelError();
    Q_EMIT errorChanged();
}

bool NeteasePlaylistDetailViewModel::ensureClientAvailable()
{
    if (m_neteaseClient != nullptr) {
        return true;
    }

    m_isLoading = false;
    Q_EMIT isLoadingChanged();
    setErrorState(ViewModelError(ViewModelError::ErrorType::Api, QStringLiteral("NetEase client not available")));
    return false;
}

void NeteasePlaylistDetailViewModel::setErrorState(const ViewModelError &error)
{
    m_error = error;
    m_hasError = true;
    Q_EMIT errorChanged();
}

void NeteasePlaylistDetailViewModel::handleLoadException(const std::exception &ex, const char *entityName)
{
    m_isLoading = false;
    Q_EMIT isLoadingChanged();
    setErrorState(ViewModelError(ViewModelError::ErrorType::Unknown, QString::fromUtf8(ex.what())));
    Logger::get("viewmodel")->warn("Failed to load NetEase {}: {}", entityName, ex.what());
}

void NeteasePlaylistDetailViewModel::applyPlaylist(const Playlist &playlist)
{
    m_headerName = playlist.name;
    m_headerCoverUrl = playlist.coverUrl.toString();
    m_headerTrackCount = playlist.songCount;

    Q_EMIT headerNameChanged();
    Q_EMIT headerCoverUrlChanged();
    Q_EMIT headerTrackCountChanged();

    m_songs->setSongs(playlist.songs);
    m_songRepo->saveBatch(playlist.songs);
}

void NeteasePlaylistDetailViewModel::applyAlbumSongs(const QVector<Song> &songs)
{
    if (!songs.isEmpty()) {
        m_headerName = songs.first().album;
        m_headerCoverUrl = songs.first().coverUrl.toString();
    } else {
        m_headerName.clear();
        m_headerCoverUrl.clear();
    }

    m_headerTrackCount = songs.size();

    Q_EMIT headerNameChanged();
    Q_EMIT headerCoverUrlChanged();
    Q_EMIT headerTrackCountChanged();

    m_songs->setSongs(songs);
    m_songRepo->saveBatch(songs);
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

} // namespace QeriPlayerQt
