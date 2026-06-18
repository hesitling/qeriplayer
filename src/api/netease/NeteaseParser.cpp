/// @file NeteaseParser.cpp
/// @brief JSON-to-domain parsing for NetEase API responses

#include "api/netease/NeteaseParser.h"

#include "core/logger/Logger.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace QeriPlayerQt {

void NeteaseParser::logMalformed(const QString &method, const QString &detail)
{
    Logger::get("api")->warn("NeteaseParser::{}: malformed JSON — {}", method.toStdString(), detail.toStdString());
}

// ─── Song ───────────────────────────────────────────────────────────────────

Song NeteaseParser::parseSong(const QJsonObject &json)
{
    Song song;

    // NetEase wraps songs differently depending on endpoint:
    // - /song/detail: { songs: [{...}] }
    // - /cloudsearch/pc: result.songs[{...}]
    // - Direct song object when already unwrapped
    const QJsonArray songsArr = json.value(QLatin1String("songs")).toArray();
    const QJsonObject &songJson = (!songsArr.isEmpty()) ? songsArr.first().toObject() : json;

    if (songsArr.size() > 1) {
        logMalformed(QStringLiteral("parseSong"),
                     QStringLiteral("songs array has %1 entries, using first only — use parseSongs() instead")
                         .arg(songsArr.size()));
    }

    song.id = QString::number(songJson[QLatin1String("id")].toInteger());
    song.name = songJson[QLatin1String("name")].toString();
    song.platform = MusicPlatform::NetEase;

    // Artists — "ar" array or "artists" array
    const QJsonArray artists = songJson.contains(QLatin1String("ar")) ? songJson[QLatin1String("ar")].toArray()
                                                                      : songJson[QLatin1String("artists")].toArray();

    if (!artists.isEmpty()) {
        QStringList names;
        for (const auto &a : artists) {
            QJsonObject artistObj = a.toObject();
            names.append(artistObj[QLatin1String("name")].toString());
        }
        song.artist = names.join(QStringLiteral(", "));
    }

    // Album — "al" object or "album" object
    const QJsonObject albumObj = songJson.contains(QLatin1String("al")) ? songJson[QLatin1String("al")].toObject()
                                                                        : songJson[QLatin1String("album")].toObject();

    song.album = albumObj[QLatin1String("name")].toString();
    song.albumId = QString::number(albumObj[QLatin1String("id")].toInteger());
    song.coverUrl = QUrl(albumObj[QLatin1String("picUrl")].toString());

    // Duration
    song.durationMs = songJson[QLatin1String("dt")].toInteger();
    if (song.durationMs == 0) {
        song.durationMs = songJson[QLatin1String("duration")].toInteger();
    }

    if (song.id.isEmpty() || song.id == QLatin1String("0")) {
        logMalformed(QStringLiteral("parseSong"), QStringLiteral("missing or zero id"));
    }

    return song;
}

QVector<Song> NeteaseParser::parseSongs(const QJsonArray &array)
{
    QVector<Song> songs;
    songs.reserve(array.size());
    for (const auto &item : array) {
        songs.append(parseSong(item.toObject()));
    }
    return songs;
}

// ─── Album ──────────────────────────────────────────────────────────────────

Album NeteaseParser::parseAlbum(const QJsonObject &json)
{
    Album album;

    // Unwrap if needed: { album: {...} }
    const QJsonObject &albumJson
        = json.contains(QLatin1String("album")) ? json[QLatin1String("album")].toObject() : json;

    album.id = QString::number(albumJson[QLatin1String("id")].toInteger());
    album.name = albumJson[QLatin1String("name")].toString();
    album.platform = MusicPlatform::NetEase;

    // Artist
    if (albumJson.contains(QLatin1String("artist"))) {
        album.artist = albumJson[QLatin1String("artist")].toObject()[QLatin1String("name")].toString();
    } else if (albumJson.contains(QLatin1String("artists"))) {
        const QJsonArray artists = albumJson[QLatin1String("artists")].toArray();
        QStringList names;
        for (const auto &a : artists) {
            names.append(a.toObject()[QLatin1String("name")].toString());
        }
        album.artist = names.join(QStringLiteral(", "));
    }

    album.coverUrl = QUrl(albumJson[QLatin1String("picUrl")].toString());
    album.size = albumJson[QLatin1String("size")].toInt();

    return album;
}

Album NeteaseParser::parseAlbumWithSongs(const QJsonObject &json, QVector<Song> &songsOut)
{
    Album album = parseAlbum(json);

    if (json.contains(QLatin1String("songs"))) {
        songsOut = parseSongs(json[QLatin1String("songs")].toArray());
    }

    return album;
}

// ─── Artist ─────────────────────────────────────────────────────────────────

Artist NeteaseParser::parseArtist(const QJsonObject &json)
{
    Artist artist;

    // Unwrap if needed: { artist: {...} }
    const QJsonObject &artistJson
        = json.contains(QLatin1String("artist")) ? json[QLatin1String("artist")].toObject() : json;

    artist.id = QString::number(artistJson[QLatin1String("id")].toInteger());
    artist.name = artistJson[QLatin1String("name")].toString();
    artist.avatarUrl = QUrl(artistJson[QLatin1String("picUrl")].toString());
    artist.description = artistJson[QLatin1String("briefDesc")].toString();
    artist.platform = MusicPlatform::NetEase;

    return artist;
}

QVector<Artist> NeteaseParser::parseArtists(const QJsonArray &array)
{
    QVector<Artist> artists;
    artists.reserve(array.size());
    for (const auto &item : array) {
        artists.append(parseArtist(item.toObject()));
    }
    return artists;
}

// ─── Playlist ───────────────────────────────────────────────────────────────

Playlist NeteaseParser::parsePlaylist(const QJsonObject &json)
{
    Playlist playlist;

    // Unwrap if needed: { playlist: {...} }
    const QJsonObject &plJson
        = json.contains(QLatin1String("playlist")) ? json[QLatin1String("playlist")].toObject() : json;

    playlist.id = QString::number(plJson[QLatin1String("id")].toInteger());
    playlist.name = plJson[QLatin1String("name")].toString();
    playlist.description = plJson[QLatin1String("description")].toString();
    playlist.coverUrl = QUrl(plJson[QLatin1String("coverImgUrl")].toString());
    playlist.songCount = plJson[QLatin1String("trackCount")].toInt();
    playlist.platform = MusicPlatform::NetEase;

    // Owner
    if (plJson.contains(QLatin1String("creator"))) {
        playlist.owner = plJson[QLatin1String("creator")].toObject()[QLatin1String("nickname")].toString();
    }

    // Modified timestamp
    playlist.modifiedAt = plJson[QLatin1String("updateTime")].toInteger();

    return playlist;
}

Playlist NeteaseParser::parsePlaylistDetail(const QJsonObject &json)
{
    Playlist playlist = parsePlaylist(json);

    const QJsonObject &plJson
        = json.contains(QLatin1String("playlist")) ? json[QLatin1String("playlist")].toObject() : json;

    if (plJson.contains(QLatin1String("tracks"))) {
        playlist.songs = parseSongs(plJson[QLatin1String("tracks")].toArray());
    }

    return playlist;
}

// ─── Lyrics ─────────────────────────────────────────────────────────────────

Lyrics NeteaseParser::parseLyrics(const QJsonObject &json)
{
    Lyrics lyrics;

    // NetEase lyrics response: { lrc: { lyric: "..." }, tlyric: { lyric: "..." } }
    if (json.contains(QLatin1String("lrc"))) {
        lyrics.rawText = json[QLatin1String("lrc")].toObject()[QLatin1String("lyric")].toString();
    }

    // Parse LRC format: [mm:ss.xx]text
    // Also handles word-level timing from YRC if available
    const QString &rawLyrics = lyrics.rawText;
    const QStringList lines = rawLyrics.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString &line : lines) {
        // Match [mm:ss.xx] or [mm:ss.xxx]
        thread_local const QRegularExpression timeRegex(QStringLiteral(R"(\[(\d{2}):(\d{2})\.(\d{2,3})\](.*))"));
        QRegularExpressionMatch match = timeRegex.match(line);

        if (match.hasMatch()) {
            int minutes = match.captured(1).toInt();
            int seconds = match.captured(2).toInt();
            int millis = match.captured(3).toInt();
            if (match.captured(3).size() == 2) {
                millis *= 10; // Convert centiseconds to milliseconds
            }

            qint64 startTime = minutes * 60000LL + seconds * 1000LL + millis;
            QString text = match.captured(4).trimmed();

            if (!text.isEmpty()) {
                LyricLine lyricLine;
                lyricLine.startTimeMs = startTime;
                lyricLine.text = text;
                lyrics.lines.append(lyricLine);
            }
        }
    }

    // Sort by start time
    std::sort(lyrics.lines.begin(), lyrics.lines.end(),
              [](const LyricLine &a, const LyricLine &b) { return a.startTimeMs < b.startTimeMs; });

    // Set end times (each line ends when the next starts, or +5s for last)
    for (int i = 0; i < lyrics.lines.size(); ++i) {
        if (i + 1 < lyrics.lines.size()) {
            lyrics.lines[i].endTimeMs = lyrics.lines[i + 1].startTimeMs;
        } else {
            lyrics.lines[i].endTimeMs = lyrics.lines[i].startTimeMs + 5000;
        }
    }

    return lyrics;
}

// ─── Search Result ──────────────────────────────────────────────────────────

SearchResult NeteaseParser::parseSearchResult(const QJsonObject &json, SearchType type)
{
    SearchResult result;

    const QJsonObject &resultJson
        = json.contains(QLatin1String("result")) ? json[QLatin1String("result")].toObject() : json;

    result.totalCount = resultJson[QLatin1String("songCount")].toInt();

    switch (type) {
        case SearchType::Song:
            if (resultJson.contains(QLatin1String("songs"))) {
                result.songs = parseSongs(resultJson[QLatin1String("songs")].toArray());
                result.totalCount = resultJson[QLatin1String("songCount")].toInt();
            }
            break;
        case SearchType::Playlist:
            if (resultJson.contains(QLatin1String("playlists"))) {
                const QJsonArray arr = resultJson[QLatin1String("playlists")].toArray();
                result.playlists.reserve(arr.size());
                for (const auto &item : arr) {
                    result.playlists.append(parsePlaylist(item.toObject()));
                }
                result.totalCount = resultJson[QLatin1String("playlistCount")].toInt();
            }
            break;
        case SearchType::Album:
            if (resultJson.contains(QLatin1String("albums"))) {
                const QJsonArray arr = resultJson[QLatin1String("albums")].toArray();
                result.albums.reserve(arr.size());
                for (const auto &item : arr) {
                    result.albums.append(parseAlbum(item.toObject()));
                }
                result.totalCount = resultJson[QLatin1String("albumCount")].toInt();
            }
            break;
        case SearchType::Artist:
            if (resultJson.contains(QLatin1String("artists"))) {
                const QJsonArray arr = resultJson[QLatin1String("artists")].toArray();
                result.artists.reserve(arr.size());
                for (const auto &item : arr) {
                    result.artists.append(parseArtist(item.toObject()));
                }
                result.totalCount = resultJson[QLatin1String("artistCount")].toInt();
            }
            break;
        case SearchType::All:
            // Parse all types
            if (resultJson.contains(QLatin1String("songs"))) {
                result.songs = parseSongs(resultJson[QLatin1String("songs")].toArray());
            }
            if (resultJson.contains(QLatin1String("playlists"))) {
                const QJsonArray arr = resultJson[QLatin1String("playlists")].toArray();
                result.playlists.reserve(arr.size());
                for (const auto &item : arr) {
                    result.playlists.append(parsePlaylist(item.toObject()));
                }
            }
            if (resultJson.contains(QLatin1String("albums"))) {
                const QJsonArray arr = resultJson[QLatin1String("albums")].toArray();
                result.albums.reserve(arr.size());
                for (const auto &item : arr) {
                    result.albums.append(parseAlbum(item.toObject()));
                }
            }
            if (resultJson.contains(QLatin1String("artists"))) {
                const QJsonArray arr = resultJson[QLatin1String("artists")].toArray();
                result.artists.reserve(arr.size());
                for (const auto &item : arr) {
                    result.artists.append(parseArtist(item.toObject()));
                }
            }
            break;
    }

    return result;
}

// ─── Login Result ───────────────────────────────────────────────────────────

LoginResult NeteaseParser::parseLoginResult(const QJsonObject &json)
{
    LoginResult result;

    // Account info
    if (json.contains(QLatin1String("account"))) {
        const QJsonObject account = json[QLatin1String("account")].toObject();
        result.userId = QString::number(account[QLatin1String("id")].toInteger());
    }

    // Profile info
    if (json.contains(QLatin1String("profile"))) {
        const QJsonObject profile = json[QLatin1String("profile")].toObject();
        result.nickname = profile[QLatin1String("nickname")].toString();
        result.avatarUrl = QUrl(profile[QLatin1String("avatarUrl")].toString());
        if (result.userId.isEmpty()) {
            result.userId = QString::number(profile[QLatin1String("userId")].toInteger());
        }
    }

    // Cookies — build from response cookie headers or token fields
    // The actual cookie string is typically built from Set-Cookie headers
    // in HttpClient, not from the JSON body. We store it if present.
    if (json.contains(QLatin1String("cookie"))) {
        result.cookie = json[QLatin1String("cookie")].toString();
    } else if (json.contains(QLatin1String("token"))) {
        result.cookie = QStringLiteral("MUSIC_U=") + json[QLatin1String("token")].toString();
    }

    if (result.userId.isEmpty() || result.userId == QLatin1String("0")) {
        logMalformed(QStringLiteral("parseLoginResult"), QStringLiteral("missing userId"));
    }

    return result;
}

// ─── Song URL ───────────────────────────────────────────────────────────────

SongUrlResult NeteaseParser::parseSongUrl(const QJsonObject &json)
{
    SongUrlResult result;

    // NetEase response: { data: [{ url, br, type, ... }] }
    const QJsonArray dataArray = json[QLatin1String("data")].toArray();
    if (dataArray.isEmpty()) {
        logMalformed(QStringLiteral("parseSongUrl"), QStringLiteral("empty data array"));
        result.status = SongUrlResult::Status::Failure;
        return result;
    }

    const QJsonObject &data = dataArray.first().toObject();
    const QString url = data[QLatin1String("url")].toString();

    if (url.isEmpty()) {
        // URL is null/empty when login is required or song is unavailable
        result.status = SongUrlResult::Status::RequiresLogin;
        return result;
    }

    result.status = SongUrlResult::Status::Success;
    result.url = url;
    result.durationMs = data[QLatin1String("time")].toInteger();
    result.mimeType = data[QLatin1String("type")].toString();
    result.expectedContentLength = data[QLatin1String("size")].toInteger();

    // Audio info
    result.audioInfo.bitrateKbps = data[QLatin1String("br")].toInt() / 1000;
    result.audioInfo.mimeType = result.mimeType;

    if (data.contains(QLatin1String("md5"))) {
        result.cacheKeyOverride = data[QLatin1String("md5")].toString();
    }

    return result;
}

} // namespace QeriPlayerQt
