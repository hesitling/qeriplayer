# QQ Music Module (qqmusic/)

## 1. Overview

The QQ Music module provides a wrapper for the QQ Music API, primarily used for metadata completion (lyrics, cover art).

## 2. Directory Structure

```
src/api/qqmusic/
├── QQMusicClient.h/.cpp    # QQ Music client
├── QQMusicParser.h/.cpp    # Data parser
└── QQMusicTypes.h          # Type definitions
```

## 3. Main Class Design

### 3.1 QQMusicClient

QQ Music client.

```cpp
class QQMusicClient : public QObject {
    Q_OBJECT
public:
    explicit QQMusicClient(HttpClient *httpClient,
                           QObject *parent = nullptr);
    
    // ==================== Search ====================
    
    // Search songs
    QCoro::Task<ApiResult<SearchResult>> searchSongs(const QString &keyword,
                                                  int limit = 30);
    
    // ==================== Songs ====================
    
    // Song details
    QCoro::Task<ApiResult<SongDetail>> getSongDetail(const QString &songMid);
    
    // Lyrics
    QCoro::Task<ApiResult<Lyrics>> getLyrics(const QString &songMid);
    
    // Cover art
    QCoro::Task<ApiResult<QString>> getCoverUrl(const QString &songMid);
    
    // Playback URL (optional)
    QCoro::Task<ApiResult<PlaybackUrl>> getSongUrl(const QString &songMid,
                                                AudioQuality quality = AudioQuality::High);
    
private:
    HttpClient *m_httpClient;
};
```

## 4. Type Definitions

### 4.1 QQMusicSong

QQ Music song information.

```cpp
struct QQMusicSong {
    QString songMid;        // Song MID
    QString songName;       // Song name
    QString singerMid;      // Singer MID
    QString singerName;     // Singer name
    QString albumMid;       // Album MID
    QString albumName;      // Album name
    int duration;           // Duration (seconds)
    QString coverUrl;       // Cover URL
};
```

## 5. API Endpoints

### 5.1 Search

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/soso/fcgi-bin/client_search_cp` | GET | Search songs |

### 5.2 Songs

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/v8/fcg-bin/fcg_v8_album_detail_cp.fcgi` | GET | Album details |
| `/lyric/fcgi-bin/fcg_query_lyric_new.fcgi` | GET | Lyrics |
| `/img/` | GET | Cover image |

## 6. Usage Examples

### 6.1 Search Songs

```cpp
auto *client = ServiceLocator::instance()->service<QQMusicClient>();

auto result = co_await client->searchSongs("Jay Chou");
if (result.isSuccess()) {
    for (const auto &song : result.data().songs) {
        qDebug() << song.title << song.artist;
    }
}
```

### 6.2 Get Lyrics

```cpp
auto lyricsResult = co_await client->getLyrics("003OUlho2HcRHC");
if (lyricsResult.isSuccess()) {
    Lyrics lyrics = lyricsResult.data();
    for (const auto &line : lyrics.lines) {
        qDebug() << line.timestamp << line.text;
    }
}
```

### 6.3 Get Cover Art

```cpp
auto coverResult = co_await client->getCoverUrl("003OUlho2HcRHC");
if (coverResult.isSuccess()) {
    QString coverUrl = coverResult.data();
    // Display cover art
}
```

## 7. Metadata Completion

The QQ Music module is primarily used for metadata completion scenarios:

```cpp
class MetadataCompleter {
public:
    // Complete song metadata
    QCoro::Task<SongMetadata> completeMetadata(const Song &song) {
        // 1. Try NetEase first
        auto neteaseResult = co_await m_neteaseClient->searchSongs(
            song.title + " " + song.artist);
        
        if (neteaseResult.isSuccess() && 
            !neteaseResult.data().songs.isEmpty()) {
            // Use NetEase data
            return buildMetadata(neteaseResult.data().songs.first());
        }
        
        // 2. Try QQ Music
        auto qqResult = co_await m_qqMusicClient->searchSongs(
            song.title + " " + song.artist);
        
        if (qqResult.isSuccess() && 
            !qqResult.data().songs.isEmpty()) {
            // Use QQ Music data
            return buildMetadata(qqResult.data().songs.first());
        }
        
        // 3. Use original data
        return SongMetadata(song);
    }
    
private:
    NeteaseClient *m_neteaseClient;
    QQMusicClient *m_qqMusicClient;
};
```

## 8. Testing

```cpp
class QQMusicClientTest : public QObject {
    Q_OBJECT
private slots:
    void testSearchSongs();
    void testGetSongDetail();
    void testGetLyrics();
    void testGetCoverUrl();
};
```

## 9. Limitations

The current QQ Music module has limited functionality:
- No login support
- No playback support
- No playlist management
- Used primarily for metadata completion

## 10. Summary

The QQ Music module provides basic QQ Music platform support:
- Song search
- Lyrics retrieval
- Cover art retrieval
- Primarily used for metadata completion scenarios
