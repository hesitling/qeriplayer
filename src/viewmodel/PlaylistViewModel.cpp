/// @file PlaylistViewModel.cpp
/// @brief Implementation of PlaylistViewModel

#include "viewmodel/PlaylistViewModel.h"

#include "core/logger/Logger.h"

#include <QJsonArray>
#include <QJsonObject>

namespace QeriPlayerQt {

PlaylistViewModel::PlaylistViewModel(IPlaylistRepository *playlistRepo, NeteaseClient *neteaseClient, QObject *parent)
    : QObject(parent)
    , m_playlistRepo(playlistRepo)
    , m_neteaseClient(neteaseClient)
{
}

PlaylistViewModel::~PlaylistViewModel() = default;

// --- Getters ---

QVariantList PlaylistViewModel::localPlaylists() const
{
    return m_localPlaylists;
}

QVariantList PlaylistViewModel::neteasePlaylists() const
{
    return m_neteasePlaylists;
}

QVariantList PlaylistViewModel::neteaseAlbums() const
{
    return m_neteaseAlbums;
}

bool PlaylistViewModel::isLoading() const
{
    return m_isLoading;
}

bool PlaylistViewModel::hasError() const
{
    return m_hasError;
}

ViewModelError PlaylistViewModel::error() const
{
    return m_error;
}

// --- Loading ---

void PlaylistViewModel::loadLocalPlaylists()
{
    try {
        auto summaries = m_playlistRepo->findAll();

        QVariantList list;
        for (const auto &summary : summaries) {
            list.append(QVariant::fromValue(summary));
        }

        m_localPlaylists = list;
        Q_EMIT localPlaylistsChanged();
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to load local playlists: {}", ex.what());
    }
}

void PlaylistViewModel::loadNeteasePlaylists()
{
    loadNeteasePlaylistsImpl();
}

void PlaylistViewModel::loadNeteaseAlbums()
{
    loadNeteaseAlbumsImpl();
}

// --- Local playlist CRUD ---

void PlaylistViewModel::createLocalPlaylist(const QString &name)
{
    try {
        m_playlistRepo->create(name);
        loadLocalPlaylists();
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to create local playlist: {}", ex.what());
    }
}

void PlaylistViewModel::deleteLocalPlaylist(const QString &id)
{
    try {
        m_playlistRepo->remove(id);
        loadLocalPlaylists();
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to delete local playlist: {}", ex.what());
    }
}

void PlaylistViewModel::renameLocalPlaylist(const QString &id, const QString &name)
{
    try {
        // Get existing playlist to preserve other metadata
        auto playlist = m_playlistRepo->findById(id);
        if (playlist.has_value()) {
            m_playlistRepo->updateMetadata(id, name, playlist->description, playlist->coverUrl.toString());
            loadLocalPlaylists();
        }
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to rename local playlist: {}", ex.what());
    }
}

// --- Error ---

void PlaylistViewModel::clearError()
{
    m_hasError = false;
    m_error = ViewModelError();
    Q_EMIT errorChanged();
}

// --- Private ---

QCoro::Task<void> PlaylistViewModel::loadNeteasePlaylistsImpl()
{
    m_isLoading = true;
    Q_EMIT isLoadingChanged();
    clearError();

    try {
        auto result = co_await m_neteaseClient->getUserPlaylists(QString()); // Empty = current user

        m_isLoading = false;
        Q_EMIT isLoadingChanged();

        if (result.isError()) {
            m_error = ViewModelError::fromApiError(result.error());
            m_hasError = true;
            Q_EMIT errorChanged();
            co_return;
        }

        QVariantList list;
        for (const auto &playlist : result.data()) {
            PlaylistSummary summary;
            summary.id = playlist.id;
            summary.name = playlist.name;
            summary.coverUrl = playlist.coverUrl;
            summary.trackCount = playlist.songCount;
            list.append(QVariant::fromValue(summary));
        }

        m_neteasePlaylists = list;
        Q_EMIT neteasePlaylistsChanged();
    } catch (const std::exception &ex) {
        m_isLoading = false;
        Q_EMIT isLoadingChanged();
        m_error = ViewModelError(ViewModelError::ErrorType::Unknown, QString::fromUtf8(ex.what()));
        m_hasError = true;
        Q_EMIT errorChanged();
        Logger::get("viewmodel")->warn("Failed to load NetEase playlists: {}", ex.what());
    }
}

QCoro::Task<void> PlaylistViewModel::loadNeteaseAlbumsImpl()
{
    m_isLoading = true;
    Q_EMIT isLoadingChanged();
    clearError();

    try {
        auto result = co_await m_neteaseClient->getUserStarredAlbums(QString()); // Empty = current user

        m_isLoading = false;
        Q_EMIT isLoadingChanged();

        if (result.isError()) {
            m_error = ViewModelError::fromApiError(result.error());
            m_hasError = true;
            Q_EMIT errorChanged();
            co_return;
        }

        // Parse albums from response
        QJsonArray playlistArray = result.data().value(QStringLiteral("playlist")).toArray();
        QVariantList list;
        for (const QJsonValue &val : playlistArray) {
            QJsonObject obj = val.toObject();
            PlaylistSummary summary;
            summary.id = obj.value(QStringLiteral("id")).toVariant().toString();
            summary.name = obj.value(QStringLiteral("name")).toString();
            summary.coverUrl = QUrl(obj.value(QStringLiteral("coverImgUrl")).toString());
            summary.trackCount = obj.value(QStringLiteral("trackCount")).toInt();
            list.append(QVariant::fromValue(summary));
        }
        m_neteaseAlbums = list;
        Q_EMIT neteaseAlbumsChanged();
    } catch (const std::exception &ex) {
        m_isLoading = false;
        Q_EMIT isLoadingChanged();
        m_error = ViewModelError(ViewModelError::ErrorType::Unknown, QString::fromUtf8(ex.what()));
        m_hasError = true;
        Q_EMIT errorChanged();
        Logger::get("viewmodel")->warn("Failed to load NetEase albums: {}", ex.what());
    }
}

} // namespace QeriPlayerQt
