## 1. Domain Models

- [x] 1.1 Create `src/domain/` directory and `Domain.h` umbrella header
- [x] 1.2 Define `MusicPlatform` enum with Unknown, NetEase, Bilibili, YouTube, QQMusic values
- [x] 1.3 Define `SearchType` enum with Song, Album, Artist, Playlist, All values
- [x] 1.4 Define `PlaybackState` enum with Stopped, Playing, Paused, Loading, Error values
- [x] 1.5 Define `RepeatMode` enum with Off, One, All values
- [x] 1.6 Define `AudioQuality` enum with Low, Standard, High, Lossless values
- [x] 1.7 Define `PlaybackAudioSource` enum with Local, NetEase, Bilibili, YouTube values
- [x] 1.8 Define `BiliPlaylistKind` enum with CreatedFavorite, CollectedFavorite, Collection values
- [x] 1.9 Define `Song` struct with all fields aligned with Android SongItem (name, durationMs, mediaUri, lyric matching, customizations, local file, platform identifiers)
- [x] 1.10 Define `Album` struct with fields aligned with Android AlbumSummary (name, size)
- [x] 1.11 Define `Artist` struct with all fields
- [x] 1.12 Define `Playlist` struct with embedded songs vector, modifiedAt, customCoverUrl
- [x] 1.13 Define `LyricWord`, `LyricLine`, and `Lyrics` structs with word-level timing
- [x] 1.14 Define `SearchResult` struct
- [x] 1.15 Define `SongIdentity` struct with stableKey() for cross-platform deduplication
- [x] 1.16 Define `SongUrlResult` and `AudioInfo` structs for URL resolution
- [x] 1.17 Define `PlaylistSummary`, `AlbumSummary`, and `BiliPlaylist` lightweight models
- [x] 1.18 Define `PersistedPlayerState` struct for player state persistence
- [x] 1.19 Register all types with Q_DECLARE_METATYPE for QVariant interop
- [x] 1.20 Define `ListenTogetherTrack` wire-format struct (stableKey, channelId, audioId, subAudioId, playlistContextId, mediaUri, streamUrl, name, artist, album, durationMs, coverUrl)
- [x] 1.21 Define `ListenTogetherRoomState`, `ListenTogetherMember`, `ListenTogetherPlaybackState`, `ListenTogetherRoomSettings`
- [x] 1.22 Define `ListenTogetherEvent` for control events (SET_TRACK, PLAY, PAUSE, SEEK, HEARTBEAT, etc.)
- [x] 1.23 Define `ListenTogetherConnectionState` enum and `ListenTogetherSessionState`
- [x] 1.24 Define `ListenTogetherCause` and `ListenTogetherChannels`/`ListenTogetherRoomStatuses` constants
- [x] 1.25 Implement `songToTrack()` and `trackToSong()` conversion helpers matching Android's mapper logic
- [x] 1.26 Implement `buildStableTrackKey()`, `resolvedChannelId()`, `resolvedAudioId()` matching Android's logic

## 2. Database Module

- [x] 2.1 Create `src/core/database/` directory structure
- [x] 2.2 Implement `DatabaseManager` class with open/close lifecycle
- [x] 2.3 Implement `SchemaManager` with version table and migration registration
- [x] 2.4 Implement `QueryHelper` with positional and named parameter binding
- [x] 2.5 Implement SELECT query execution returning rows as QVector<QVariant>
- [x] 2.6 Implement INSERT/UPDATE/DELETE execution with affected row count
- [x] 2.7 Implement transaction support (begin, commit, rollback)
- [x] 2.8 Define `DatabaseError` exception type
- [x] 2.9 Write initial migration (version 1) creating songs_cache, playlists, playlist_songs, settings, play_history tables
- [x] 2.10 Add SQLite dependency to CMakeLists.txt (bundled or system)

## 3. Logger Module

- [x] 3.1 Create `src/core/logger/` directory structure
- [x] 3.2 Add spdlog dependency to CMakeLists.txt
- [x] 3.3 Implement `Logger` class with initialize() and get(name) methods
- [x] 3.4 Configure file sink with daily rotation and 7-day retention
- [x] 3.5 Configure console sink with ANSI color support detection
- [x] 3.6 Define log format: `[timestamp] [level] [logger] message`
- [x] 3.7 Implement runtime log level change
- [x] 3.8 Create predefined logger categories: "app", "network", "player", "api", "ui"

## 4. FileSystem Module

- [x] 4.1 Create `src/core/filesystem/` directory structure
- [x] 4.2 Implement `AppPaths::dataDir()` with platform-specific paths and auto-creation
- [x] 4.3 Implement `AppPaths::configDir()` with platform-specific paths and auto-creation
- [x] 4.4 Implement `AppPaths::cacheDir()` with platform-specific paths and auto-creation
- [x] 4.5 Implement `AppPaths::tempDir()` with auto-creation
- [x] 4.6 Implement `FileUtils::ensureDir()` for recursive directory creation
- [x] 4.7 Implement `FileUtils::readFile()` returning QByteArray
- [x] 4.8 Implement `FileUtils::writeFile()` with atomic write (temp + rename)
- [x] 4.9 Implement `FileWatcher` class with fileChanged signal

## 5. Crypto Module

- [x] 5.1 Create `src/core/crypto/` directory structure
- [x] 5.2 Add OpenSSL dependency check to CMakeLists.txt
- [x] 5.3 Implement `Encryptor` class with AES-256-GCM encrypt (12-byte random nonce prepended)
- [x] 5.4 Implement `Decryptor` class with AES-256-GCM decrypt and tag verification
- [x] 5.5 Define `CryptoError` exception type
- [x] 5.6 Implement `CryptoUtils::generateKey()` for 32-byte random key
- [x] 5.7 Implement `CryptoUtils::sha256()` returning hex string
- [x] 5.8 Implement `SecureStorage` class with encrypted key-value persistence on disk
- [x] 5.9 Implement encryption key derivation/storage with per-machine random key sidecar file (0600 permissions)

## 6. App Bootstrap Integration

- [x] 6.1 Add getter methods to `ServiceLocator` for DatabaseManager, Logger, AppPaths, SecureStorage
- [x] 6.2 Update `NeriPlayerApplication::initializeCoreServices()` to create and register all new services
- [x] 6.3 Ensure initialization order: Logger → FileSystem → Database → Crypto
- [x] 6.4 Implement graceful shutdown in `NeriPlayerApplication` destructor (close DB, flush logs)
- [x] 6.5 Update CMakeLists.txt to include all new source files

## 7. Verification

- [x] 7.1 Build the project and verify no compilation errors
- [x] 7.2 Run the application and verify all services initialize without error
- [x] 7.3 Verify database file is created on first run with correct schema
- [x] 7.4 Verify log files are created with expected format and rotation
- [x] 7.5 Verify SecureStorage can store and retrieve encrypted values
