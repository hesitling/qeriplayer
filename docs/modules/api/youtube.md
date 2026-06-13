# YouTube Music Module (youtube/)

## 1. Overview

The YouTube Music module provides a wrapper for the YouTube Music API, supporting home page browsing, search, playback, lyrics retrieval, and more.

## 2. Directory Structure

```
src/api/youtube/
├── YouTubeMusicClient.h/.cpp    # YouTube Music client
├── YouTubeMusicParser.h/.cpp    # Data parser
└── YouTubeMusicTypes.h          # Type definitions
```

## 3. Main Class Design

### 3.1 YouTubeMusicClient

YouTube Music client.

```cpp
class YouTubeMusicClient : public QObject {
    Q_OBJECT
public:
    explicit YouTubeMusicClient(HttpClient *httpClient,
                                QObject *parent = nullptr);
    
    // ==================== Authentication ====================
    
    // Cookie authentication
    QFuture<ApiResult<LoginResult>> authenticate(const QString &cookies);
    
    // Logout
    QFuture<ApiResult<VoidResult>> logout();
    
    // Check login status
    bool isAuthenticated() const;
    
    // ==================== Home Page ====================
    
    // Home page content
    QFuture<ApiResult<HomePage>> getHomePage();
    
    // Home carousels
    QFuture<ApiResult<QList<MusicCarousel>>> getHomeCarousels();
    
    // ==================== Search ====================
    
    // Search
    QFuture<ApiResult<SearchResult>> search(const QString &query,
                                            SearchFilter filter = {});
    
    // Search songs
    QFuture<ApiResult<SearchResult>> searchSongs(const QString &query);
    
    // Search playlists
    QFuture<ApiResult<SearchResult>> searchPlaylists(const QString &query);
    
    // Search albums
    QFuture<ApiResult<SearchResult>> searchAlbums(const QString &query);
    
    // Search artists
    QFuture<ApiResult<SearchResult>> searchArtists(const QString &query);
    
    // ==================== Songs ====================
    
    // Song details
    QFuture<ApiResult<SongDetail>> getSongDetail(const QString &videoId);
    
    // Playback URL
    QFuture<ApiResult<PlaybackUrl>> getSongUrl(const QString &videoId);
    
    // Lyrics
    QFuture<ApiResult<Lyrics>> getLyrics(const QString &videoId);
    
    // Similar songs
    QFuture<ApiResult<QList<Song>>> getSimilarSongs(const QString &videoId);
    
    // ==================== Playlists ====================
    
    // Playlist details
    QFuture<ApiResult<PlaylistDetail>> getPlaylistDetail(
        const QString &playlistId);
    
    // User playlists
    QFuture<ApiResult<QList<Playlist>>> getUserPlaylists();
    
    // Charts
    QFuture<ApiResult<QList<Playlist>>> getCharts();
    
    // ==================== Albums ====================
    
    // Album details
    QFuture<ApiResult<AlbumDetail>> getAlbumDetail(const QString &browseId);
    
    // New albums
    QFuture<ApiResult<QList<Album>>> getNewAlbums();
    
    // ==================== Artists ====================
    
    // Artist details
    QFuture<ApiResult<ArtistDetail>> getArtistDetail(const QString &browseId);
    
    // Artist songs
    QFuture<ApiResult<QList<Song>>> getArtistSongs(const QString &browseId);
    
    // Artist albums
    QFuture<ApiResult<QList<Album>>> getArtistAlbums(const QString &browseId);
    
    // ==================== Recommendations ====================
    
    // Recommended songs
    QFuture<ApiResult<QList<Song>>> getRecommendations();
    
    // Mood playlists
    QFuture<ApiResult<QList<Playlist>>> getMoodPlaylists();
    
    // Genres
    QFuture<ApiResult<QList<Genre>>> getGenres();
    
signals:
    void loginStateChanged(bool loggedIn);
    void errorOccurred(const ApiError &error);
    
private:
    HttpClient *m_httpClient;
    QString m_visitorData;
    QString m_cookies;
    bool m_authenticated = false;
};
```

## 4. Type Definitions

### 4.1 HomePage

Home page content.

```cpp
struct HomePage {
    QList<MusicCarousel> carousels;  // Carousels
    QList<Song> quickPicks;          // Quick picks
    QList<Playlist> playlists;       // Recommended playlists
};
```

### 4.2 MusicCarousel

Music carousel.

```cpp
struct MusicCarousel {
    QString title;           // Title
    QString browseId;        // Browse ID
    QList<Song> songs;       // Song list
    QList<Playlist> playlists; // Playlist list
};
```

### 4.3 Genre

Genre.

```cpp
struct Genre {
    QString id;        // Genre ID
    QString name;      // Name
    QString color;     // Color
};
```

## 5. API Endpoints

### 5.1 Home Page

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/browse` | POST | Home page content |
| `/browse` | POST | Carousel content |

### 5.2 Search

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/search` | POST | Search |

### 5.3 Songs

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/player` | POST | Playback URL |
| `/get_video_info` | GET | Video info |
| `/timedtext` | GET | Lyrics |

### 5.4 Playlists

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/browse` | POST | Playlist details |
| `/browse` | POST | User playlists |

## 6. Usage Examples

### 6.1 Home Page Browsing

```cpp
auto *client = ServiceLocator::instance()->service<YouTubeMusicClient>();

auto homeResult = co_await client->getHomePage();
if (homeResult.isSuccess()) {
    HomePage home = homeResult.data();
    
    // Display carousels
    for (const auto &carousel : home.carousels) {
        qDebug() << carousel.title;
    }
    
    // Display recommended songs
    for (const auto &song : home.quickPicks) {
        qDebug() << song.title << song.artist;
    }
}
```

### 6.2 Search

```cpp
auto searchResult = co_await client->searchSongs("lofi hip hop");
if (searchResult.isSuccess()) {
    for (const auto &song : searchResult.data().songs) {
        qDebug() << song.title << song.artist;
    }
}
```

### 6.3 Playback

```cpp
auto songResult = co_await client->getSongUrl("dQw4w9WgXcQ");
if (songResult.isSuccess()) {
    PlaybackUrl url = songResult.data();
    player->play(url.url);
}
```

### 6.4 Get Lyrics

```cpp
auto lyricsResult = co_await client->getLyrics("dQw4w9WgXcQ");
if (lyricsResult.isSuccess()) {
    Lyrics lyrics = lyricsResult.data();
    for (const auto &line : lyrics.lines) {
        qDebug() << line.timestamp << line.text;
    }
}
```

## 7. Special Handling

### 7.1 PoToken

YouTube uses PoToken for anti-scraping verification:

```cpp
// Get PoToken
QString poToken = generatePoToken(visitorData);

// Add to request
headers["X-Goog-Visitor-Id"] = visitorData;
```

### 7.2 JS Challenge

Some requests require JS Challenge:

```cpp
// Execute JS Challenge using JavaScript engine
QByteArray executeJsChallenge(const QByteArray &challenge);
```

## 8. Testing

```cpp
class YouTubeMusicClientTest : public QObject {
    Q_OBJECT
private slots:
    void testGetHomePage();
    void testSearch();
    void testGetSongDetail();
    void testGetSongUrl();
    void testGetLyrics();
};
```

## 9. Summary

The YouTube Music module provides complete YouTube Music platform support:
- Cookie authentication
- Home page browsing and recommendations
- Search functionality
- Song playback URL retrieval
- Lyrics retrieval
- Playlist, album, and artist information
