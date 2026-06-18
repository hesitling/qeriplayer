/// @file ISongRepository.h
/// @brief Interface for song persistence operations

#ifndef QERIPLAYERQT_ISONGREPOSITORY_H
#define QERIPLAYERQT_ISONGREPOSITORY_H

#include "domain/Enums.h"
#include "domain/Song.h"

#include <QString>
#include <QStringList>
#include <QVector>

#include <optional>

namespace QeriPlayerQt {

/**
 * @brief Abstract interface for song CRUD operations on songs_cache
 */
class ISongRepository {
public:
    virtual ~ISongRepository() = default;

    /**
     * @brief Find a song by its ID
     * @return Song if found, empty optional otherwise
     */
    virtual std::optional<Song> findById(const QString &id) = 0;

    /**
     * @brief Find multiple songs by their IDs
     * @return Songs that exist (missing IDs silently omitted)
     */
    virtual QVector<Song> findByIds(const QStringList &ids) = 0;

    /**
     * @brief Insert or update a song (INSERT OR REPLACE)
     */
    virtual void save(const Song &song) = 0;

    /**
     * @brief Insert or update multiple songs in a single transaction
     */
    virtual void saveBatch(const QVector<Song> &songs) = 0;

    /**
     * @brief Delete a song by ID (cascades to playlist_songs)
     */
    virtual void remove(const QString &id) = 0;

    /**
     * @brief Check if a song exists by ID
     */
    virtual bool exists(const QString &id) = 0;

    /**
     * @brief Find all songs matching a platform
     */
    virtual QVector<Song> findByPlatform(MusicPlatform platform) = 0;

    /**
     * @brief Search songs by substring match on name, artist, or album
     * @param query Search text (case-insensitive)
     * @param limit Maximum results (default 50)
     */
    virtual QVector<Song> search(const QString &query, int limit = 50) = 0;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_ISONGREPOSITORY_H
