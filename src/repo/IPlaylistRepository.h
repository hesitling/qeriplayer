/// @file IPlaylistRepository.h
/// @brief Interface for playlist persistence operations

#ifndef QERIPLAYERQT_IPLAYLISTREPOSITORY_H
#define QERIPLAYERQT_IPLAYLISTREPOSITORY_H

#include "domain/Playlist.h"
#include "domain/PlaylistSummary.h"

#include <QString>
#include <QStringList>
#include <QVector>

#include <optional>

namespace QeriPlayerQt {

/**
 * @brief Abstract interface for playlist CRUD operations on playlists/playlist_songs
 */
class IPlaylistRepository {
public:
    virtual ~IPlaylistRepository() = default;

    /**
     * @brief List all playlists as lightweight summaries (no songs)
     * @return Ordered by modified_at descending
     */
    virtual QVector<PlaylistSummary> findAll() = 0;

    /**
     * @brief Load a full playlist with its songs
     * @return Playlist with songs populated in position order, empty optional if not found
     */
    virtual std::optional<Playlist> findById(const QString &id) = 0;

    /**
     * @brief Create a new playlist
     * @param name Playlist name
     * @param platform Optional platform tag
     * @return The created playlist (with generated ID)
     */
    virtual Playlist create(const QString &name, MusicPlatform platform = MusicPlatform::Unknown) = 0;

    /**
     * @brief Update playlist name, description, and cover URL
     */
    virtual void updateMetadata(const QString &id, const QString &name, const QString &description,
                                const QString &coverUrl) = 0;

    /**
     * @brief Delete a playlist and its song associations
     */
    virtual void remove(const QString &id) = 0;

    /**
     * @brief Add a song to a playlist
     * @param position Position to insert (-1 = append)
     * @return true if added, false if song already in playlist
     */
    virtual bool addSong(const QString &playlistId, const QString &songId, int position = -1) = 0;

    /**
     * @brief Remove a song from a playlist
     */
    virtual void removeSong(const QString &playlistId, const QString &songId) = 0;

    /**
     * @brief Reorder all songs in a playlist
     * @param songIds Ordered list of song IDs (songs not in list are appended)
     */
    virtual void reorderSongs(const QString &playlistId, const QStringList &songIds) = 0;

    /**
     * @brief Get the number of songs in a playlist
     */
    virtual int songCount(const QString &playlistId) = 0;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_IPLAYLISTREPOSITORY_H
