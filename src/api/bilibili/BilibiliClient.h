/// @file BilibiliClient.h
/// @brief Bilibili API client implementing IMusicPlatformPlugin

#ifndef NERIPLAYERQT_BILIBILICLIENT_H
#define NERIPLAYERQT_BILIBILICLIENT_H

#include "api/bilibili/BilibiliTypes.h"
#include "api/common/ApiResult.h"
#include "api/common/IMusicPlatformPlugin.h"
#include "api/common/LoginResult.h"
#include "api/common/QrCodeData.h"
#include "api/common/VoidResult.h"

#include <QCoroTask>
#include <QDateTime>
#include <QNetworkRequest>
#include <QObject>
#include <QUrlQuery>

namespace NeriPlayerQt {

struct HttpResponse;
class HttpClient;
class SecureStorage;

class BilibiliClient : public QObject, public IMusicPlatformPlugin {
    Q_OBJECT

public:
    explicit BilibiliClient(HttpClient *httpClient,
                            SecureStorage *secureStorage = nullptr,
                            QObject *parent = nullptr);

    // ==================== IMusicPlatformPlugin ====================
    QCoro::Task<ApiResult<SearchResult>> search(const QString &keyword, SearchType type,
                                                int limit, int offset) override;
    QCoro::Task<ApiResult<Song>> getSongDetail(const QString &songId) override;
    QCoro::Task<ApiResult<SongUrlResult>> getSongUrl(const QString &songId,
                                                     AudioQuality quality) override;
    QCoro::Task<ApiResult<Lyrics>> getLyrics(const QString &songId) override;
    bool isAuthenticated() const override;
    QString platformName() const override;

    // ==================== Authentication ====================
    QCoro::Task<ApiResult<QrCodeData>> generateQrCode();
    QCoro::Task<ApiResult<BiliLoginPollResult>> checkQrCodeStatus(const QString &key);
    QCoro::Task<ApiResult<VoidResult>> logout();
    QCoro::Task<ApiResult<BiliUserProfile>> getUserProfile();

    // ==================== Search ====================
    QCoro::Task<ApiResult<BiliSearchVideoPage>> searchVideos(const QString &keyword, int page = 1);
    QCoro::Task<ApiResult<QStringList>> getHotSearches();

    // ==================== Videos ====================
    QCoro::Task<ApiResult<BiliVideoDetail>> getVideoDetail(const QString &bvid);
    QCoro::Task<ApiResult<BiliVideoDetail>> getVideoDetail(int avid);
    QCoro::Task<ApiResult<QList<BiliVideoPage>>> getVideoPages(const QString &bvid);
    QCoro::Task<ApiResult<BiliVideoStream>> getVideoStream(const QString &bvid, int cid,
                                                           BiliVideoQuality quality = BiliVideoQuality::Q720P);
    QCoro::Task<ApiResult<BiliAudioStream>> getAudioStream(const QString &bvid, int cid);

    // ==================== Favorites ====================
    QCoro::Task<ApiResult<QList<BiliFavoriteList>>> getUserFavorites(int mid);
    QCoro::Task<ApiResult<BiliFavoriteDetail>> getFavoriteDetail(int mediaId, int page = 1, int pageSize = 20);
    QCoro::Task<ApiResult<VoidResult>> addVideoToFavorite(int mediaId, int avid);
    QCoro::Task<ApiResult<VoidResult>> removeVideoFromFavorite(int mediaId, int avid);

signals:
    void loginStateChanged(bool loggedIn);
    void errorOccurred(const ApiError &error);

private:
    // WBI signing
    QCoro::Task<QString> signWbiParams(QUrlQuery params);
    QCoro::Task<QString> getMixinKey();
    QString extractFilenameStem(const QString &url);
    static QString md5Hex(const QByteArray &data);

    // Cookie persistence
    void loadCookies();
    void saveCookies();

    // Request helpers
    QNetworkRequest buildRequest(const QUrl &url) const;
    QCoro::Task<HttpResponse> apiGet(const QString &baseUrl,
                                     const QUrlQuery &params = {},
                                     bool wbiSign = false);
    QCoro::Task<HttpResponse> apiPost(const QString &baseUrl,
                                      const QUrlQuery &formData);

    // Cookie state
    QMap<QString, QString> m_cookies;
    bool m_authenticated = false;
    bool m_fingerprintLoaded = false;

    // WBI state
    QString m_mixinKey;
    QDateTime m_mixinKeyExpiry;

    HttpClient *m_httpClient = nullptr;
    SecureStorage *m_secureStorage = nullptr;
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_BILIBILICLIENT_H
