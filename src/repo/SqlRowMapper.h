/// @file SqlRowMapper.h
/// @brief Converts between SQL rows and domain types

#ifndef QERIPLAYERQT_SQLROWMAPPER_H
#define QERIPLAYERQT_SQLROWMAPPER_H

#include "core/database/DatabaseManager.h"
#include "domain/PersistedPlayerState.h"
#include "domain/PlaylistSummary.h"
#include "domain/Song.h"

#include <QVariant>

namespace QeriPlayerQt {

/**
 * @brief Utility for converting between SQL query rows and domain types
 */
class SqlRowMapper {
public:
    /**
     * @brief Convert a query row to a Song
     * @param row Row from a SELECT * on songs_cache (27 columns)
     * @return Populated Song struct
     */
    static Song toSong(const QueryRow &row);

    /**
     * @brief Get positional bind parameters for INSERT OR REPLACE into songs_cache
     * @param song Song to serialize
     * @return QVariantList of 25 values (id through extra_json)
     */
    static QVariantList songToInsertParams(const Song &song);

    /**
     * @brief Get positional bind parameters for UPDATE songs_cache SET ... WHERE id = ?
     * @param song Song to serialize
     * @return QVariantList of 25 values (all columns except id, then id last for WHERE)
     */
    static QVariantList songToUpdateParams(const Song &song);

    /**
     * @brief Convert a query row to a PlaylistSummary
     * @param row Row from a SELECT * on playlists
     * @return Populated PlaylistSummary struct
     */
    static PlaylistSummary toPlaylistSummary(const QueryRow &row);

    /**
     * @brief Serialize PersistedPlayerState to a QVariantMap for JSON storage
     * @param state Player state to serialize
     * @return QVariantMap suitable for storing in player_state columns
     */
    static QVariantMap playerStateToJson(const PersistedPlayerState &state);

    /**
     * @brief Deserialize PersistedPlayerState from a QVariantMap
     * @param map Map from player_state row columns
     * @return Populated PersistedPlayerState
     */
    static PersistedPlayerState playerStateFromJson(const QVariantMap &map);

    /**
     * @brief Convert a MusicPlatform enum to its string representation
     */
    static QString platformToString(MusicPlatform p);

    /**
     * @brief Convert a string to a MusicPlatform enum
     */
    static MusicPlatform stringToPlatform(const QString &s);

private:
    SqlRowMapper() = default;

    static QVariant songToJsonField(const Song &song, const QString &field);
    static void fillSongFromJson(Song &song, const QString &field, const QVariant &value);
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_SQLROWMAPPER_H
