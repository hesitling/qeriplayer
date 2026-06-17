/// @file BilibiliParser.cpp
/// @brief JSON parsers for Bilibili API responses

#include "api/bilibili/BilibiliParser.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

namespace NeriPlayerQt {

static QJsonObject extractData(const QByteArray &json, int *outCode = nullptr, QString *outMessage = nullptr)
{
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError) {
        if (outCode) *outCode = -1;
        if (outMessage) *outMessage = QStringLiteral("JSON parse error");
        return {};
    }
    auto root = doc.object();
    int code = root.value("code").toInt(-1);
    if (outCode) *outCode = code;
    if (outMessage) *outMessage = root.value("message").toString();
    if (code != 0)
        return {};
    return root.value("data").toObject();
}

static QJsonObject extractRoot(const QByteArray &json, int *outCode = nullptr, QString *outMessage = nullptr)
{
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError) {
        if (outCode) *outCode = -1;
        if (outMessage) *outMessage = QStringLiteral("JSON parse error");
        return {};
    }
    auto root = doc.object();
    if (outCode) *outCode = root.value("code").toInt(-1);
    if (outMessage) *outMessage = root.value("message").toString();
    return root;
}

QString BilibiliParser::stripHtmlTags(const QString &html)
{
    static QRegularExpression tagRe("<[^>]*>");
    QString result = html;
    result.replace(tagRe, "");
    result.replace("&amp;", "&");
    result.replace("&lt;", "<");
    result.replace("&gt;", ">");
    result.replace("&quot;", "\"");
    result.replace("&#39;", "'");
    return result;
}

std::optional<QrCodeData> BilibiliParser::parseQrCodeData(const QByteArray &json)
{
    auto data = extractData(json);
    if (data.isEmpty())
        return std::nullopt;
    QrCodeData qr;
    qr.qrUrl = QUrl(data.value("url").toString());
    qr.key = data.value("qrcode_key").toString();
    qr.expiresInSeconds = 180; // Bilibili QR codes expire in 3 minutes
    if (qr.qrUrl.isEmpty() || qr.key.isEmpty())
        return std::nullopt;
    return qr;
}

std::optional<BiliLoginPollResult> BilibiliParser::parseLoginPollResult(const QByteArray &json)
{
    auto root = extractRoot(json);
    BiliLoginPollResult result;
    int code = root.value("code").toInt(-1);
    if (code == 0) {
        result.status = BiliQrCodeStatus::Confirmed;
        result.redirectUrl = root.value("data").toObject().value("url").toString();
    } else if (code == 86101) {
        result.status = BiliQrCodeStatus::Expired;
    } else if (code == 86090) {
        result.status = BiliQrCodeStatus::Scanned;
    } else {
        result.status = BiliQrCodeStatus::Expired;
        result.message = root.value("message").toString();
    }
    return result;
}

std::optional<BiliUserProfile> BilibiliParser::parseUserProfile(const QByteArray &json)
{
    auto data = extractData(json);
    if (data.isEmpty())
        return std::nullopt;
    BiliUserProfile profile;
    profile.mid = data.value("mid").toVariant().toString();
    profile.name = data.value("uname").toString();
    profile.avatarUrl = data.value("face").toString();
    profile.level = data.value("level_info").toObject().value("current_level").toInt();
    profile.vipType = data.value("vipType").toInt();
    profile.isLogin = data.value("isLogin").toBool();
    return profile;
}

std::optional<BiliSearchVideoPage> BilibiliParser::parseSearchVideoPage(const QByteArray &json)
{
    auto data = extractData(json);
    if (data.isEmpty())
        return std::nullopt;
    BiliSearchVideoPage page;
    page.totalCount = data.value("numResults").toInt();
    page.currentPage = data.value("page").toInt();
    page.numPages = data.value("numPages").toInt();
    page.hasMore = page.currentPage < page.numPages;
    auto items = data.value("result").toArray();
    for (const QJsonValue &item : items) {
        auto obj = item.toObject();
        BiliSearchVideoItem sv;
        sv.avid = obj.value("aid").toInt();
        sv.bvid = obj.value("bvid").toString();
        sv.title = stripHtmlTags(obj.value("title").toString());
        sv.author = obj.value("author").toString();
        sv.authorMid = QString::number(obj.value("mid").toVariant().toLongLong());
        sv.coverUrl = obj.value("pic").toString();
        if (sv.coverUrl.startsWith("//"))
            sv.coverUrl = "https:" + sv.coverUrl;
        QString dur = obj.value("duration").toString();
        if (!dur.isEmpty()) {
            QStringList parts = dur.split(':');
            if (parts.size() == 3)
                sv.durationSec = parts[0].toInt() * 3600 + parts[1].toInt() * 60 + parts[2].toInt();
            else if (parts.size() == 2)
                sv.durationSec = parts[0].toInt() * 60 + parts[1].toInt();
        }
        sv.playCount = obj.value("play").toInt();
        sv.pubDate = QDateTime::fromSecsSinceEpoch(obj.value("pubdate").toVariant().toLongLong());
        page.items.append(sv);
    }
    return page;
}

std::optional<BiliVideoDetail> BilibiliParser::parseVideoDetail(const QByteArray &json)
{
    auto data = extractData(json);
    if (data.isEmpty())
        return std::nullopt;
    BiliVideoDetail detail;
    detail.avid = data.value("aid").toInt();
    detail.bvid = data.value("bvid").toString();
    detail.title = data.value("title").toString();
    detail.description = data.value("desc").toString();
    detail.coverUrl = data.value("pic").toString();
    if (detail.coverUrl.startsWith("//"))
        detail.coverUrl = "https:" + detail.coverUrl;
    detail.duration = data.value("duration").toInt();
    detail.createdAt = QDateTime::fromSecsSinceEpoch(data.value("pubdate").toVariant().toLongLong());
    auto owner = data.value("owner").toObject();
    detail.creatorName = owner.value("name").toString();
    detail.creatorMid = QString::number(owner.value("mid").toVariant().toLongLong());
    auto stat = data.value("stat").toObject();
    detail.viewCount = stat.value("view").toInt();
    detail.danmakuCount = stat.value("danmaku").toInt();
    detail.likeCount = stat.value("like").toInt();
    detail.coinCount = stat.value("coin").toInt();
    detail.favoriteCount = stat.value("favorite").toInt();
    detail.shareCount = stat.value("share").toInt();
    auto pages = data.value("pages").toArray();
    for (const QJsonValue &p : pages) {
        auto pObj = p.toObject();
        BiliVideoPage vp;
        vp.cid = pObj.value("cid").toInt();
        vp.page = pObj.value("page").toInt();
        vp.title = pObj.value("part").toString();
        vp.duration = pObj.value("duration").toInt();
        detail.pages.append(vp);
    }
    return detail;
}

std::optional<QList<BiliVideoPage>> BilibiliParser::parseVideoPages(const QByteArray &json)
{
    auto root = extractRoot(json);
    if (root.value("code").toInt(-1) != 0)
        return std::nullopt;
    auto arr = root.value("data").toArray();
    QList<BiliVideoPage> pages;
    for (const QJsonValue &p : arr) {
        auto pObj = p.toObject();
        BiliVideoPage vp;
        vp.cid = pObj.value("cid").toInt();
        vp.page = pObj.value("page").toInt();
        vp.title = pObj.value("part").toString();
        vp.duration = pObj.value("duration").toInt();
        pages.append(vp);
    }
    return pages;
}

std::optional<BiliVideoStream> BilibiliParser::parseVideoStream(const QByteArray &json)
{
    auto data = extractData(json);
    if (data.isEmpty())
        return std::nullopt;
    BiliVideoStream stream;
    auto dash = data.value("dash").toObject();
    if (!dash.isEmpty()) {
        stream.isDash = true;
        auto audioArr = dash.value("audio").toArray();
        for (const QJsonValue &a : audioArr) {
            auto aObj = a.toObject();
            BiliAudioStream as;
            as.id = aObj.value("id").toInt();
            as.baseUrl = aObj.value("baseUrl").toString();
            auto backups = aObj.value("backupUrl").toArray();
            for (const QJsonValue &b : backups)
                as.backupUrls.append(b.toString());
            as.bandwidth = aObj.value("bandwidth").toInt();
            as.mimeType = aObj.value("mimeType").toString();
            as.codecs = aObj.value("codecs").toString();
            int qid = as.id;
            if (qid >= 30280)      as.quality = BiliAudioQuality::HiRes;
            else if (qid >= 30260) as.quality = BiliAudioQuality::Lossless;
            else if (qid >= 30232) as.quality = BiliAudioQuality::High;
            else if (qid >= 30216) as.quality = BiliAudioQuality::Medium;
            else                   as.quality = BiliAudioQuality::Low;
            stream.audios.append(as);
        }
        auto videoArr = dash.value("video").toArray();
        for (const QJsonValue &v : videoArr) {
            auto vObj = v.toObject();
            BiliDashStream ds;
            ds.id = vObj.value("id").toInt();
            ds.baseUrl = vObj.value("baseUrl").toString();
            auto backups = vObj.value("backupUrl").toArray();
            for (const QJsonValue &b : backups)
                ds.backupUrls.append(b.toString());
            ds.bandwidth = vObj.value("bandwidth").toInt();
            ds.mimeType = vObj.value("mimeType").toString();
            ds.codecs = vObj.value("codecs").toString();
            ds.width = vObj.value("width").toInt();
            ds.height = vObj.value("height").toInt();
            ds.frameRate = vObj.value("frameRate").toString();
            ds.codecid = vObj.value("codecid").toInt();
            stream.videos.append(ds);
        }
    } else {
        stream.isDash = false;
        auto durlArr = data.value("durl").toArray();
        for (const QJsonValue &d : durlArr)
            stream.durl.append(d.toObject().value("url").toString());
    }
    return stream;
}

std::optional<QList<BiliFavoriteList>> BilibiliParser::parseFavoriteList(const QByteArray &json)
{
    auto data = extractData(json);
    if (data.isEmpty())
        return std::nullopt;
    QList<BiliFavoriteList> lists;
    auto arr = data.value("list").toArray();
    for (const QJsonValue &item : arr) {
        auto obj = item.toObject();
        BiliFavoriteList fav;
        fav.id = obj.value("id").toInt();
        fav.fid = obj.value("fid").toInt();
        fav.mid = obj.value("mid").toInt();
        fav.title = obj.value("title").toString();
        fav.description = obj.value("intro").toString();
        fav.coverUrl = obj.value("cover").toString();
        if (fav.coverUrl.startsWith("//"))
            fav.coverUrl = "https:" + fav.coverUrl;
        fav.mediaCount = obj.value("media_count").toInt();
        fav.itemType = obj.value("attr").toInt();
        lists.append(fav);
    }
    return lists;
}

std::optional<BiliFavoriteDetail> BilibiliParser::parseFavoriteDetail(
    const QByteArray &json, const BiliFavoriteList &folderInfo)
{
    auto data = extractData(json);
    if (data.isEmpty())
        return std::nullopt;
    BiliFavoriteDetail detail;
    detail.info = folderInfo;
    detail.totalCount = data.value("info").toObject().value("media_count").toInt();
    auto medias = data.value("medias").toArray();
    for (const QJsonValue &m : medias) {
        auto mObj = m.toObject();
        BiliFavoriteResource res;
        res.type = mObj.value("type").toInt();
        res.id = mObj.value("id").toInt();
        res.bvid = mObj.value("bvid").toString();
        res.title = mObj.value("title").toString();
        res.coverUrl = mObj.value("cover").toString();
        if (res.coverUrl.startsWith("//"))
            res.coverUrl = "https:" + res.coverUrl;
        res.intro = mObj.value("intro").toString();
        res.durationSec = mObj.value("duration").toInt();
        auto upper = mObj.value("upper").toObject();
        res.upperMid = QString::number(upper.value("mid").toVariant().toLongLong());
        res.upperName = upper.value("name").toString();
        auto cnt = mObj.value("cnt_info").toObject();
        res.playCount = cnt.value("play").toInt();
        res.danmakuCount = cnt.value("danmaku").toInt();
        res.favTime = mObj.value("fav_time").toVariant().toLongLong();
        detail.items.append(res);
    }
    detail.hasMore = data.value("has_more").toBool();
    return detail;
}

std::optional<QStringList> BilibiliParser::parseHotSearches(const QByteArray &json)
{
    auto data = extractData(json);
    if (data.isEmpty())
        return std::nullopt;
    QStringList hot;
    auto trending = data.value("trending").toObject();
    auto list = trending.value("list").toArray();
    for (const QJsonValue &item : list)
        hot.append(item.toObject().value("keyword").toString());
    return hot;
}

std::optional<BilibiliParser::WbiImgInfo> BilibiliParser::parseWbiImg(const QByteArray &json)
{
    auto data = extractData(json);
    if (data.isEmpty())
        return std::nullopt;
    auto wbiImg = data.value("wbi_img").toObject();
    WbiImgInfo info;
    info.imgUrl = wbiImg.value("img_url").toString();
    info.subUrl = wbiImg.value("sub_url").toString();
    if (info.imgUrl.isEmpty())
        return std::nullopt;
    return info;
}

std::optional<BilibiliParser::FingerSpi> BilibiliParser::parseFingerSpi(const QByteArray &json)
{
    auto data = extractData(json);
    if (data.isEmpty())
        return std::nullopt;
    auto b3 = data.value("b_3").toString();
    auto b4 = data.value("b_4").toString();
    if (b3.isEmpty())
        return std::nullopt;
    return FingerSpi{b3, b4};
}

} // namespace NeriPlayerQt
