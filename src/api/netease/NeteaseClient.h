/// @file NeteaseClient.h
/// @brief NetEase Cloud Music API client

#ifndef QERIPLAYERQT_NETEASECLIENT_H
#define QERIPLAYERQT_NETEASECLIENT_H

#include "api/common/ApiResult.h"
#include "api/common/IMusicPlatformPlugin.h"
#include "api/common/LoginResult.h"
#include "api/common/VoidResult.h"
#include "domain/Album.h"
#include "domain/Enums.h"
#include "domain/Lyrics.h"
#include "domain/Playlist.h"
#include "domain/SearchResult.h"
#include "domain/Song.h"
#include "domain/SongUrlResult.h"

#include <QCoroTask>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVector>

#include <QNetworkRequest>
#include <QRegularExpression>

namespace QeriPlayerQt {

class HttpClient;
class SecureStorage;

/**
 * @brief NetEase Cloud Music API client
 *
 * Implements IMusicPlatformPlugin for cross-platform operations.
 * Additional platform-specific methods (playlist CRUD, user operations,
 * recommendations) are on the concrete class.
 *
 * Uses WeAPI encryption (NeteaseCrypto) for all POST requests.
 * Cookies are persisted in SecureStorage for session continuity.
 */
class NeteaseClient : public QObject, public IMusicPlatformPlugin {
    Q_OBJECT

public:
    /**
     * @brief Construct a NeteaseClient
     * @param httpClient HTTP client for network requests (must not be null)
     * @param storage Optional secure storage for cookie persistence
     * @param parent Qt parent object
     */
    explicit NeteaseClient(HttpClient *httpClient, SecureStorage *storage = nullptr, QObject *parent = nullptr);

    // ─── IMusicPlatformPlugin ─────────────────────────────────────────────

    QCoro::Task<ApiResult<SearchResult>> search(const QString &keyword, SearchType type, int limit,
                                                int offset) override;

    QCoro::Task<ApiResult<Song>> getSongDetail(const QString &songId) override;
    QCoro::Task<ApiResult<SongUrlResult>> getSongUrl(const QString &songId,
                                                     AudioQuality quality = AudioQuality::High) override;
    QCoro::Task<ApiResult<Lyrics>> getLyrics(const QString &songId) override;

    /// @brief Check if we have a stored session (cookies + CSRF token)
    /// @note This is a lazy check — it does not validate the session against the server.
    ///       Expired cookies will still return true until a request fails with code 301.
    bool isAuthenticated() const override;
    QString platformName() const override;

    // ─── Configuration ────────────────────────────────────────────────────

    void setBaseUrl(const QUrl &url);

    // ─── Authentication ───────────────────────────────────────────────────

    QCoro::Task<ApiResult<LoginResult>> login(const QString &phone, const QString &password, int ctcode = 86);
    QCoro::Task<ApiResult<LoginResult>> loginByEmail(const QString &email, const QString &password);
    QCoro::Task<ApiResult<VoidResult>> logout();

    /**
     * @brief Set cookies directly (bypass login)
     *
     * Use when cookies are obtained externally (e.g., browser export).
     * At minimum, provide MUSIC_U for auth and __csrf for WeAPI requests.
     * Example: "MUSIC_U=xxx; __csrf=yyy"
     */
    void setCookies(const QString &cookieString);

    /**
     * @brief Visit homepage to obtain __csrf cookie
     *
     * Call after setCookies() if __csrf is not provided.
     * Matches Kotlin ensureWeapiSession().
     */
    QCoro::Task<void> ensureWeapiSession();

    // ─── Search ───────────────────────────────────────────────────────────

    QCoro::Task<ApiResult<SearchResult>> searchSongs(const QString &keyword, int limit = 30, int offset = 0);

    // ─── Playlists ────────────────────────────────────────────────────────

    QCoro::Task<ApiResult<Playlist>> getPlaylistDetail(const QString &playlistId);
    QCoro::Task<ApiResult<QVector<Playlist>>> getUserPlaylists(const QString &userId);
    QCoro::Task<ApiResult<QVector<Playlist>>> getRecommendedPlaylists();
    QCoro::Task<ApiResult<QVector<Playlist>>> getHighQualityPlaylists(const QString &category, int limit = 30);

    // ─── Albums ───────────────────────────────────────────────────────────

    QCoro::Task<ApiResult<QVector<Song>>> getAlbumDetail(const QString &albumId);

    // ─── User ─────────────────────────────────────────────────────────────

    QCoro::Task<ApiResult<VoidResult>> likeSong(const QString &songId);
    QCoro::Task<ApiResult<VoidResult>> unlikeSong(const QString &songId);
    QCoro::Task<ApiResult<QStringList>> getLikedSongIds(const QString &userId);
    QCoro::Task<ApiResult<QJsonObject>> getCurrentUserAccount();

    /**
     * @brief Get the current logged-in user's userId
     * @return userId as long long, or -1 if not logged in
     */
    QCoro::Task<ApiResult<long long>> getCurrentUserId();

    // ─── Captcha Auth ─────────────────────────────────────────────────────

    QCoro::Task<ApiResult<LoginResult>> loginByCaptcha(const QString &phone, const QString &captcha, int ctcode = 86);
    QCoro::Task<ApiResult<VoidResult>> sendCaptcha(const QString &phone, int ctcode = 86);
    QCoro::Task<ApiResult<VoidResult>> verifyCaptcha(const QString &phone, const QString &captcha, int ctcode = 86);

    // ─── Download ─────────────────────────────────────────────────────────

    QCoro::Task<ApiResult<QJsonObject>> getSongDownloadUrl(const QString &songId,
                                                           const QString &level = QStringLiteral("lossless"));

    // ─── High Quality Playlists ───────────────────────────────────────────

    QCoro::Task<ApiResult<QStringList>> getHighQualityTags();

    // ─── DJ Radio ────────────────────────────────────────────────────────

    QCoro::Task<ApiResult<QJsonObject>> getDjRadioDetail(const QString &radioId, int n = 100000, int s = 8);

    // ─── Related Playlists ────────────────────────────────────────────────

    QCoro::Task<ApiResult<QJsonObject>> getRelatedPlaylists(const QString &playlistId);

    // ─── User Collections ─────────────────────────────────────────────────

    QCoro::Task<ApiResult<QJsonObject>> getUserAlbums(const QString &userId, int limit = 30, int offset = 0);
    QCoro::Task<ApiResult<QJsonObject>> getUserDjRadios(const QString &userId, int limit = 30, int offset = 0);

    // ─── User Playlist Wrappers ───────────────────────────────────────────

    QCoro::Task<ApiResult<QVector<Playlist>>> getUserCreatedPlaylists(const QString &userId, int limit = 1000,
                                                                      int offset = 0);
    QCoro::Task<ApiResult<QVector<Playlist>>> getUserSubscribedPlaylists(const QString &userId, int limit = 1000,
                                                                         int offset = 0);
    QCoro::Task<ApiResult<QJsonObject>> getUserStarredAlbums(const QString &userId, int limit = 1000, int offset = 0);
    QCoro::Task<ApiResult<QString>> getLikedPlaylistId(const QString &userId);

private:
    // Request helpers
    QCoro::Task<ApiResult<QJsonObject>> makeRequest(const QString &path, const QJsonObject &params = {},
                                                    const QString &host = {}, bool retried = false);

    QCoro::Task<ApiResult<QJsonObject>> makeEapiRequest(const QString &path, const QJsonObject &params = {},
                                                        const QString &host
                                                        = QStringLiteral("https://interface.music.163.com"),
                                                        bool returnRawOnNon200 = false, bool retried = false);

    QCoro::Task<ApiResult<QJsonObject>> makeUnencryptedRequest(const QString &path, const QJsonObject &params = {});

    void injectCookies(QNetworkRequest &request);
    void persistCookies(const QString &cookieString);
    void clearCookies();

    // Members
    HttpClient *m_httpClient;
    SecureStorage *m_storage;
    QUrl m_baseUrl;
    QString m_cookie;
    QString m_csrfToken;
    bool m_authenticated = false;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_NETEASECLIENT_H
