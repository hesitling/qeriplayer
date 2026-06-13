# API Common Types

## 1. Overview

This document defines the common data types and structures used by the API module.

## 2. Music Types

### 2.1 Song

Song information.

```cpp
struct Song {
    QString id;              // Unique identifier
    QString title;           // Song title
    QString artist;          // Artist name
    QString album;           // Album name
    QString coverUrl;        // Cover URL
    qint64 duration;         // Duration (milliseconds)
    MusicPlatform platform;  // Platform
    QVariantMap metadata;    // Extended metadata
    
    // Platform-specific fields
    QString neteaseId;       // NetEase ID
    QString bilibiliBvid;    // Bilibili BV number
    QString youtubeVideoId;  // YouTube video ID
    
    // Comparison operators
    bool operator==(const Song &other) const;
    bool operator!=(const Song &other) const;
};
```

### 2.2 Playlist

Playlist.

```cpp
struct Playlist {
    QString id;              // Unique identifier
    QString name;            // Name
    QString description;     // Description
    QString coverUrl;        // Cover URL
    QString creator;         // Creator
    QList<Song> songs;       // Song list
    int songCount;           // Song count
    QDateTime createdAt;     // Creation time
    QDateTime updatedAt;     // Update time
    MusicPlatform platform;  // Platform
};
```

### 2.3 Album

Album.

```cpp
struct Album {
    QString id;              // Unique identifier
    QString name;            // Name
    QString artist;          // Artist
    QString coverUrl;        // Cover URL
    QString description;     // Description
    QList<Song> songs;       // Song list
    int songCount;           // Song count
    QDateTime releaseDate;   // Release date
    MusicPlatform platform;  // Platform
};
```

### 2.4 Artist

Artist.

```cpp
struct Artist {
    QString id;              // Unique identifier
    QString name;            // Name
    QString avatarUrl;       // Avatar URL
    QString description;     // Description
    int songCount;           // Song count
    int albumCount;          // Album count
    MusicPlatform platform;  // Platform
};
```

### 2.5 Lyrics

Lyrics.

```cpp
struct Lyrics {
    struct Line {
        qint64 timestamp;          // Timestamp (milliseconds)
        QString text;              // Lyric text
        QString translation;       // Translation
        QList<WordTiming> words;   // Word-level lyrics
    };
    
    QList<Line> lines;       // Lyric lines
    QString rawText;         // Raw text
    QString translation;     // Full translation
    
    bool hasTranslation() const;
    bool hasWordTiming() const;
    
    // Get current line
    int currentLine(qint64 position) const;
    Line lineAt(int index) const;
};

// Word timing
struct WordTiming {
    qint64 startTime;  // Start time (milliseconds)
    qint64 endTime;    // End time (milliseconds)
    QString word;      // Word
};
```

### 2.6 UserProfile

User profile.

```cpp
struct UserProfile {
    QString id;              // User ID
    QString name;            // Username
    QString avatarUrl;       // Avatar URL
    QString description;     // Description
    MusicPlatform platform;  // Platform
};
```

## 3. Enumeration Types

### 3.1 MusicPlatform

Music platform.

```cpp
enum class MusicPlatform {
    All,          // All platforms
    Netease,      // NetEase Cloud Music
    Bilibili,     // Bilibili
    YouTubeMusic, // YouTube Music
    QQMusic,      // QQ Music
    Local         // Local
};
```

### 3.2 AudioQuality

Audio quality.

```cpp
enum class AudioQuality {
    Low = 128000,       // 128kbps
    Medium = 192000,    // 192kbps
    High = 320000,      // 320kbps
    Lossless = 1000000  // Lossless
};
```

### 3.3 SearchType

Search type.

```cpp
enum class SearchType {
    All,       // Comprehensive search
    Song,      // Song
    Playlist,  // Playlist
    Album,     // Album
    Artist     // Artist
};
```

## 4. Search Types

### 4.1 SearchResult

Search result.

```cpp
struct SearchResult {
    MusicPlatform platform;    // Platform
    SearchType type;           // Type
    QList<Song> songs;         // Song list
    QList<Playlist> playlists; // Playlist list
    QList<Album> albums;       // Album list
    QList<Artist> artists;     // Artist list
    int totalCount;            // Total count
    bool hasMore;              // Has more results
    QString nextPageToken;     // Next page token
    QString query;             // Query string
};
```

### 4.2 SearchFilter

Search filter.

```cpp
struct SearchFilter {
    SearchType type = SearchType::All;
    MusicPlatform platform = MusicPlatform::All;
    AudioQuality minQuality = AudioQuality::Low;
    QString language;          // Language
    QString region;            // Region
};
```

## 5. API Errors

### 5.1 ApiError

API error class.

```cpp
class ApiError {
public:
    ApiError(int code, const QString &message, 
             const QString &details = {});
    
    int code() const;           // Error code
    QString message() const;    // Error message
    QString details() const;    // Details
    
    // Error type checks
    bool isNetworkError() const;    // Network error
    bool isAuthError() const;       // Authentication error
    bool isRateLimitError() const;  // Rate limit error
    bool isNotFoundError() const;   // Not found
    
    // User-friendly error message
    QString userMessage() const;
    
private:
    int m_code;
    QString m_message;
    QString m_details;
};
```

### 5.2 ApiResult

API result template class.

```cpp
template<typename T>
class ApiResult {
public:
    ApiResult(T data);
    ApiResult(ApiError error);
    
    bool isSuccess() const;
    bool isError() const;
    
    T data() const;
    ApiError error() const;
    
    // Implicit conversion
    operator bool() const { return isSuccess(); }
    
private:
    bool m_success;
    T m_data;
    ApiError m_error;
};
```

## 6. Detail Types

### 6.1 SongDetail

Song detail.

```cpp
struct SongDetail {
    Song song;                 // Basic information
    Lyrics lyrics;             // Lyrics
    QList<Song> similarSongs;  // Similar songs
    QString playbackUrl;       // Playback URL
    AudioQuality quality;      // Audio quality
};
```

### 6.2 PlaylistDetail

Playlist detail.

```cpp
struct PlaylistDetail {
    Playlist playlist;         // Basic information
    QList<Song> tracks;        // Complete song list
    QStringList tags;          // Tags
    int playCount;             // Play count
    int shareCount;            // Share count
};
```

### 6.3 AlbumDetail

Album detail.

```cpp
struct AlbumDetail {
    Album album;               // Basic information
    QList<Song> tracks;        // Complete song list
    QString company;           // Distribution company
    QStringList genres;        // Genres
};
```

### 6.4 ArtistDetail

Artist detail.

```cpp
struct ArtistDetail {
    Artist artist;             // Basic information
    QList<Song> hotSongs;      // Hot songs
    QList<Album> albums;       // Album list
    QStringList genres;        // Genres
};
```

## 7. Usage Examples

```cpp
// Create song
Song song;
song.id = "12345";
song.title = "Song Title";
song.artist = "Artist Name";
song.album = "Album Name";
song.duration = 180000;
song.platform = MusicPlatform::Netease;

// Search result handling
SearchResult result;
if (result.songs.isEmpty()) {
    // No results
} else {
    for (const auto &song : result.songs) {
        qDebug() << song.title << song.artist;
    }
}

// API result handling
ApiResult<Song> result = co_await client->getSongDetail("12345");
if (result.isSuccess()) {
    Song song = result.data();
    // Process song
} else {
    ApiError error = result.error();
    // Handle error
}
```
