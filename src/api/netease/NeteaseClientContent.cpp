/// @file NeteaseClientContent.cpp
/// @brief NeteaseClient — IMusicPlatformPlugin, search, playlists, albums

#include "api/netease/NeteaseClient.h"

#include "api/common/ApiError.h"
#include "api/netease/NeteaseParser.h"

#include <QJsonArray>
#include <QJsonObject>

namespace QeriPlayerQt {

// ─── IMusicPlatformPlugin ───────────────────────────────────────────────────

QCoro::Task<ApiResult<SearchResult>> NeteaseClient::search(const QString &keyword, SearchType type, int limit,
                                                           int offset)
{
    // NetEase search type values (not 1-indexed!)
    int neteaseType;
    switch (type) {
        case SearchType::Song:
            neteaseType = 1;
            break;
        case SearchType::Album:
            neteaseType = 10;
            break;
        case SearchType::Artist:
            neteaseType = 100;
            break;
        case SearchType::Playlist:
            neteaseType = 1000;
            break;
        default:
            neteaseType = 1;
            break;
    }

    QJsonObject params;
    params[QLatin1String("s")] = keyword;
    params[QLatin1String("type")] = QString::number(neteaseType);
    params[QLatin1String("limit")] = QString::number(limit);
    params[QLatin1String("offset")] = QString::number(offset);
    params[QLatin1String("total")] = QStringLiteral("true");

    auto result = co_await makeRequest(QStringLiteral("/weapi/cloudsearch/get/web"), params);
    if (result.isError()) {
        co_return ApiResult<SearchResult>(result.error());
    }

    co_return ApiResult<SearchResult>(NeteaseParser::parseSearchResult(result.data(), type));
}

QCoro::Task<ApiResult<Song>> NeteaseClient::getSongDetail(const QString &songId)
{
    // Match Kotlin: c=[{"id":12345}], ids=[12345]
    QString idsArray = QStringLiteral("[%1]").arg(songId);
    QString cArray = QStringLiteral("[{\"id\":%1}]").arg(songId);

    QJsonObject params;
    params[QLatin1String("c")] = cArray;
    params[QLatin1String("ids")] = idsArray;

    auto result = co_await makeRequest(QStringLiteral("/weapi/v3/song/detail"), params);
    if (result.isError()) {
        co_return ApiResult<Song>(result.error());
    }

    co_return ApiResult<Song>(NeteaseParser::parseSong(result.data()));
}

QCoro::Task<ApiResult<SongUrlResult>> NeteaseClient::getSongUrl(const QString &songId, AudioQuality quality)
{
    // Map AudioQuality to NetEase bitrate
    int br;
    switch (quality) {
        case AudioQuality::Low:
            br = 128000;
            break;
        case AudioQuality::Standard:
            br = 192000;
            break;
        case AudioQuality::High:
            br = 320000;
            break;
        case AudioQuality::Lossless:
            br = 999000;
            break;
    }

    QJsonObject params;
    params[QLatin1String("ids")] = QStringLiteral("[%1]").arg(songId);
    params[QLatin1String("br")] = br;

    auto result = co_await makeRequest(QStringLiteral("/weapi/song/enhance/player/url"), params);
    if (result.isError()) {
        co_return ApiResult<SongUrlResult>(result.error());
    }

    co_return ApiResult<SongUrlResult>(NeteaseParser::parseSongUrl(result.data()));
}

QCoro::Task<ApiResult<Lyrics>> NeteaseClient::getLyrics(const QString &songId)
{
    QJsonObject params;
    params[QLatin1String("id")] = songId;
    params[QLatin1String("cp")] = QStringLiteral("false");
    params[QLatin1String("lv")] = 0;
    params[QLatin1String("tv")] = 1;
    params[QLatin1String("rv")] = 0;
    params[QLatin1String("yv")] = 1;
    params[QLatin1String("ytv")] = 1;
    params[QLatin1String("yrv")] = 0;

    auto result = co_await makeEapiRequest(QStringLiteral("/song/lyric/v1"), params);
    if (result.isError()) {
        co_return ApiResult<Lyrics>(result.error());
    }

    co_return ApiResult<Lyrics>(NeteaseParser::parseLyrics(result.data()));
}

bool NeteaseClient::isAuthenticated() const
{
    return m_authenticated;
}

QString NeteaseClient::platformName() const
{
    return QStringLiteral("NetEase");
}

// ─── Search ────────────────────────────────────────────────────────────────

QCoro::Task<ApiResult<SearchResult>> NeteaseClient::searchSongs(const QString &keyword, int limit, int offset)
{
    co_return co_await search(keyword, SearchType::Song, limit, offset);
}

// ─── Playlists ─────────────────────────────────────────────────────────────

QCoro::Task<ApiResult<Playlist>> NeteaseClient::getPlaylistDetail(const QString &playlistId)
{
    QJsonObject params;
    params[QLatin1String("id")] = playlistId;
    params[QLatin1String("n")] = 100000; // Max songs to return

    // Playlist detail uses unencrypted API endpoint
    auto result = co_await makeUnencryptedRequest(QStringLiteral("/api/v6/playlist/detail"), params);
    if (result.isError()) {
        co_return ApiResult<Playlist>(result.error());
    }

    co_return ApiResult<Playlist>(NeteaseParser::parsePlaylistDetail(result.data()));
}

QCoro::Task<ApiResult<QVector<Playlist>>> NeteaseClient::getUserPlaylists(const QString &userId)
{
    QJsonObject params;
    params[QLatin1String("uid")] = userId;
    params[QLatin1String("limit")] = 1000;
    params[QLatin1String("offset")] = 0;

    auto result = co_await makeRequest(QStringLiteral("/weapi/user/playlist"), params);
    if (result.isError()) {
        co_return ApiResult<QVector<Playlist>>(result.error());
    }

    QJsonArray playlistArray = result.data()[QLatin1String("playlist")].toArray();
    QVector<Playlist> playlists;
    playlists.reserve(playlistArray.size());
    for (const auto &item : playlistArray) {
        playlists.append(NeteaseParser::parsePlaylist(item.toObject()));
    }
    co_return ApiResult<QVector<Playlist>>(playlists);
}

QCoro::Task<ApiResult<QVector<Playlist>>> NeteaseClient::getRecommendedPlaylists()
{
    QJsonObject params;
    params[QLatin1String("limit")] = 30;

    // Use the correct endpoint from Kotlin version
    auto result = co_await makeRequest(QStringLiteral("/weapi/personalized/playlist"), params);
    if (result.isError()) {
        co_return ApiResult<QVector<Playlist>>(result.error());
    }

    QJsonArray playlistArray = result.data()[QLatin1String("result")].toArray();
    QVector<Playlist> playlists;
    playlists.reserve(playlistArray.size());
    for (const auto &item : playlistArray) {
        playlists.append(NeteaseParser::parsePlaylist(item.toObject()));
    }
    co_return ApiResult<QVector<Playlist>>(playlists);
}

QCoro::Task<ApiResult<QVector<Playlist>>> NeteaseClient::getHighQualityPlaylists(const QString &category, int limit)
{
    QJsonObject params;
    params[QLatin1String("cat")] = category;
    params[QLatin1String("limit")] = limit;
    params[QLatin1String("lasttime")] = 0;
    params[QLatin1String("total")] = true;

    // Use the correct endpoint from Kotlin version
    auto result = co_await makeRequest(QStringLiteral("/weapi/playlist/highquality/list"), params);
    if (result.isError()) {
        co_return ApiResult<QVector<Playlist>>(result.error());
    }

    QJsonArray playlistArray = result.data()[QLatin1String("playlists")].toArray();
    QVector<Playlist> playlists;
    playlists.reserve(playlistArray.size());
    for (const auto &item : playlistArray) {
        playlists.append(NeteaseParser::parsePlaylist(item.toObject()));
    }
    co_return ApiResult<QVector<Playlist>>(playlists);
}

// ─── Albums ────────────────────────────────────────────────────────────────

QCoro::Task<ApiResult<QVector<Song>>> NeteaseClient::getAlbumDetail(const QString &albumId)
{
    QJsonObject albumParams;
    albumParams[QLatin1String("n")] = 100000;
    albumParams[QLatin1String("s")] = 8;

    auto result = co_await makeRequest(QStringLiteral("/weapi/v1/album/") + albumId, albumParams,
                                       QStringLiteral("https://interface.music.163.com"));
    if (result.isError()) {
        co_return ApiResult<QVector<Song>>(result.error());
    }

    QJsonArray songsArray = result.data()[QLatin1String("songs")].toArray();
    co_return ApiResult<QVector<Song>>(NeteaseParser::parseSongs(songsArray));
}

} // namespace QeriPlayerQt
