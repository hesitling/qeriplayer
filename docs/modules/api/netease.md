# NetEase Cloud Music API (api/netease/)

## Overview

The NeteaseClient implements `IMusicPlatformPlugin` for NetEase Cloud Music. It handles authentication (phone, email, captcha, QR code), content browsing, song playback URLs, lyrics, playlist management, and user data. Uses WeAPI encryption for all POST requests.

## Source Files

```
src/api/netease/
├── NeteaseClient.h         # Client class declaration
├── NeteaseClientCore.cpp   # Request helpers, cookie management
├── NeteaseClientAuth.cpp   # Login, logout, captcha
├── NeteaseClientContent.cpp # Search, song detail, URLs, lyrics, playlists
├── NeteaseClientUser.cpp   # User data (likes, playlists, account)
├── NeteaseCrypto.h / .cpp  # WeAPI encryption (AES-128-CBC + RSA)
└── NeteaseParser.h / .cpp  # JSON → domain type parsing
```

## NeteaseClient

```cpp
class NeteaseClient : public QObject, public IMusicPlatformPlugin {
    Q_OBJECT
public:
    explicit NeteaseClient(HttpClient *httpClient,
                           SecureStorage *storage = nullptr,
                           QObject *parent = nullptr);

    // ─── IMusicPlatformPlugin ────────────────────────────────
    QCoro::Task<ApiResult<SearchResult>> search(
        const QString &keyword, SearchType type, int limit, int offset) override;
    QCoro::Task<ApiResult<Song>> getSongDetail(const QString &songId) override;
    QCoro::Task<ApiResult<SongUrlResult>> getSongUrl(
        const QString &songId, AudioQuality quality = AudioQuality::High) override;
    QCoro::Task<ApiResult<Lyrics>> getLyrics(const QString &songId) override;
    bool isAuthenticated() const override;
    QString platformName() const override;

    // ─── Configuration ───────────────────────────────────────
    void setBaseUrl(const QUrl &url);

    // ─── Authentication ──────────────────────────────────────
    QCoro::Task<ApiResult<LoginResult>> login(const QString &phone, const QString &password, int ctcode = 86);
    QCoro::Task<ApiResult<LoginResult>> loginByEmail(const QString &email, const QString &password);
    QCoro::Task<ApiResult<LoginResult>> loginByCaptcha(const QString &phone, const QString &captcha, int ctcode = 86);
    QCoro::Task<ApiResult<VoidResult>> sendCaptcha(const QString &phone, int ctcode = 86);
    QCoro::Task<ApiResult<VoidResult>> verifyCaptcha(const QString &phone, const QString &captcha, int ctcode = 86);
    QCoro::Task<ApiResult<VoidResult>> logout();
    void setCookies(const QString &cookieString);
    QCoro::Task<void> ensureWeapiSession();

    // ─── Search ──────────────────────────────────────────────
    QCoro::Task<ApiResult<SearchResult>> searchSongs(const QString &keyword, int limit = 30, int offset = 0);

    // ─── Playlists ───────────────────────────────────────────
    QCoro::Task<ApiResult<Playlist>> getPlaylistDetail(const QString &playlistId);
    QCoro::Task<ApiResult<QVector<Playlist>>> getUserPlaylists(const QString &userId);
    QCoro::Task<ApiResult<QVector<Playlist>>> getRecommendedPlaylists();
    QCoro::Task<ApiResult<QVector<Playlist>>> getHighQualityPlaylists(
        const QString &category, int limit = 30);

    // ─── Albums ──────────────────────────────────────────────
    QCoro::Task<ApiResult<QVector<Song>>> getAlbumDetail(const QString &albumId);

    // ─── User ────────────────────────────────────────────────
    QCoro::Task<ApiResult<VoidResult>> likeSong(const QString &songId);
    QCoro::Task<ApiResult<VoidResult>> unlikeSong(const QString &songId);
    QCoro::Task<ApiResult<QStringList>> getLikedSongIds(const QString &userId);
    QCoro::Task<ApiResult<QJsonObject>> getCurrentUserAccount();
    QCoro::Task<ApiResult<long long>> getCurrentUserId();

    // ─── Download ────────────────────────────────────────────
    QCoro::Task<ApiResult<QJsonObject>> getSongDownloadUrl(
        const QString &songId, const QString &level = "lossless");

    // ─── Extended ────────────────────────────────────────────
    QCoro::Task<ApiResult<QStringList>> getHighQualityTags();
    QCoro::Task<ApiResult<QJsonObject>> getDjRadioDetail(const QString &radioId, int n = 100000, int s = 8);
    QCoro::Task<ApiResult<QJsonObject>> getRelatedPlaylists(const QString &playlistId);
    QCoro::Task<ApiResult<QJsonObject>> getUserAlbums(const QString &userId, int limit = 30, int offset = 0);
    QCoro::Task<ApiResult<QJsonObject>> getUserDjRadios(const QString &userId, int limit = 30, int offset = 0);
    QCoro::Task<ApiResult<QVector<Playlist>>> getUserCreatedPlaylists(
        const QString &userId, int limit = 1000, int offset = 0);
    QCoro::Task<ApiResult<QVector<Playlist>>> getUserSubscribedPlaylists(
        const QString &userId, int limit = 1000, int offset = 0);
    QCoro::Task<ApiResult<QJsonObject>> getUserStarredAlbums(
        const QString &userId, int limit = 1000, int offset = 0);
    QCoro::Task<ApiResult<QString>> getLikedPlaylistId(const QString &userId);
};
```

## NeteaseCrypto

WeAPI encryption matching the NetEase web client protocol (AES-128-CBC + RSA). One-way only — server responses arrive over HTTPS and need no client-side decryption.

```cpp
class NeteaseCrypto {
public:
    static QByteArray weapiEncrypt(const QByteArray &data);
};
```

## NeteaseParser

Static methods to parse NetEase JSON responses into domain types. Does not throw — malformed JSON returns default values with a logged warning.

```cpp
class NeteaseParser {
public:
    static Song parseSong(const QJsonObject &json);
    static SearchResult parseSearchResult(const QJsonObject &json);
    static Album parseAlbum(const QJsonObject &json);
    static Artist parseArtist(const QJsonObject &json);
    static Playlist parsePlaylist(const QJsonObject &json);
    static Lyrics parseLyrics(const QJsonObject &json);
    static LoginResult parseLoginResult(const QJsonObject &json);
    // ... etc.
};
```

## API Endpoints

### Authentication

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/login/cellphone` | POST | Phone login |
| `/login` | POST | Email login |
| `/login/qr/key` | GET | Generate QR key |
| `/login/qr/create` | GET | Create QR code |
| `/login/qr/check` | GET | Check QR status |
| `/captcha/sent` | POST | Send captcha |
| `/captcha/verify` | POST | Verify captcha |
| `/logout` | GET | Logout |

### Content

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/cloudsearch/pc` | POST | Search (songs, playlists, albums, artists) |
| `/search/hot` | GET | Hot searches |
| `/song/detail` | GET | Song details |
| `/song/url` | GET | Playback URL |
| `/lyric` | GET | Lyrics |
| `/playlist/detail` | GET | Playlist detail |
| `/user/playlist` | GET | User playlists |
| `/personalized` | GET | Recommended playlists |
| `/top/playlist` | GET | High-quality playlists |
| `/album` | GET | Album detail |

## Design Decisions

- **Constructor takes `HttpClient*` and optional `SecureStorage*`** — no global dependencies. Cookies are restored from SecureStorage on construction.
- **`isAuthenticated()` is a lazy in-memory check** — it returns true if cookies are present, without validating against the server. Expired cookies will return true until a request fails with code 301.
- **`setCookies()` allows bypassing login** — useful when cookies are obtained externally (e.g., browser export).
- **`ensureWeapiSession()`** — visits the homepage to obtain a `__csrf` cookie. Call after `setCookies()` if `__csrf` is not provided.

## Testing

- `tests/api/TestNeteaseCrypto.cpp` — WeAPI encryption test vectors
- `tests/api/TestNeteaseParser.cpp` — JSON parsing with recorded fixtures
- `tests/api/TestNeteaseE2E.cpp` — End-to-end tests (requires network)
- `tests/fixtures/netease/` — Recorded JSON response fixtures
