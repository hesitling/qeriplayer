# Domain Models (domain/)

## Overview

The domain layer contains all shared value types used across API clients, repositories, services, and UI. All structs are plain value types, default-constructible, copyable, and registered with `Q_DECLARE_METATYPE`. `Song` uses `Q_GADGET` with `Q_PROPERTY(READ)` to enable QML property access — other structs have no QObject dependency.

## Source Files

```text
src/domain/
├── Domain.h                # Umbrella header
├── Enums.h                 # MusicPlatform, SearchType, PlaybackState, RepeatMode, AudioQuality, etc.
├── Song.h                  # Song model
├── Album.h                 # Album model
├── Artist.h                # Artist model
├── Playlist.h              # Playlist model
├── PlaylistSummary.h       # PlaylistSummary, AlbumSummary, BiliPlaylist
├── Lyrics.h                # Lyrics, LyricLine, LyricWord
├── SearchResult.h          # Aggregated search results
├── SongUrlResult.h         # SongUrlResult, AudioInfo
├── SongIdentity.h          # Cross-platform song deduplication
├── PersistedPlayerState.h  # Player state for persistence
└── ListenTogether.h        # Listen Together domain types
```

## Enums

```cpp
enum class MusicPlatform : uint8_t { Unknown, NetEase, Bilibili, YouTube, QQMusic };
enum class SearchType : uint8_t { Song, Album, Artist, Playlist, All };
enum class PlaybackState : uint8_t { Stopped, Playing, Paused, Loading, Error };
enum class RepeatMode : uint8_t { Off, One, All };
enum class AudioQuality : uint8_t { Low, Standard, High, Lossless };
enum class PlaybackAudioSource : uint8_t { Local, NetEase, Bilibili, YouTube };
enum class BiliPlaylistKind : uint8_t { CreatedFavorite, CollectedFavorite, Collection };
```

## Song

Core domain model aligned with Android QeriPlayer's `SongItem`.

```cpp
struct Song {
    Q_GADGET
    Q_PROPERTY(QString id READ getId CONSTANT)
    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QString artist READ getArtist CONSTANT)
    Q_PROPERTY(QString album READ getAlbum CONSTANT)
    Q_PROPERTY(qint64 durationMs READ getDurationMs CONSTANT)
    Q_PROPERTY(QUrl coverUrl READ getCoverUrl CONSTANT)
    Q_PROPERTY(MusicPlatform platform READ getPlatform CONSTANT)

    // Core identity
    QString id, name, artist, album, albumId;
    qint64 durationMs = 0;
    QUrl coverUrl, mediaUri;
    MusicPlatform platform = MusicPlatform::Unknown;

    // Lyric matching
    QString matchedLyric, matchedTranslatedLyric;
    MusicPlatform matchedLyricSource = MusicPlatform::Unknown;
    QString matchedSongId;
    qint64 userLyricOffsetMs = 0;
    Lyrics lyrics; // Parsed lyrics with timing

    // User customizations
    QString customCoverUrl, customName, customArtist;

    // Original values (before user edits)
    QString originalName, originalArtist, originalCoverUrl;
    QString originalLyric, originalTranslatedLyric;

    // Local file support
    QString localFileName, localFilePath;

    // Platform-specific identifiers
    QString channelId, audioId, subAudioId, playlistContextId, streamUrl;

    // Arbitrary platform data
    QVariantMap extra;
};
```

## Album

```cpp
struct Album {
    QString id, name, artist;
    QUrl coverUrl;
    int size = 0; // Track count
    MusicPlatform platform = MusicPlatform::Unknown;
};
```

## Artist

```cpp
struct Artist {
    QString id, name;
    QUrl avatarUrl;
    QString description;
    MusicPlatform platform = MusicPlatform::Unknown;
};
```

## Playlist

```cpp
struct Playlist {
    QString id, name, description;
    QUrl coverUrl;
    int songCount = 0;
    QString owner;
    MusicPlatform platform = MusicPlatform::Unknown;
    QVector<Song> songs;    // Optionally populated
    qint64 modifiedAt = 0;  // Epoch milliseconds
    QString customCoverUrl;
};
```

## PlaylistSummary / AlbumSummary / BiliPlaylist

Lightweight summary types for list/card UIs.

```cpp
struct PlaylistSummary {
    QString id, name;
    QUrl coverUrl;
    qint64 playCount = 0;
    int trackCount = 0;
};

struct AlbumSummary {
    QString id, name;
    QUrl coverUrl;
    int size = 0;
};

struct BiliPlaylist {
    qint64 mediaId = 0, fid = 0, mid = 0;
    QString title;
    int count = 0;
    QUrl coverUrl;
    BiliPlaylistKind kind = BiliPlaylistKind::CreatedFavorite;
    QString subtitle;
};
```

## Lyrics

```cpp
struct LyricWord {
    QString text;
    qint64 startTimeMs = 0, endTimeMs = 0;
};

struct LyricLine {
    qint64 startTimeMs = 0, endTimeMs = 0;
    QString text;
    QVector<LyricWord> words; // Optional word-level timing
};

struct Lyrics {
    QString rawText;
    QVector<LyricLine> lines; // Sorted by startTimeMs ascending
};
```

## SearchResult

```cpp
struct SearchResult {
    QVector<Song> songs;
    QVector<Album> albums;
    QVector<Artist> artists;
    QVector<Playlist> playlists;
    int totalCount = 0;
    bool hasMore = false;
};
```

## SongUrlResult

```cpp
struct AudioInfo {
    QString qualityKey, qualityLabel, codecLabel, mimeType;
    int bitrateKbps = 0, sampleRateHz = 0, bitDepth = 0, channelCount = 0;
};

struct SongUrlResult {
    enum class Status : uint8_t { Success, WaitingForAuthoritativeStream, RequiresLogin, Failure };

    Status status = Status::Failure;
    QString url;
    qint64 durationMs = 0;
    QString mimeType, noticeMessage;
    qint64 expectedContentLength = 0;
    AudioInfo audioInfo;
    QString cacheKeyOverride;
};
```

## SongIdentity

Cross-platform song deduplication key.

```cpp
struct SongIdentity {
    QString id, album, mediaUri;
    QString stableKey() const; // "id|album|mediaUri"
    bool operator==(const SongIdentity &other) const;
};
```

## PersistedPlayerState

```cpp
struct PersistedPlayerState {
    QVector<Song> playlist;
    int currentIndex = 0;
    QString mediaUrl;
    qint64 positionMs = 0;
    bool shouldResumePlayback = false;
    RepeatMode repeatMode = RepeatMode::Off;
    bool shuffleEnabled = false;
};
```

## Listen Together

Full set of Listen Together domain types for synchronized playback: `ListenTogetherTrack`, `ListenTogetherRoomState`, `ListenTogetherMember`, `ListenTogetherPlaybackState`, `ListenTogetherRoomSettings`, `ListenTogetherEvent`, `ListenTogetherSessionState`, `ListenTogetherCause`, `ListenTogetherConnectionState`.

Includes conversion helpers: `songToTrack()`, `trackToSong()`, `buildStableTrackKey()`, `resolvedChannelId()`, `resolvedAudioId()`.

See `src/domain/ListenTogether.h` for full details.
