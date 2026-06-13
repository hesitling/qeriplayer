# NetEase Cloud Music Module (netease/)

## 1. Overview

The NetEase Cloud Music module provides a complete wrapper for the NetEase Cloud Music API, including authentication, search, playback, lyrics retrieval, playlist management, and more.

## 2. Directory Structure

```
src/api/netease/
├── NeteaseClient.h/.cpp    # NetEase client
├── NeteaseCrypto.h/.cpp    # Encryption utilities
├── NeteaseParser.h/.cpp    # Data parser
└── NeteaseTypes.h          # Type definitions
```

## 3. Main Class Design

### 3.1 NeteaseClient

NetEase Cloud Music client.

```cpp
class NeteaseClient : public QObject {
    Q_OBJECT
public:
    explicit NeteaseClient(HttpClient *httpClient, 
                           QObject *parent = nullptr);
    
    // ==================== Authentication ====================
    
    // Phone number login
    QCoro::Task<ApiResult<LoginResult>> login(const QString &phone, 
                                          const QString &password);
    
    // Email login
    QCoro::Task<ApiResult<LoginResult>> loginByEmail(const QString &email,
                                                  const QString &password);
    
    // QR code login
    QCoro::Task<ApiResult<QrCodeData>> generateQrCode();
    QCoro::Task<ApiResult<LoginResult>> checkQrCodeStatus(const QString &key);
    
    // Logout
    QCoro::Task<ApiResult<VoidResult>> logout();
    
    // Check login status
    bool isAuthenticated() const;
    
    // Get user profile
    QCoro::Task<ApiResult<UserProfile>> getUserProfile();
    
    // ==================== Search ====================
    
    // Comprehensive search
    QCoro::Task<ApiResult<SearchResult>> search(const QString &keyword,
                                            SearchType type = SearchType::All,
                                            int limit = 30, int offset = 0);
    
    // Search songs
    QCoro::Task<ApiResult<SearchResult>> searchSongs(const QString &keyword,
                                                  int limit = 30, 
                                                  int offset = 0);
    
    // Search playlists
    QCoro::Task<ApiResult<SearchResult>> searchPlaylists(const QString &keyword,
                                                      int limit = 30,
                                                      int offset = 0);
    
    // Search albums
    QCoro::Task<ApiResult<SearchResult>> searchAlbums(const QString &keyword,
                                                   int limit = 30,
                                                   int offset = 0);
    
    // Search artists
    QCoro::Task<ApiResult<SearchResult>> searchArtists(const QString &keyword,
                                                    int limit = 30,
                                                    int offset = 0);
    
    // Hot searches
    QCoro::Task<ApiResult<QStringList>> getHotSearches();
    
    // ==================== Songs ====================
    
    // Song details
    QCoro::Task<ApiResult<SongDetail>> getSongDetail(const QString &songId);
    
    // Song playback URL
    QCoro::Task<ApiResult<PlaybackUrl>> getSongUrl(const QString &songId,
                                                AudioQuality quality = AudioQuality::High);
    
    // Lyrics
    QCoro::Task<ApiResult<Lyrics>> getLyrics(const QString &songId);
    
    // Similar songs
    QCoro::Task<ApiResult<QList<Song>>> getSimilarSongs(const QString &songId);
    
    // ==================== Playlists ====================
    
    // Playlist details
    QCoro::Task<ApiResult<PlaylistDetail>> getPlaylistDetail(const QString &playlistId);
    
    // User playlists
    QCoro::Task<ApiResult<QList<Playlist>>> getUserPlaylists(const QString &userId);
    
    // Recommended playlists
    QCoro::Task<ApiResult<QList<Playlist>>> getRecommendedPlaylists();
    
    // High-quality playlists
    QCoro::Task<ApiResult<QList<Playlist>>> getHighQualityPlaylists(
        const QString &category = {}, int limit = 30);
    
    // Create playlist
    QCoro::Task<ApiResult<Playlist>> createPlaylist(const QString &name,
                                                 const QString &description = {});
    
    // Delete playlist
    QCoro::Task<ApiResult<VoidResult>> deletePlaylist(const QString &playlistId);
    
    // Add song to playlist
    QCoro::Task<ApiResult<VoidResult>> addSongToPlaylist(const QString &playlistId,
                                                      const QString &songId);
    
    // Remove song from playlist
    QCoro::Task<ApiResult<VoidResult>> removeSongFromPlaylist(
        const QString &playlistId, const QString &songId);
    
    // ==================== Albums ====================
    
    // Album details
    QCoro::Task<ApiResult<AlbumDetail>> getAlbumDetail(const QString &albumId);
    
    // New albums
    QCoro::Task<ApiResult<QList<Album>>> getNewAlbums(int limit = 30);
    
    // ==================== Artists ====================
    
    // Artist details
    QCoro::Task<ApiResult<ArtistDetail>> getArtistDetail(const QString &artistId);
    
    // Artist songs
    QCoro::Task<ApiResult<QList<Song>>> getArtistSongs(const QString &artistId,
                                                    int limit = 50,
                                                    int offset = 0);
    
    // Artist albums
    QCoro::Task<ApiResult<QList<Album>>> getArtistAlbums(const QString &artistId,
                                                      int limit = 30,
                                                      int offset = 0);
    
    // Top artists
    QCoro::Task<ApiResult<QList<Artist>>> getTopArtists(int limit = 50);
    
    // ==================== User ====================
    
    // Liked songs
    QCoro::Task<ApiResult<QList<Song>>> getLikedSongs(const QString &userId);
    
    // Like song
    QCoro::Task<ApiResult<VoidResult>> likeSong(const QString &songId);
    
    // Unlike song
    QCoro::Task<ApiResult<VoidResult>> unlikeSong(const QString &songId);
    
    // Play history
    QCoro::Task<ApiResult<QList<PlayHistory>>> getPlayHistory();
    
    // ==================== Recommendations ====================
    
    // Daily recommendations
    QCoro::Task<ApiResult<QList<Song>>> getDailyRecommendations();
    
    // New songs
    QCoro::Task<ApiResult<QList<Song>>> getNewSongs();
    
    // Personal FM
    QCoro::Task<ApiResult<QList<Song>>> getPersonalFM();
    
signals:
    void loginStateChanged(bool loggedIn);
    void errorOccurred(const ApiError &error);
    
private:
    QJsonObject encryptRequest(const QJsonObject &data);
    QJsonObject decryptResponse(const QJsonObject &data);
    QCoro::Task<ApiResult<QJsonObject>> makeRequest(
        const QString &url, const QJsonObject &data = {},
        const HttpHeaders &headers = {});
    
    HttpClient *m_httpClient;
    NeteaseCrypto *m_crypto;
    QString m_cookie;
    QString m_csrfToken;
    bool m_authenticated = false;
};
```

### 3.2 NeteaseCrypto

NetEase encryption utilities.

```cpp
class NeteaseCrypto {
public:
    // WeAPI encryption
    static QByteArray weapiEncrypt(const QByteArray &data);
    static QByteArray weapiDecrypt(const QByteArray &data);
    
    // Linux API encryption
    static QByteArray linuxEncrypt(const QByteArray &data);
    static QByteArray linuxDecrypt(const QByteArray &data);
    
    // EAPI encryption
    static QByteArray eapiEncrypt(const QByteArray &data);
    static QByteArray eapiDecrypt(const QByteArray &data);
    
private:
    static QByteArray aesEncrypt(const QByteArray &data, 
                                 const QByteArray &key,
                                 const QByteArray &iv);
    static QByteArray aesDecrypt(const QByteArray &data,
                                 const QByteArray &key,
                                 const QByteArray &iv);
    static QByteArray rsaEncrypt(const QByteArray &data,
                                 const QByteArray &key);
    
    static const QByteArray WEAPI_KEY;
    static const QByteArray WEAPI_IV;
    static const QByteArray LINUX_KEY;
    static const QByteArray LINUX_IV;
    static const QByteArray EAPI_KEY;
    static const QByteArray EAPI_IV;
};
```

## 4. API Endpoints

### 4.1 Authentication

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/login/cellphone` | POST | Phone number login |
| `/login` | POST | Email login |
| `/login/qr/key` | GET | Generate QR code |
| `/login/qr/create` | GET | Create QR code |
| `/login/qr/check` | GET | Check QR code status |
| `/logout` | GET | Logout |

### 4.2 Search

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/cloudsearch/pc` | POST | Comprehensive search |
| `/search/hot` | GET | Hot searches |
| `/search/hot/detail` | GET | Hot search details |

### 4.3 Songs

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/song/detail` | GET | Song details |
| `/song/url` | GET | Playback URL |
| `/lyric` | GET | Lyrics |
| `/simi/song` | GET | Similar songs |

### 4.4 Playlists

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/playlist/detail` | GET | Playlist details |
| `/user/playlist` | GET | User playlists |
| `/personalized` | GET | Recommended playlists |
| `/top/playlist` | GET | High-quality playlists |
| `/playlist/create` | POST | Create playlist |
| `/playlist/delete` | POST | Delete playlist |
| `/playlist/tracks` | POST | Add/remove songs |

## 5. Usage Examples

### 5.1 Login

```cpp
auto *client = ServiceLocator::instance()->service<NeteaseClient>();

// Phone number login
auto result = co_await client->login("13800138000", "password");
if (result.isSuccess()) {
    LOG_INFO("Netease", "Login successful");
} else {
    LOG_ERROR("Netease", "Login failed: " + result.error().message());
}

// QR code login
auto qrResult = co_await client->generateQrCode();
if (qrResult.isSuccess()) {
    QrCodeData qrData = qrResult.data();
    // Display QR code
    // ...
    
    // Poll for status
    while (true) {
        auto status = co_await client->checkQrCodeStatus(qrData.key);
        if (status.isSuccess()) {
            break;
        }
        co_await QThread::sleep(2);
    }
}
```

### 5.2 Search

```cpp
// Search songs
auto result = co_await client->searchSongs("Jay Chou", 30, 0);
if (result.isSuccess()) {
    for (const auto &song : result.data().songs) {
        qDebug() << song.title << song.artist;
    }
}

// Search playlists
auto playlistResult = co_await client->searchPlaylists("Chinese Pop");
```

### 5.3 Get Lyrics

```cpp
auto result = co_await client->getLyrics("12345");
if (result.isSuccess()) {
    Lyrics lyrics = result.data();
    for (const auto &line : lyrics.lines) {
        qDebug() << line.timestamp << line.text;
    }
}
```

### 5.4 Playlist Management

```cpp
// Create playlist
auto result = co_await client->createPlaylist("My Playlist", "Playlist description");

// Add song
co_await client->addSongToPlaylist(playlistId, songId);

// Remove song
co_await client->removeSongFromPlaylist(playlistId, songId);
```

## 6. Error Handling

```cpp
auto result = co_await client->getSongDetail("12345");
if (result.isError()) {
    ApiError error = result.error();
    
    if (error.isAuthError()) {
        // Need to re-authenticate
    } else if (error.isRateLimitError()) {
        // Rate limited, wait and retry
    } else if (error.isNetworkError()) {
        // Network error
    }
    
    LOG_ERROR("Netease", error.userMessage());
}
```

## 7. Testing

```cpp
class NeteaseClientTest : public QObject {
    Q_OBJECT
private slots:
    void testSearch();
    void testGetSongDetail();
    void testGetLyrics();
    void testAuthentication();
    void testPlaylistOperations();
};
```

## 8. Summary

The NetEase Cloud Music module provides complete NetEase Cloud Music API support:
- Multiple login methods (phone number, email, QR code)
- Complete search functionality (songs, playlists, albums, artists)
- Song playback URL and lyrics retrieval
- Playlist management (create, delete, add/remove songs)
- User data (liked songs, play history)
