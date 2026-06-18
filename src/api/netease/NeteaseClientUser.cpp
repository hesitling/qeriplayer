/// @file NeteaseClientUser.cpp
/// @brief NeteaseClient — user operations, download, collections, wrappers

#include "api/netease/NeteaseClient.h"

#include "api/common/ApiError.h"
#include "api/netease/NeteaseParser.h"
#include "core/network/HttpClient.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrlQuery>

namespace QeriPlayerQt {

// ─── User Operations ───────────────────────────────────────────────────────

QCoro::Task<ApiResult<VoidResult>> NeteaseClient::likeSong(const QString &songId)
{
    QJsonObject params;
    params[QLatin1String("trackId")] = songId;
    params[QLatin1String("like")] = QStringLiteral("true");

    auto result = co_await makeRequest(QStringLiteral("/weapi/song/like"), params);
    if (result.isError()) {
        co_return ApiResult<VoidResult>(result.error());
    }

    co_return ApiResult<VoidResult>(VoidResult {});
}

QCoro::Task<ApiResult<VoidResult>> NeteaseClient::unlikeSong(const QString &songId)
{
    QJsonObject params;
    params[QLatin1String("trackId")] = songId;
    params[QLatin1String("like")] = QStringLiteral("false");

    auto result = co_await makeRequest(QStringLiteral("/weapi/song/like"), params);
    if (result.isError()) {
        co_return ApiResult<VoidResult>(result.error());
    }

    co_return ApiResult<VoidResult>(VoidResult {});
}

QCoro::Task<ApiResult<QStringList>> NeteaseClient::getLikedSongIds(const QString &userId)
{
    QJsonObject params;
    params[QLatin1String("uid")] = userId;

    auto result = co_await makeRequest(QStringLiteral("/weapi/song/like/get"), params);
    if (result.isError()) {
        co_return ApiResult<QStringList>(result.error());
    }

    QJsonArray idsArray = result.data()[QLatin1String("ids")].toArray();
    QStringList ids;
    ids.reserve(idsArray.size());
    for (const auto &id : idsArray) {
        ids.append(QString::number(id.toVariant().toLongLong()));
    }
    co_return ApiResult<QStringList>(ids);
}

// ─── Download ─────────────────────────────────────────────────────────────

QCoro::Task<ApiResult<QJsonObject>> NeteaseClient::getSongDownloadUrl(const QString &songId, const QString &level)
{
    QJsonObject params;
    params[QLatin1String("ids")] = QStringLiteral("[%1]").arg(songId);
    params[QLatin1String("level")] = level;
    params[QLatin1String("encodeType")] = QStringLiteral("flac");

    auto result = co_await makeEapiRequest(QStringLiteral("/song/enhance/player/url/v1"), params);
    if (result.isError()) {
        co_return ApiResult<QJsonObject>(result.error());
    }

    co_return ApiResult<QJsonObject>(result.data());
}

// ─── High Quality Playlists ───────────────────────────────────────────────

QCoro::Task<ApiResult<QStringList>> NeteaseClient::getHighQualityTags()
{
    QJsonObject params;

    auto result = co_await makeRequest(QStringLiteral("/weapi/playlist/highquality/tags"), params);
    if (result.isError()) {
        co_return ApiResult<QStringList>(result.error());
    }

    // Parse tags from response: { tags: [{ name, category, hot }, ...] }
    QStringList tags;
    QJsonArray tagArray = result.data()[QLatin1String("tags")].toArray();
    for (const auto &item : tagArray) {
        QString name = item.toObject()[QLatin1String("name")].toString();
        if (!name.isEmpty()) {
            tags.append(name);
        }
    }
    co_return ApiResult<QStringList>(tags);
}

// ─── User Collections ─────────────────────────────────────────────────────

QCoro::Task<ApiResult<QJsonObject>> NeteaseClient::getUserAlbums(const QString &userId, int limit, int offset)
{
    QJsonObject params;
    params[QLatin1String("userId")] = userId;
    params[QLatin1String("offset")] = offset;
    params[QLatin1String("limit")] = limit;
    params[QLatin1String("pageType")] = QStringLiteral("3");
    params[QLatin1String("needRcmd")] = QStringLiteral("0");
    params[QLatin1String("isVistor")] = QStringLiteral("false");
    params[QLatin1String("includeStarPodcast")] = QStringLiteral("true");

    auto result = co_await makeEapiRequest(QStringLiteral("/mine/rn/resource/list"), params,
                                           QStringLiteral("https://interface3.music.163.com"));
    if (result.isError()) {
        co_return ApiResult<QJsonObject>(result.error());
    }

    co_return ApiResult<QJsonObject>(result.data());
}

QCoro::Task<ApiResult<QJsonObject>> NeteaseClient::getUserDjRadios(const QString &userId, int limit, int offset)
{
    QJsonObject params;
    params[QLatin1String("uid")] = userId;
    params[QLatin1String("offset")] = offset;
    params[QLatin1String("limit")] = limit;

    auto result = co_await makeRequest(QStringLiteral("/weapi/user/djradio/get/subed"), params);
    if (result.isError()) {
        co_return ApiResult<QJsonObject>(result.error());
    }

    co_return ApiResult<QJsonObject>(result.data());
}

// ─── DJ Radio ──────────────────────────────────────────────────────────────

QCoro::Task<ApiResult<QJsonObject>> NeteaseClient::getDjRadioDetail(const QString &radioId, int n, int s)
{
    QJsonObject params;
    params[QLatin1String("id")] = radioId;
    params[QLatin1String("n")] = n;
    params[QLatin1String("s")] = s;

    auto result = co_await makeUnencryptedRequest(QStringLiteral("/api/v6/playlist/detail"), params);
    if (result.isError()) {
        co_return ApiResult<QJsonObject>(result.error());
    }

    co_return ApiResult<QJsonObject>(result.data());
}

// ─── Related Playlists ────────────────────────────────────────────────────

QCoro::Task<ApiResult<QJsonObject>> NeteaseClient::getRelatedPlaylists(const QString &playlistId)
{
    // Scrape related playlists from the playlist HTML page (matches Kotlin)
    QUrl url(QStringLiteral("https://music.163.com/playlist"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("id"), playlistId);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("Referer", "https://music.163.com");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
                                       "(KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
    injectCookies(request);

    auto response = co_await m_httpClient->get(request);
    if (!response.isSuccess()) {
        co_return ApiResult<QJsonObject>(ApiError(response.statusCode, response.errorString));
    }

    const QString html = QString::fromUtf8(response.body);

    // Parse related playlists from HTML (matches Kotlin regex)
    // Fragile: depends on NetEase HTML structure — will break silently if they change their markup
    static const QRegularExpression regex(
        QStringLiteral("<div class=\"cver u-cover u-cover-3\">[\\s\\S]*?"
                       "<img src=\"([^\"]+)\">[\\s\\S]*?"
                       "<a class=\"sname f-fs1 s-fc0\" href=\"([^\"]+)\"[^>]*>([^<]+?)</a>[\\s\\S]*?"
                       "<a class=\"nm nm f-thide s-fc3\" href=\"([^\"]+)\"[^>]*>([^<]+?)</a>"),
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);

    QJsonArray playlists;
    QRegularExpressionMatchIterator it = regex.globalMatch(html);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();

        QString cover = match.captured(1);
        cover.replace(QRegularExpression(QStringLiteral("\\?param=\\d+y\\d+$")), QString());

        QString idStr = match.captured(2);
        idStr.remove(QStringLiteral("/playlist?id="));
        QString playlistName = match.captured(3);
        QString userIdStr = match.captured(4);
        userIdStr.remove(QStringLiteral("/user/home?id="));
        QString nickname = match.captured(5);

        QJsonObject creator;
        creator[QLatin1String("userId")] = userIdStr;
        creator[QLatin1String("nickname")] = nickname;

        QJsonObject item;
        item[QLatin1String("creator")] = creator;
        item[QLatin1String("coverImgUrl")] = cover;
        item[QLatin1String("name")] = playlistName;
        item[QLatin1String("id")] = idStr;

        playlists.append(item);
    }

    QJsonObject result;
    result[QLatin1String("code")] = 200;
    result[QLatin1String("playlists")] = playlists;
    co_return ApiResult<QJsonObject>(result);
}

// ─── User Playlist Wrappers ───────────────────────────────────────────────

QCoro::Task<ApiResult<QVector<Playlist>>> NeteaseClient::getUserCreatedPlaylists(const QString &userId, int limit,
                                                                                 int offset)
{
    QJsonObject params;
    params[QLatin1String("uid")] = userId;
    params[QLatin1String("limit")] = limit;
    params[QLatin1String("offset")] = offset;
    params[QLatin1String("includeVideo")] = QStringLiteral("true");

    auto raw = co_await makeRequest(QStringLiteral("/weapi/user/playlist"), params);
    if (raw.isError()) {
        co_return ApiResult<QVector<Playlist>>(raw.error());
    }

    long long uid = userId.toLongLong();
    QJsonArray playlistArray = raw.data()[QLatin1String("playlist")].toArray();
    QVector<Playlist> playlists;
    playlists.reserve(playlistArray.size());
    for (const auto &item : playlistArray) {
        QJsonObject pl = item.toObject();
        bool subscribed = pl[QLatin1String("subscribed")].toBool();
        long long creatorId = pl[QLatin1String("creator")].toObject()[QLatin1String("userId")].toVariant().toLongLong();
        if (creatorId == uid || !subscribed) {
            playlists.append(NeteaseParser::parsePlaylist(pl));
        }
    }

    co_return ApiResult<QVector<Playlist>>(playlists);
}

QCoro::Task<ApiResult<QVector<Playlist>>> NeteaseClient::getUserSubscribedPlaylists(const QString &userId, int limit,
                                                                                    int offset)
{
    QJsonObject params;
    params[QLatin1String("uid")] = userId;
    params[QLatin1String("limit")] = limit;
    params[QLatin1String("offset")] = offset;
    params[QLatin1String("includeVideo")] = QStringLiteral("true");

    auto raw = co_await makeRequest(QStringLiteral("/weapi/user/playlist"), params);
    if (raw.isError()) {
        co_return ApiResult<QVector<Playlist>>(raw.error());
    }

    QJsonArray playlistArray = raw.data()[QLatin1String("playlist")].toArray();
    QVector<Playlist> playlists;
    playlists.reserve(playlistArray.size());
    for (const auto &item : playlistArray) {
        QJsonObject pl = item.toObject();
        if (pl[QLatin1String("subscribed")].toBool()) {
            playlists.append(NeteaseParser::parsePlaylist(pl));
        }
    }

    co_return ApiResult<QVector<Playlist>>(playlists);
}

QCoro::Task<ApiResult<QJsonObject>> NeteaseClient::getUserStarredAlbums(const QString &userId, int limit, int offset)
{
    auto raw = co_await getUserAlbums(userId, limit, offset);
    if (raw.isError()) {
        co_return ApiResult<QJsonObject>(raw.error());
    }

    // Extract albums from the nested structure
    QJsonArray dataList = raw.data()
                              .value(QLatin1String("data"))
                              .toObject()
                              .value(QLatin1String("mainCollectInfo"))
                              .toObject()
                              .value(QLatin1String("mineAllTabDto"))
                              .toObject()
                              .value(QLatin1String("dataList"))
                              .toArray();

    QJsonObject result;
    result[QLatin1String("code")] = 200;
    result[QLatin1String("playlist")] = dataList;
    result[QLatin1String("count")] = dataList.size();
    co_return ApiResult<QJsonObject>(result);
}

QCoro::Task<ApiResult<QString>> NeteaseClient::getLikedPlaylistId(const QString &userId)
{
    QJsonObject params;
    params[QLatin1String("uid")] = userId;
    params[QLatin1String("limit")] = 1000;
    params[QLatin1String("offset")] = 0;
    params[QLatin1String("includeVideo")] = QStringLiteral("true");

    auto raw = co_await makeRequest(QStringLiteral("/weapi/user/playlist"), params);
    if (raw.isError()) {
        co_return ApiResult<QString>(raw.error());
    }

    long long uid = userId.toLongLong();
    QJsonArray playlistArray = raw.data()[QLatin1String("playlist")].toArray();
    for (const auto &item : playlistArray) {
        QJsonObject pl = item.toObject();
        int specialType = pl[QLatin1String("specialType")].toInt();
        long long creatorId = pl[QLatin1String("creator")].toObject()[QLatin1String("userId")].toVariant().toLongLong();
        if (creatorId == uid && specialType == 5) {
            co_return ApiResult<QString>(QString::number(pl[QLatin1String("id")].toVariant().toLongLong()));
        }
    }

    co_return ApiResult<QString>(ApiError(404, QStringLiteral("Liked playlist not found")));
}

} // namespace QeriPlayerQt
