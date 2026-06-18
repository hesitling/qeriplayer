/// @file PlayHistoryRepository.cpp
/// @brief SQLite-backed play history repository implementation

#include "repo/PlayHistoryRepository.h"

#include "core/database/DatabaseManager.h"
#include "core/logger/Logger.h"
#include "repo/SqlRowMapper.h"

namespace QeriPlayerQt {

PlayHistoryRepository::PlayHistoryRepository(DatabaseManager *db)
    : m_db(db)
{
}

void PlayHistoryRepository::record(const QString &songId)
{
    m_db->beginTransaction();
    try {
        m_db->exec("INSERT INTO play_history (song_id) VALUES (?)", {songId});
        m_db->exec("UPDATE songs_cache SET last_played_at = CURRENT_TIMESTAMP WHERE id = ?", {songId});
        m_db->commitTransaction();
    } catch (...) {
        try {
            m_db->rollbackTransaction();
        } catch (const std::exception &rbEx) {
            Logger::get("repo")->warn("PlayHistoryRepository: rollback failed: {}", rbEx.what());
        }
        throw;
    }
}

QVector<Song> PlayHistoryRepository::recent(int limit)
{
    // Select distinct songs ordered by their most recent play time
    auto rows = m_db->exec("SELECT sc.id, sc.platform, sc.name, sc.artist, sc.album, sc.album_id, "
                           "sc.duration_ms, sc.cover_url, sc.media_uri, sc.custom_name, sc.custom_artist, "
                           "sc.custom_cover_url, sc.original_name, sc.original_artist, sc.original_cover_url, "
                           "sc.local_file_name, sc.local_file_path, sc.matched_lyric_source, sc.matched_song_id, "
                           "sc.user_lyric_offset_ms, sc.lyrics_json, sc.channel_id, sc.audio_id, sc.sub_audio_id, "
                           "sc.extra_json, sc.cached_at, sc.last_played_at "
                           "FROM songs_cache sc "
                           "INNER JOIN ("
                           "  SELECT song_id, MAX(played_at) AS latest "
                           "  FROM play_history GROUP BY song_id"
                           ") ph ON sc.id = ph.song_id "
                           "ORDER BY ph.latest DESC "
                           "LIMIT ?",
                           {limit});

    QVector<Song> result;
    for (const auto &row : rows) {
        result.append(SqlRowMapper::toSong(row));
    }
    return result;
}

void PlayHistoryRepository::clear()
{
    m_db->exec("DELETE FROM play_history");
}

void PlayHistoryRepository::remove(const QStringList &songIds)
{
    if (songIds.isEmpty())
        return;

    QString placeholders;
    for (int i = 0; i < songIds.size(); ++i) {
        if (i > 0)
            placeholders += ',';
        placeholders += '?';
    }

    QVariantList params;
    for (const auto &id : songIds) {
        params << id;
    }

    m_db->exec("DELETE FROM play_history WHERE song_id IN (" + placeholders + ")", params);
}

int PlayHistoryRepository::playCount(const QString &songId)
{
    auto rows = m_db->exec("SELECT COUNT(*) FROM play_history WHERE song_id = ?", {songId});

    if (rows.isEmpty())
        return 0;
    return rows[0][0].toInt();
}

} // namespace QeriPlayerQt
