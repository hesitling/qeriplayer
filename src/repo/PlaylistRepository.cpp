/// @file PlaylistRepository.cpp
/// @brief SQLite-backed playlist repository implementation

#include "repo/PlaylistRepository.h"

#include "core/database/DatabaseManager.h"
#include "core/logger/Logger.h"
#include "repo/SqlRowMapper.h"

#include <QDateTime>

namespace QeriPlayerQt {

PlaylistRepository::PlaylistRepository(DatabaseManager *db)
    : m_db(db)
{
}

QVector<PlaylistSummary> PlaylistRepository::findAll()
{
    auto rows = m_db->exec("SELECT id, platform, name, description, cover_url, song_count, owner, "
                           "custom_cover_url, modified_at "
                           "FROM playlists ORDER BY modified_at DESC");

    QVector<PlaylistSummary> result;
    for (const auto &row : rows) {
        result.append(SqlRowMapper::toPlaylistSummary(row));
    }
    return result;
}

std::optional<Playlist> PlaylistRepository::findById(const QString &id)
{
    auto plRows = m_db->exec("SELECT id, platform, name, description, cover_url, song_count, owner, "
                             "custom_cover_url, modified_at "
                             "FROM playlists WHERE id = ?",
                             {id});

    if (plRows.isEmpty())
        return std::nullopt;

    const auto &row = plRows[0];
    Playlist pl;
    pl.id = row[0].toString();
    pl.platform = SqlRowMapper::stringToPlatform(row[1].toString());
    pl.name = row[2].toString();
    pl.description = row[3].toString();
    pl.coverUrl = QUrl(row[4].toString());
    pl.songCount = row[5].toInt();
    pl.owner = row[6].toString();
    pl.customCoverUrl = row[7].toString();
    pl.modifiedAt = row[8].toLongLong();

    // Load songs via JOIN with playlist_songs
    auto songRows = m_db->exec("SELECT sc.id, sc.platform, sc.name, sc.artist, sc.album, sc.album_id, "
                               "sc.duration_ms, sc.cover_url, sc.media_uri, sc.custom_name, sc.custom_artist, "
                               "sc.custom_cover_url, sc.original_name, sc.original_artist, sc.original_cover_url, "
                               "sc.local_file_name, sc.local_file_path, sc.matched_lyric_source, sc.matched_song_id, "
                               "sc.user_lyric_offset_ms, sc.lyrics_json, sc.channel_id, sc.audio_id, sc.sub_audio_id, "
                               "sc.extra_json, sc.cached_at, sc.last_played_at "
                               "FROM songs_cache sc "
                               "INNER JOIN playlist_songs ps ON sc.id = ps.song_id "
                               "WHERE ps.playlist_id = ? "
                               "ORDER BY ps.position",
                               {id});

    for (const auto &srow : songRows) {
        pl.songs.append(SqlRowMapper::toSong(srow));
    }

    return pl;
}

Playlist PlaylistRepository::create(const QString &name, MusicPlatform platform)
{
    Playlist pl;
    pl.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    pl.name = name;
    pl.platform = platform;
    pl.modifiedAt = QDateTime::currentMSecsSinceEpoch();

    m_db->exec("INSERT INTO playlists (id, platform, name, modified_at) VALUES (?, ?, ?, ?)",
               {pl.id, SqlRowMapper::platformToString(platform), pl.name, pl.modifiedAt});

    return pl;
}

void PlaylistRepository::updateMetadata(const QString &id, const QString &name, const QString &description,
                                        const QString &coverUrl)
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    m_db->exec("UPDATE playlists SET name = ?, description = ?, cover_url = ?, modified_at = ? WHERE id = ?",
               {name, description, coverUrl, now, id});
}

void PlaylistRepository::remove(const QString &id)
{
    // ON DELETE CASCADE handles playlist_songs cleanup
    m_db->exec("DELETE FROM playlists WHERE id = ?", {id});
}

bool PlaylistRepository::addSong(const QString &playlistId, const QString &songId, int position)
{
    // Check for duplicate
    auto existing
        = m_db->exec("SELECT 1 FROM playlist_songs WHERE playlist_id = ? AND song_id = ?", {playlistId, songId});
    if (!existing.isEmpty())
        return false;

    m_db->beginTransaction();
    try {
        if (position < 0) {
            // Append at end
            auto countRows = m_db->exec("SELECT COALESCE(MAX(position), -1) FROM playlist_songs WHERE playlist_id = ?",
                                        {playlistId});
            position = countRows[0][0].toInt() + 1;
        } else {
            // Shift existing songs at position >= insert position
            m_db->exec("UPDATE playlist_songs SET position = position + 1 "
                       "WHERE playlist_id = ? AND position >= ?",
                       {playlistId, position});
        }

        m_db->exec("INSERT INTO playlist_songs (playlist_id, song_id, position) VALUES (?, ?, ?)",
                   {playlistId, songId, position});

        updateSongCount(playlistId);
        m_db->commitTransaction();
    } catch (...) {
        try {
            m_db->rollbackTransaction();
        } catch (const std::exception &rbEx) {
            Logger::get("repo")->warn("PlaylistRepository: rollback failed: {}", rbEx.what());
        }
        throw;
    }
    return true;
}

void PlaylistRepository::removeSong(const QString &playlistId, const QString &songId)
{
    // Get the position of the song being removed
    auto posRows
        = m_db->exec("SELECT position FROM playlist_songs WHERE playlist_id = ? AND song_id = ?", {playlistId, songId});

    if (posRows.isEmpty())
        return;

    int removedPos = posRows[0][0].toInt();

    m_db->beginTransaction();
    try {
        m_db->exec("DELETE FROM playlist_songs WHERE playlist_id = ? AND song_id = ?", {playlistId, songId});

        // Shift down songs that were after the removed one
        m_db->exec("UPDATE playlist_songs SET position = position - 1 "
                   "WHERE playlist_id = ? AND position > ?",
                   {playlistId, removedPos});

        updateSongCount(playlistId);
        m_db->commitTransaction();
    } catch (...) {
        try {
            m_db->rollbackTransaction();
        } catch (const std::exception &rbEx) {
            Logger::get("repo")->warn("PlaylistRepository: rollback failed: {}", rbEx.what());
        }
        throw;
    }
}

void PlaylistRepository::reorderSongs(const QString &playlistId, const QStringList &songIds)
{
    m_db->beginTransaction();
    try {
        for (int i = 0; i < songIds.size(); ++i) {
            m_db->exec("UPDATE playlist_songs SET position = ? WHERE playlist_id = ? AND song_id = ?",
                       {i, playlistId, songIds[i]});
        }
        m_db->commitTransaction();
    } catch (...) {
        try {
            m_db->rollbackTransaction();
        } catch (const std::exception &rbEx) {
            Logger::get("repo")->warn("PlaylistRepository: rollback failed: {}", rbEx.what());
        }
        throw;
    }
}

int PlaylistRepository::songCount(const QString &playlistId)
{
    auto rows = m_db->exec("SELECT song_count FROM playlists WHERE id = ?", {playlistId});

    if (rows.isEmpty())
        return 0;
    return rows[0][0].toInt();
}

void PlaylistRepository::updateSongCount(const QString &playlistId)
{
    m_db->exec("UPDATE playlists SET song_count = "
               "(SELECT COUNT(*) FROM playlist_songs WHERE playlist_id = ?) "
               "WHERE id = ?",
               {playlistId, playlistId});
}

} // namespace QeriPlayerQt
