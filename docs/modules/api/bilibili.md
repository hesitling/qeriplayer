# Bilibili Module (bilibili/)

## 1. Overview

The Bilibili module provides a wrapper for the Bilibili API, supporting video search, playback, favorites management, and more.

## 2. Directory Structure

```
src/api/bilibili/
├── BilibiliClient.h/.cpp    # Bilibili client
├── BilibiliParser.h/.cpp    # Data parser
└── BilibiliTypes.h          # Type definitions
```

## 3. Main Class Design

### 3.1 BilibiliClient

Bilibili client.

```cpp
class BilibiliClient : public QObject {
    Q_OBJECT
public:
    explicit BilibiliClient(HttpClient *httpClient,
                            QObject *parent = nullptr);
    
    // ==================== Authentication ====================
    
    // QR code login
    QFuture<ApiResult<QrCodeData>> generateQrCode();
    QFuture<ApiResult<LoginResult>> checkQrCodeStatus(const QString &key);
    
    // Logout
    QFuture<ApiResult<VoidResult>> logout();
    
    // Check login status
    bool isAuthenticated() const;
    
    // Get user profile
    QFuture<ApiResult<UserProfile>> getUserProfile();
    
    // ==================== Search ====================
    
    // Search videos
    QFuture<ApiResult<SearchResult>> searchVideos(const QString &keyword,
                                                   int page = 1,
                                                   int pageSize = 20);
    
    // Search anime
    QFuture<ApiResult<SearchResult>> searchBangumi(const QString &keyword,
                                                    int page = 1,
                                                    int pageSize = 20);
    
    // Search users
    QFuture<ApiResult<SearchResult>> searchUsers(const QString &keyword,
                                                  int page = 1,
                                                  int pageSize = 20);
    
    // Hot searches
    QFuture<ApiResult<QStringList>> getHotSearches();
    
    // ==================== Videos ====================
    
    // Video details
    QFuture<ApiResult<VideoDetail>> getVideoDetail(const QString &bvid);
    QFuture<ApiResult<VideoDetail>> getVideoDetail(int avid);
    
    // Video pages
    QFuture<ApiResult<QList<VideoPage>>> getVideoPages(const QString &bvid);
    
    // Video stream
    QFuture<ApiResult<VideoStream>> getVideoStream(const QString &bvid,
                                                    int cid,
                                                    VideoQuality quality = VideoQuality::Q720P);
    
    // Audio stream
    QFuture<ApiResult<AudioStream>> getAudioStream(const QString &bvid,
                                                    int cid);
    
    // Recommended videos
    QFuture<ApiResult<QList<VideoDetail>>> getRecommendedVideos();
    
    // Popular videos
    QFuture<ApiResult<QList<VideoDetail>>> getPopularVideos(int page = 1);
    
    // ==================== Favorites ====================
    
    // User favorites
    QFuture<ApiResult<QList<FavoriteList>>> getUserFavorites(
        const QString &userId);
    
    // Favorite details
    QFuture<ApiResult<FavoriteDetail>> getFavoriteDetail(
        int mediaId, int page = 1, int pageSize = 20);
    
    // Create favorite
    QFuture<ApiResult<FavoriteList>> createFavorite(
        const QString &name, const QString &description = {});
    
    // Delete favorite
    QFuture<ApiResult<VoidResult>> deleteFavorite(int mediaId);
    
    // Add video to favorite
    QFuture<ApiResult<VoidResult>> addVideoToFavorite(
        int mediaId, const QString &bvid);
    
    // Remove video from favorite
    QFuture<ApiResult<VoidResult>> removeVideoFromFavorite(
        int mediaId, const QString &bvid);
    
    // ==================== User ====================
    
    // User uploads
    QFuture<ApiResult<QList<VideoDetail>>> getUserVideos(
        const QString &userId, int page = 1, int pageSize = 30);
    
    // History
    QFuture<ApiResult<QList<VideoDetail>>> getHistory(
        int page = 1, int pageSize = 20);
    
    // User stats
    QFuture<ApiResult<UserStat>> getUserStat();
    
    // Following list
    QFuture<ApiResult<QList<UserProfile>>> getFollowing(
        const QString &userId, int page = 1, int pageSize = 20);
    
signals:
    void loginStateChanged(bool loggedIn);
    void errorOccurred(const ApiError &error);
    
private:
    HttpClient *m_httpClient;
    QString m_cookie;
    QString m_csrfToken;
    bool m_authenticated = false;
};
```

## 4. Type Definitions

### 4.1 VideoQuality

Video quality.

```cpp
enum class VideoQuality {
    Q240P = 6,
    Q360P = 16,
    Q480P = 32,
    Q720P = 64,
    Q720P60 = 74,
    Q1080P = 80,
    Q1080P60 = 112,
    Q4K = 120
};
```

### 4.2 VideoDetail

Video detail.

```cpp
struct VideoDetail {
    QString bvid;           // BV number
    int avid;               // AV number
    QString title;          // Title
    QString description;    // Description
    QString coverUrl;       // Cover URL
    QString creator;        // UP主 (creator)
    int duration;           // Duration (seconds)
    QDateTime createdAt;    // Creation time
    int viewCount;          // View count
    int danmakuCount;       // Danmaku count
    int likeCount;          // Like count
    int coinCount;          // Coin count
    int favoriteCount;      // Favorite count
    QList<VideoPage> pages; // Page list
};
```

### 4.3 VideoPage

Video page.

```cpp
struct VideoPage {
    int cid;        // Content ID
    int page;       // Page number
    QString title;  // Title
    int duration;   // Duration (seconds)
};
```

### 4.4 FavoriteList

Favorite list.

```cpp
struct FavoriteList {
    int id;                 // Favorite ID
    QString title;          // Title
    QString description;    // Description
    int mediaCount;         // Video count
    QString coverUrl;       // Cover URL
};
```

### 4.5 UserStat

User stats.

```cpp
struct UserStat {
    int following;      // Following count
    int follower;       // Follower count
    int like;           // Like count
    int coin;           // Coin count
    int favorite;       // Favorite count
};
```

## 5. API Endpoints

### 5.1 Authentication

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/qrcode/getLoginUrl` | GET | Get login URL |
| `/qrcode/getLoginInfo` | POST | Check login status |

### 5.2 Search

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/search/type` | GET | Category search |
| `/search/hot` | GET | Hot searches |

### 5.3 Videos

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/view` | GET | Video details |
| `/pagelist` | GET | Page list |
| `/player/playurl` | GET | Playback URL |
| `/recsys/arc` | GET | Recommended videos |

### 5.4 Favorites

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/favFolder/created/list-all` | GET | User favorites |
| `/fav/resourceList` | GET | Favorite details |
| `/folder/create` | POST | Create favorite |
| `/folder/delete` | POST | Delete favorite |
| `/fav/deal` | POST | Add/remove favorites |

## 6. Usage Examples

### 6.1 Search Videos

```cpp
auto *client = ServiceLocator::instance()->service<BilibiliClient>();

auto result = co_await client->searchVideos("music", 1, 20);
if (result.isSuccess()) {
    for (const auto &video : result.data().videos) {
        qDebug() << video.title << video.creator;
    }
}
```

### 6.2 Get Video Playback URL

```cpp
auto detailResult = co_await client->getVideoDetail("BV1xx411c7mD");
if (detailResult.isSuccess()) {
    VideoDetail detail = detailResult.data();
    
    // Get playback URL for the first page
    if (!detail.pages.isEmpty()) {
        auto streamResult = co_await client->getVideoStream(
            detail.bvid, detail.pages[0].cid);
        
        if (streamResult.isSuccess()) {
            VideoStream stream = streamResult.data();
            // Play video
        }
    }
}
```

### 6.3 Favorites Management

```cpp
// Get user favorites
auto favoritesResult = co_await client->getUserFavorites(userId);
if (favoritesResult.isSuccess()) {
    for (const auto &fav : favoritesResult.data()) {
        qDebug() << fav.title << fav.mediaCount;
    }
}

// Create favorite
auto createResult = co_await client->createFavorite("My Favorites", "Favorite description");

// Add video to favorite
co_await client->addVideoToFavorite(mediaId, bvid);
```

## 7. Audio Extraction

The Bilibili module supports extracting audio from videos:

```cpp
// Get audio stream
auto audioResult = co_await client->getAudioStream(bvid, cid);
if (audioResult.isSuccess()) {
    AudioStream audio = audioResult.data();
    // Play audio
    player->play(audio.url);
}
```

## 8. Testing

```cpp
class BilibiliClientTest : public QObject {
    Q_OBJECT
private slots:
    void testSearchVideos();
    void testGetVideoDetail();
    void testGetVideoStream();
    void testGetAudioStream();
    void testFavorites();
};
```

## 9. Summary

The Bilibili module provides complete Bilibili platform support:
- QR code login
- Video search and playback
- Audio extraction
- Favorites management
- User data access
