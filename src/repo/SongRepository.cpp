/// @file SongRepository.cpp
/// @brief SQLite-backed song repository implementation

#include "repo/SongRepository.h"

#include "core/database/DatabaseManager.h"
#include "core/logger/Logger.h"
#include "repo/SqlRowMapper.h"

namespace QeriPlayerQt {

SongRepository::SongRepository(DatabaseManager *db)
    : m_db(db)
{
}

std::optional<Song> SongRepository::findById(const QString &id)
{
    auto rows = m_db->exec("SELECT id, platform, name, artist, album, album_id, duration_ms, "
                           "cover_url, media_uri, custom_name, custom_artist, custom_cover_url, "
                           "original_name, original_artist, original_cover_url, "
                           "local_file_name, local_file_path, matched_lyric_source, matched_song_id, "
                           "user_lyric_offset_ms, lyrics_json, channel_id, audio_id, sub_audio_id, "
                           "extra_json, cached_at, last_played_at "
                           "FROM songs_cache WHERE id = ?",
                           {id});

    if (rows.isEmpty())
        return std::nullopt;
    return SqlRowMapper::toSong(rows[0]);
}

QVector<Song> SongRepository::findByIds(const QStringList &ids)
{
    if (ids.isEmpty())
        return {};

    QVector<Song> result;
    // Process in chunks to avoid overly long SQL
    constexpr int chunkSize = 100;
    for (int i = 0; i < ids.size(); i += chunkSize) {
        QStringList chunk = ids.mid(i, chunkSize);
        QString placeholders;
        for (int j = 0; j < chunk.size(); ++j) {
            if (j > 0)
                placeholders += ',';
            placeholders += '?';
        }

        QVariantList params;
        for (const auto &id : chunk) {
            params << id;
        }

        auto rows = m_db->exec("SELECT id, platform, name, artist, album, album_id, duration_ms, "
                               "cover_url, media_uri, custom_name, custom_artist, custom_cover_url, "
                               "original_name, original_artist, original_cover_url, "
                               "local_file_name, local_file_path, matched_lyric_source, matched_song_id, "
                               "user_lyric_offset_ms, lyrics_json, channel_id, audio_id, sub_audio_id, "
                               "extra_json, cached_at, last_played_at "
                               "FROM songs_cache WHERE id IN ("
                                   + placeholders + ")",
                               params);

        for (const auto &row : rows) {
            result.append(SqlRowMapper::toSong(row));
        }
    }

    return result;
}

void SongRepository::save(const Song &song)
{
    m_db->exec("INSERT OR REPLACE INTO songs_cache ("
               "id, platform, name, artist, album, album_id, duration_ms, "
               "cover_url, media_uri, custom_name, custom_artist, custom_cover_url, "
               "original_name, original_artist, original_cover_url, "
               "local_file_name, local_file_path, matched_lyric_source, matched_song_id, "
               "user_lyric_offset_ms, lyrics_json, channel_id, audio_id, sub_audio_id, extra_json"
               ") VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
               SqlRowMapper::songToInsertParams(song));
}

void SongRepository::saveBatch(const QVector<Song> &songs)
{
    if (songs.isEmpty())
        return;

    m_db->beginTransaction();
    try {
        for (const auto &song : songs) {
            save(song);
        }
        m_db->commitTransaction();
    } catch (...) {
        try {
            m_db->rollbackTransaction();
        } catch (const std::exception &rbEx) {
            Logger::get("repo")->warn("SongRepository: rollback failed: {}", rbEx.what());
        }
        throw;
    }
}

void SongRepository::remove(const QString &id)
{
    m_db->exec("DELETE FROM songs_cache WHERE id = ?", {id});
}

bool SongRepository::exists(const QString &id)
{
    auto rows = m_db->exec("SELECT 1 FROM songs_cache WHERE id = ?", {id});
    return !rows.isEmpty();
}

QVector<Song> SongRepository::findByPlatform(MusicPlatform platform)
{
    auto rows = m_db->exec("SELECT id, platform, name, artist, album, album_id, duration_ms, "
                           "cover_url, media_uri, custom_name, custom_artist, custom_cover_url, "
                           "original_name, original_artist, original_cover_url, "
                           "local_file_name, local_file_path, matched_lyric_source, matched_song_id, "
                           "user_lyric_offset_ms, lyrics_json, channel_id, audio_id, sub_audio_id, "
                           "extra_json, cached_at, last_played_at "
                           "FROM songs_cache WHERE platform = ?",
                           {SqlRowMapper::platformToString(platform)});

    QVector<Song> result;
    for (const auto &row : rows) {
        result.append(SqlRowMapper::toSong(row));
    }
    return result;
}

QVector<Song> SongRepository::search(const QString &query, int limit)
{
    if (limit <= 0)
        limit = 50;
    QString pattern = "%" + query + "%";
    auto rows = m_db->exec("SELECT id, platform, name, artist, album, album_id, duration_ms, "
                           "cover_url, media_uri, custom_name, custom_artist, custom_cover_url, "
                           "original_name, original_artist, original_cover_url, "
                           "local_file_name, local_file_path, matched_lyric_source, matched_song_id, "
                           "user_lyric_offset_ms, lyrics_json, channel_id, audio_id, sub_audio_id, "
                           "extra_json, cached_at, last_played_at "
                           "FROM songs_cache WHERE name LIKE ? OR artist LIKE ? OR album LIKE ? LIMIT ?",
                           {pattern, pattern, pattern, limit});

    QVector<Song> result;
    for (const auto &row : rows) {
        result.append(SqlRowMapper::toSong(row));
    }
    return result;
}

} // namespace QeriPlayerQt
