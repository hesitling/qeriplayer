/// @file BilibiliClient.cpp
/// @brief Bilibili API client implementation

#include "api/bilibili/BilibiliClient.h"
#include "api/bilibili/BilibiliParser.h"
#include "core/network/HttpClient.h"

#include "core/crypto/SecureStorage.h"

#include <QCoroNetworkReply>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkCookie>
#include <QRegularExpression>
#include <QUrl>

namespace NeriPlayerQt {

/// Extract error code and message from a Bilibili API response body
static ApiError extractApiError(const QByteArray &body, int httpStatus)
{
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(body, &err);
    if (err.error != QJsonParseError::NoError)
        return ApiError{httpStatus, QStringLiteral("HTTP %1").arg(httpStatus)};
    auto root = doc.object();
    int code = root.value("code").toInt(httpStatus);
    QString msg = root.value("message").toString();
    if (msg.isEmpty())
        msg = QStringLiteral("HTTP %1").arg(httpStatus);
    return ApiError{code, msg};
}

static const QString BASE_API = "https://api.bilibili.com";
static const QString PASSPORT_API = "https://passport.bilibili.com";

static const QString URL_QR_LOGIN = PASSPORT_API + "/x/passport-login/web/qrcode/generate";
static const QString URL_QR_POLL = PASSPORT_API + "/x/passport-login/web/qrcode/poll";
static const QString URL_NAV = BASE_API + "/x/web-interface/nav";
static const QString URL_FINGER_SPI = BASE_API + "/x/frontend/finger/spi";
static const QString URL_SEARCH = BASE_API + "/x/web-interface/wbi/search/type";
static const QString URL_VIEW = BASE_API + "/x/web-interface/wbi/view";
static const QString URL_PAGELIST = BASE_API + "/x/player/pagelist";
static const QString URL_PLAYURL = BASE_API + "/x/player/wbi/playurl";
static const QString URL_FAV_LIST = BASE_API + "/x/v3/fav/folder/created/list-all";
static const QString URL_FAV_RESOURCE = BASE_API + "/x/v3/fav/resource/list";
static const QString URL_FAV_DEAL = BASE_API + "/x/v3/fav/resource/deal";
static const QString URL_HOT_SEARCH = BASE_API + "/search/hot";

static const QByteArray DEFAULT_UA =
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
    "(KHTML, like Gecko) Chrome/124.0.0.0 Safari/537.36";
static const QByteArray DEFAULT_REFERER = "https://www.bilibili.com";

static const int MIXIN_INDEX[] = {
    46, 47, 18,  2, 53,  8, 23, 32, 15, 50, 10, 31, 58,  3, 45, 35,
    27, 43,  5, 49, 33,  9, 42, 19, 29, 28, 14, 39, 12, 38, 41, 13,
    37, 48,  7, 16, 24, 55, 40, 61, 26, 17,  0,  1, 60, 51, 30,  4,
    22, 25, 54, 21, 56, 62,  6, 63, 57, 20, 34, 52, 59, 11, 36, 44
};

// ==================== Constructor ====================

BilibiliClient::BilibiliClient(HttpClient *httpClient, SecureStorage *secureStorage, QObject *parent)
    : QObject(parent), m_httpClient(httpClient), m_secureStorage(secureStorage)
{
    loadCookies();
}

// ==================== Cookie Persistence ====================

void BilibiliClient::loadCookies()
{
    if (!m_secureStorage)
        return;
    auto json = m_secureStorage->get(QStringLiteral("bilibili_cookies"));
    if (!json.has_value() || json->isEmpty())
        return;
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(json->toUtf8(), &err);
    if (err.error != QJsonParseError::NoError)
        return;
    auto obj = doc.object();
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it)
        m_cookies[it.key()] = it.value().toString();
    m_authenticated = m_cookies.contains("SESSDATA") && !m_cookies["SESSDATA"].isEmpty();
}

void BilibiliClient::saveCookies()
{
    if (!m_secureStorage)
        return;
    QJsonObject obj;
    for (auto it = m_cookies.constBegin(); it != m_cookies.constEnd(); ++it)
        obj[it.key()] = it.value();
    m_secureStorage->set(QStringLiteral("bilibili_cookies"),
                         QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}

// ==================== Request Helpers ====================

QNetworkRequest BilibiliClient::buildRequest(const QUrl &url) const
{
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", DEFAULT_UA);
    request.setRawHeader("Referer", DEFAULT_REFERER);
    // Add cookies
    if (!m_cookies.isEmpty()) {
        QString cookieStr;
        for (auto it = m_cookies.constBegin(); it != m_cookies.constEnd(); ++it) {
            if (!cookieStr.isEmpty())
                cookieStr += "; ";
            cookieStr += it.key() + "=" + it.value();
        }
        request.setRawHeader("Cookie", cookieStr.toUtf8());
    }
    return request;
}

QCoro::Task<HttpResponse> BilibiliClient::apiGet(const QString &baseUrl, const QUrlQuery &params, bool wbiSign)
{
    if (!m_fingerprintLoaded) {
        auto fpResp = co_await m_httpClient->get(buildRequest(QUrl(URL_FINGER_SPI)));
        auto spi = BilibiliParser::parseFingerSpi(fpResp.body);
        if (spi) {
            m_cookies["buvid3"] = spi->buvid3;
            m_cookies["buvid4"] = spi->buvid4;
        }
        m_fingerprintLoaded = true;
    }

    QUrl url(baseUrl);
    if (wbiSign) {
        QString signedQuery = co_await signWbiParams(params);
        url.setQuery(signedQuery);
    } else {
        url.setQuery(params);
    }
    co_return co_await m_httpClient->get(buildRequest(url));
}

QCoro::Task<HttpResponse> BilibiliClient::apiPost(const QString &baseUrl, const QUrlQuery &formData)
{
    QUrlQuery form = formData;
    QString csrf = m_cookies.value("bili_jct");
    if (!csrf.isEmpty())
        form.addQueryItem("csrf", csrf);
    QUrl url(baseUrl);
    QNetworkRequest request = buildRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    co_return co_await m_httpClient->post(request, form.toString(QUrl::FullyEncoded).toUtf8());
}

// ==================== WBI Signing ====================

QString BilibiliClient::extractFilenameStem(const QString &url)
{
    QUrl qurl(url);
    QString path = qurl.path();
    int lastSlash = path.lastIndexOf('/');
    int lastDot = path.lastIndexOf('.');
    if (lastSlash < 0 || lastDot < 0 || lastDot <= lastSlash)
        return {};
    return path.mid(lastSlash + 1, lastDot - lastSlash - 1);
}

QString BilibiliClient::md5Hex(const QByteArray &data)
{
    return QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex());
}

QCoro::Task<QString> BilibiliClient::getMixinKey()
{
    if (!m_mixinKey.isEmpty() && QDateTime::currentDateTime() < m_mixinKeyExpiry)
        co_return m_mixinKey;
    auto resp = co_await m_httpClient->get(buildRequest(QUrl(URL_NAV)));
    auto wbi = BilibiliParser::parseWbiImg(resp.body);
    QString rawKey;
    if (wbi)
        rawKey = extractFilenameStem(wbi->imgUrl) + extractFilenameStem(wbi->subUrl);
    if (rawKey.length() < 32) {
        // WBI key derivation failed — log and use fallback
        // Signed requests will likely get -412 errors
        qWarning() << "BilibiliClient: WBI mixin key derivation failed, using fallback";
        rawKey = QString(64, '0');
    }
    QString key;
    for (int i = 0; i < 32; ++i)
        key.append(rawKey[MIXIN_INDEX[i]]);
    m_mixinKey = key;
    m_mixinKeyExpiry = QDateTime::currentDateTime().addSecs(600);
    co_return m_mixinKey;
}

QCoro::Task<QString> BilibiliClient::signWbiParams(QUrlQuery params)
{
    QString mixinKey = co_await getMixinKey();
    params.addQueryItem("wts", QString::number(QDateTime::currentSecsSinceEpoch()));
    auto items = params.queryItems(QUrl::FullyDecoded);
    std::sort(items.begin(), items.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });
    QString sorted;
    for (int i = 0; i < items.size(); ++i) {
        const QString &key = items[i].first;
        QString cleanValue = items[i].second;
        cleanValue.remove(QRegularExpression("[!'()*]"));
        if (!sorted.isEmpty())
            sorted += '&';
        sorted += QUrl::toPercentEncoding(key) + '=' + QUrl::toPercentEncoding(cleanValue);
    }
    QString wRid = md5Hex((sorted + mixinKey).toUtf8());
    params.addQueryItem("w_rid", wRid);
    co_return params.toString(QUrl::FullyEncoded);
}

// ==================== IMusicPlatformPlugin ====================

QCoro::Task<ApiResult<SearchResult>> BilibiliClient::search(
    const QString &keyword, SearchType type, int limit, int offset)
{
    int page = (offset / limit) + 1;
    auto result = co_await searchVideos(keyword, page);
    if (result.isError())
        co_return ApiResult<SearchResult>(result.error());
    SearchResult sr;
    sr.totalCount = result.data().totalCount;
    sr.hasMore = result.data().hasMore;
    for (const auto &item : result.data().items) {
        Song song;
        song.id = item.bvid;
        song.name = item.title;
        song.artist = item.author;
        song.platform = MusicPlatform::Bilibili;
        song.durationMs = item.durationSec * 1000LL;
        song.coverUrl = QUrl(item.coverUrl);
        song.extra["avid"] = item.avid;
        sr.songs.append(song);
    }
    co_return ApiResult<SearchResult>(sr);
}

QCoro::Task<ApiResult<Song>> BilibiliClient::getSongDetail(const QString &songId)
{
    auto result = co_await getVideoDetail(songId);
    if (result.isError())
        co_return ApiResult<Song>(result.error());
    const auto &vd = result.data();
    Song song;
    song.id = vd.bvid;
    song.name = vd.title;
    song.artist = vd.creatorName;
    song.platform = MusicPlatform::Bilibili;
    song.durationMs = vd.duration * 1000LL;
    song.coverUrl = QUrl(vd.coverUrl);
    song.extra["avid"] = vd.avid;
    co_return ApiResult<Song>(song);
}

QCoro::Task<ApiResult<SongUrlResult>> BilibiliClient::getSongUrl(
    const QString &songId, AudioQuality quality)
{
    auto detailResult = co_await getVideoDetail(songId);
    if (detailResult.isError())
        co_return ApiResult<SongUrlResult>(detailResult.error());
    const auto &detail = detailResult.data();
    if (detail.pages.isEmpty())
        co_return ApiResult<SongUrlResult>(ApiError{404, QStringLiteral("视频无分P")});
    int cid = detail.pages.first().cid;
    auto streamResult = co_await getAudioStream(songId, cid);
    if (streamResult.isError())
        co_return ApiResult<SongUrlResult>(streamResult.error());
    const auto &audio = streamResult.data();
    SongUrlResult sur;
    sur.status = SongUrlResult::Status::Success;
    sur.url = audio.baseUrl;
    sur.durationMs = detail.duration * 1000LL;
    sur.mimeType = audio.mimeType;
    sur.audioInfo.bitrateKbps = audio.bandwidth / 1000;
    sur.audioInfo.mimeType = audio.mimeType;
    sur.audioInfo.codecLabel = audio.codecs;
    co_return ApiResult<SongUrlResult>(sur);
}

QCoro::Task<ApiResult<Lyrics>> BilibiliClient::getLyrics(const QString &songId)
{
    co_return ApiResult<Lyrics>(ApiError{404, QStringLiteral("Bilibili 不支持歌词")});
}

bool BilibiliClient::isAuthenticated() const
{
    return m_authenticated;
}

QString BilibiliClient::platformName() const
{
    return QStringLiteral("Bilibili");
}

// ==================== Authentication ====================

QCoro::Task<ApiResult<QrCodeData>> BilibiliClient::generateQrCode()
{
    auto resp = co_await m_httpClient->get(buildRequest(QUrl(URL_QR_LOGIN)));
    if (!resp.isSuccess())
        co_return ApiResult<QrCodeData>(ApiError{resp.statusCode, QStringLiteral("获取二维码失败")});
    auto qr = BilibiliParser::parseQrCodeData(resp.body);
    if (!qr)
        co_return ApiResult<QrCodeData>(ApiError{-1, QStringLiteral("解析二维码失败")});
    co_return ApiResult<QrCodeData>(*qr);
}

QCoro::Task<ApiResult<BiliLoginPollResult>> BilibiliClient::checkQrCodeStatus(const QString &key)
{
    QUrlQuery params;
    params.addQueryItem("qrcode_key", key);
    QUrl url(URL_QR_POLL);
    url.setQuery(params);
    auto resp = co_await m_httpClient->get(buildRequest(url));
    if (!resp.isSuccess())
        co_return ApiResult<BiliLoginPollResult>(ApiError{resp.statusCode, QStringLiteral("查询登录状态失败")});
    auto result = BilibiliParser::parseLoginPollResult(resp.body);
    if (!result)
        co_return ApiResult<BiliLoginPollResult>(ApiError{-1, QStringLiteral("解析登录结果失败")});
    if (result->status == BiliQrCodeStatus::Confirmed) {
        // Extract cookies from Set-Cookie headers
        for (const auto &header : resp.headers) {
            if (header.first.toLower() == "set-cookie") {
                QString cookieStr = QString::fromUtf8(header.second);
                // Parse "name=value; ..." format
                QString nameValue = cookieStr.split(';').first().trimmed();
                int eqIdx = nameValue.indexOf('=');
                if (eqIdx > 0) {
                    QString name = nameValue.left(eqIdx).trimmed();
                    QString value = nameValue.mid(eqIdx + 1).trimmed();
                    if (name == "SESSDATA" || name == "bili_jct" ||
                        name == "DedeUserID" || name == "DedeUserID__ckMd5") {
                        m_cookies[name] = value;
                    }
                }
            }
        }
        m_authenticated = m_cookies.contains("SESSDATA");
        if (m_authenticated)
            saveCookies();
        emit loginStateChanged(m_authenticated);
    }
    co_return ApiResult<BiliLoginPollResult>(*result);
}

QCoro::Task<ApiResult<VoidResult>> BilibiliClient::logout()
{
    m_cookies.clear();
    m_authenticated = false;
    saveCookies(); // Clear persisted cookies
    emit loginStateChanged(false);
    co_return ApiResult<VoidResult>(VoidResult{});
}

QCoro::Task<ApiResult<BiliUserProfile>> BilibiliClient::getUserProfile()
{
    auto resp = co_await apiGet(URL_NAV);
    if (!resp.isSuccess())
        co_return ApiResult<BiliUserProfile>(ApiError{resp.statusCode, QStringLiteral("获取用户信息失败")});
    auto profile = BilibiliParser::parseUserProfile(resp.body);
    if (!profile)
        co_return ApiResult<BiliUserProfile>(ApiError{-1, QStringLiteral("解析用户信息失败")});
    co_return ApiResult<BiliUserProfile>(*profile);
}

// ==================== Search ====================

QCoro::Task<ApiResult<BiliSearchVideoPage>> BilibiliClient::searchVideos(const QString &keyword, int page)
{
    QUrlQuery params;
    params.addQueryItem("search_type", "video");
    params.addQueryItem("keyword", keyword);
    params.addQueryItem("order", "totalrank");
    params.addQueryItem("page", QString::number(page));
    auto resp = co_await apiGet(URL_SEARCH, params, true);
    if (!resp.isSuccess())
        co_return ApiResult<BiliSearchVideoPage>(ApiError{resp.statusCode, QStringLiteral("搜索失败")});
    auto result = BilibiliParser::parseSearchVideoPage(resp.body);
    if (!result)
        co_return ApiResult<BiliSearchVideoPage>(ApiError{-1, QStringLiteral("解析搜索结果失败")});
    co_return ApiResult<BiliSearchVideoPage>(*result);
}

QCoro::Task<ApiResult<QStringList>> BilibiliClient::getHotSearches()
{
    auto resp = co_await m_httpClient->get(buildRequest(QUrl(URL_HOT_SEARCH)));
    if (!resp.isSuccess())
        co_return ApiResult<QStringList>(ApiError{resp.statusCode, QStringLiteral("获取热搜失败")});
    auto hot = BilibiliParser::parseHotSearches(resp.body);
    if (!hot)
        co_return ApiResult<QStringList>(ApiError{-1, QStringLiteral("解析热搜失败")});
    co_return ApiResult<QStringList>(*hot);
}

// ==================== Videos ====================

QCoro::Task<ApiResult<BiliVideoDetail>> BilibiliClient::getVideoDetail(const QString &bvid)
{
    QUrlQuery params;
    params.addQueryItem("bvid", bvid);
    auto resp = co_await apiGet(URL_VIEW, params, true);
    if (!resp.isSuccess())
        co_return ApiResult<BiliVideoDetail>(ApiError{resp.statusCode, QStringLiteral("获取视频详情失败")});
    auto detail = BilibiliParser::parseVideoDetail(resp.body);
    if (!detail)
        co_return ApiResult<BiliVideoDetail>(ApiError{-1, QStringLiteral("解析视频详情失败")});
    co_return ApiResult<BiliVideoDetail>(*detail);
}

QCoro::Task<ApiResult<BiliVideoDetail>> BilibiliClient::getVideoDetail(int avid)
{
    QUrlQuery params;
    params.addQueryItem("aid", QString::number(avid));
    auto resp = co_await apiGet(URL_VIEW, params, true);
    if (!resp.isSuccess())
        co_return ApiResult<BiliVideoDetail>(ApiError{resp.statusCode, QStringLiteral("获取视频详情失败")});
    auto detail = BilibiliParser::parseVideoDetail(resp.body);
    if (!detail)
        co_return ApiResult<BiliVideoDetail>(ApiError{-1, QStringLiteral("解析视频详情失败")});
    co_return ApiResult<BiliVideoDetail>(*detail);
}

QCoro::Task<ApiResult<QList<BiliVideoPage>>> BilibiliClient::getVideoPages(const QString &bvid)
{
    QUrlQuery params;
    params.addQueryItem("bvid", bvid);
    auto resp = co_await apiGet(URL_PAGELIST, params);
    if (!resp.isSuccess())
        co_return ApiResult<QList<BiliVideoPage>>(ApiError{resp.statusCode, QStringLiteral("获取分P列表失败")});
    auto pages = BilibiliParser::parseVideoPages(resp.body);
    if (!pages)
        co_return ApiResult<QList<BiliVideoPage>>(ApiError{-1, QStringLiteral("解析分P列表失败")});
    co_return ApiResult<QList<BiliVideoPage>>(*pages);
}

QCoro::Task<ApiResult<BiliVideoStream>> BilibiliClient::getVideoStream(
    const QString &bvid, int cid, BiliVideoQuality quality)
{
    QUrlQuery params;
    params.addQueryItem("bvid", bvid);
    params.addQueryItem("cid", QString::number(cid));
    params.addQueryItem("qn", QString::number(static_cast<int>(quality)));
    params.addQueryItem("fnval", "272");
    params.addQueryItem("platform", "pc");
    params.addQueryItem("otype", "json");
    auto resp = co_await apiGet(URL_PLAYURL, params, true);
    if (!resp.isSuccess())
        co_return ApiResult<BiliVideoStream>(ApiError{resp.statusCode, QStringLiteral("获取播放地址失败")});
    auto stream = BilibiliParser::parseVideoStream(resp.body);
    if (!stream)
        co_return ApiResult<BiliVideoStream>(ApiError{-1, QStringLiteral("解析播放地址失败")});
    co_return ApiResult<BiliVideoStream>(*stream);
}

QCoro::Task<ApiResult<BiliAudioStream>> BilibiliClient::getAudioStream(const QString &bvid, int cid)
{
    auto streamResult = co_await getVideoStream(bvid, cid);
    if (streamResult.isError())
        co_return ApiResult<BiliAudioStream>(streamResult.error());
    const auto &stream = streamResult.data();
    if (stream.audios.isEmpty())
        co_return ApiResult<BiliAudioStream>(ApiError{404, QStringLiteral("无可用音频流")});
    // Sort by quality (highest first), then by bandwidth
    auto audios = stream.audios;
    std::sort(audios.begin(), audios.end(), [](const BiliAudioStream &a, const BiliAudioStream &b) {
        if (a.quality != b.quality)
            return static_cast<int>(a.quality) > static_cast<int>(b.quality);
        return a.bandwidth > b.bandwidth;
    });
    co_return ApiResult<BiliAudioStream>(audios.first());
}

// ==================== Favorites ====================

QCoro::Task<ApiResult<QList<BiliFavoriteList>>> BilibiliClient::getUserFavorites(int mid)
{
    QUrlQuery params;
    params.addQueryItem("up_mid", QString::number(mid));
    auto resp = co_await apiGet(URL_FAV_LIST, params);
    if (!resp.isSuccess())
        co_return ApiResult<QList<BiliFavoriteList>>(ApiError{resp.statusCode, QStringLiteral("获取收藏夹失败")});
    auto lists = BilibiliParser::parseFavoriteList(resp.body);
    if (!lists)
        co_return ApiResult<QList<BiliFavoriteList>>(ApiError{-1, QStringLiteral("解析收藏夹失败")});
    co_return ApiResult<QList<BiliFavoriteList>>(*lists);
}

QCoro::Task<ApiResult<BiliFavoriteDetail>> BilibiliClient::getFavoriteDetail(int mediaId, int page, int pageSize)
{
    BiliFavoriteList folderInfo;
    folderInfo.id = mediaId;
    QUrlQuery infoParams;
    infoParams.addQueryItem("media_id", QString::number(mediaId));
    auto infoResp = co_await apiGet(BASE_API + "/x/v3/fav/folder/info", infoParams);
    if (infoResp.isSuccess()) {
        auto infoData = QJsonDocument::fromJson(infoResp.body).object().value("data").toObject();
        folderInfo.title = infoData.value("title").toString();
        folderInfo.mediaCount = infoData.value("media_count").toInt();
    }
    QUrlQuery params;
    params.addQueryItem("media_id", QString::number(mediaId));
    params.addQueryItem("pn", QString::number(page));
    params.addQueryItem("ps", QString::number(pageSize));
    params.addQueryItem("order", "mtime");
    auto resp = co_await apiGet(URL_FAV_RESOURCE, params);
    if (!resp.isSuccess())
        co_return ApiResult<BiliFavoriteDetail>(ApiError{resp.statusCode, QStringLiteral("获取收藏夹内容失败")});
    auto detail = BilibiliParser::parseFavoriteDetail(resp.body, folderInfo);
    if (!detail)
        co_return ApiResult<BiliFavoriteDetail>(ApiError{-1, QStringLiteral("解析收藏夹内容失败")});
    co_return ApiResult<BiliFavoriteDetail>(*detail);
}

QCoro::Task<ApiResult<VoidResult>> BilibiliClient::addVideoToFavorite(int mediaId, int avid)
{
    QUrlQuery form;
    form.addQueryItem("rid", QString::number(avid));
    form.addQueryItem("type", "2");
    form.addQueryItem("add_media_ids", QString::number(mediaId));
    form.addQueryItem("del_media_ids", "");
    auto resp = co_await apiPost(URL_FAV_DEAL, form);
    if (!resp.isSuccess())
        co_return ApiResult<VoidResult>(ApiError{resp.statusCode, QStringLiteral("添加收藏失败")});
    auto root = QJsonDocument::fromJson(resp.body).object();
    if (root.value("code").toInt(-1) != 0)
        co_return ApiResult<VoidResult>(ApiError{root.value("code").toInt(), root.value("message").toString()});
    co_return ApiResult<VoidResult>(VoidResult{});
}

QCoro::Task<ApiResult<VoidResult>> BilibiliClient::removeVideoFromFavorite(int mediaId, int avid)
{
    QUrlQuery form;
    form.addQueryItem("rid", QString::number(avid));
    form.addQueryItem("type", "2");
    form.addQueryItem("add_media_ids", "");
    form.addQueryItem("del_media_ids", QString::number(mediaId));
    auto resp = co_await apiPost(URL_FAV_DEAL, form);
    if (!resp.isSuccess())
        co_return ApiResult<VoidResult>(ApiError{resp.statusCode, QStringLiteral("取消收藏失败")});
    auto root = QJsonDocument::fromJson(resp.body).object();
    if (root.value("code").toInt(-1) != 0)
        co_return ApiResult<VoidResult>(ApiError{root.value("code").toInt(), root.value("message").toString()});
    co_return ApiResult<VoidResult>(VoidResult{});
}

} // namespace NeriPlayerQt
