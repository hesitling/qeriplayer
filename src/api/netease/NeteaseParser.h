/// @file NeteaseParser.h
/// @brief JSON-to-domain parsing for NetEase API responses

#ifndef QERIPLAYERQT_NETEASEPARSER_H
#define QERIPLAYERQT_NETEASEPARSER_H

#include "api/common/LoginResult.h"
#include "domain/Album.h"
#include "domain/Artist.h"
#include "domain/Lyrics.h"
#include "domain/Playlist.h"
#include "domain/SearchResult.h"
#include "domain/Song.h"
#include "domain/SongUrlResult.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVector>

namespace QeriPlayerQt {

/**
 * @brief Static parser methods for NetEase API JSON responses
 *
 * Each method takes a QJsonObject (or QJsonDocument) and returns
 * the corresponding domain type. Parsing never throws — malformed
 * JSON results in empty/default values with a logged warning.
 */
class NeteaseParser {
public:
    // Song parsing
    static Song parseSong(const QJsonObject &json);
    static QVector<Song> parseSongs(const QJsonArray &array);

    // Album parsing
    static Album parseAlbum(const QJsonObject &json);
    static Album parseAlbumWithSongs(const QJsonObject &json, QVector<Song> &songsOut);

    // Artist parsing
    static Artist parseArtist(const QJsonObject &json);
    static QVector<Artist> parseArtists(const QJsonArray &array);

    // Playlist parsing
    static Playlist parsePlaylist(const QJsonObject &json);
    static Playlist parsePlaylistDetail(const QJsonObject &json);

    // Lyrics parsing
    /// @note The LRC metadata header at [00:00.00] (e.g. "Song - Artist") is included
    ///       as the first LyricLine. UI consumers may choose to display it differently.
    static Lyrics parseLyrics(const QJsonObject &json);

    // Search result parsing
    static SearchResult parseSearchResult(const QJsonObject &json, SearchType type);

    // Auth parsing
    static LoginResult parseLoginResult(const QJsonObject &json);

    // Song URL parsing
    static SongUrlResult parseSongUrl(const QJsonObject &json);

private:
    static void logMalformed(const QString &method, const QString &detail);
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_NETEASEPARSER_H
