## 1. Domain Models

- [x] 1.1 Create `src/domain/` directory and `Domain.h` umbrella header
- [x] 1.2 Define `MusicPlatform` enum with Unknown, NetEase, Bilibili, YouTube, QQMusic values
- [x] 1.3 Define `SearchType` enum with Song, Album, Artist, Playlist, All values
- [x] 1.4 Define `PlaybackState` enum with Stopped, Playing, Paused, Loading, Error values
- [x] 1.5 Define `RepeatMode` enum with Off, One, All values
- [x] 1.6 Define `AudioQuality` enum with Low, Standard, High, Lossless values
- [x] 1.7 Define `Song` struct with all fields and default values
- [x] 1.8 Define `Album` struct with all fields
- [x] 1.9 Define `Artist` struct with all fields
- [x] 1.10 Define `Playlist` struct with embedded songs vector
- [x] 1.11 Define `LyricLine` and `Lyrics` structs
- [x] 1.12 Define `SearchResult` struct
- [x] 1.13 Register all types with Q_DECLARE_METATYPE for QVariant interop

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

- [ ] 3.1 Create `src/core/logger/` directory structure
- [ ] 3.2 Add spdlog dependency to CMakeLists.txt
- [ ] 3.3 Implement `Logger` class with initialize() and get(name) methods
- [ ] 3.4 Configure file sink with daily rotation and 7-day retention
- [ ] 3.5 Configure console sink with ANSI color support detection
- [ ] 3.6 Define log format: `[timestamp] [level] [logger] message`
- [ ] 3.7 Implement runtime log level change
- [ ] 3.8 Create predefined logger categories: "app", "network", "player", "api", "ui"

## 4. FileSystem Module

- [ ] 4.1 Create `src/core/filesystem/` directory structure
- [ ] 4.2 Implement `AppPaths::dataDir()` with platform-specific paths and auto-creation
- [ ] 4.3 Implement `AppPaths::configDir()` with platform-specific paths and auto-creation
- [ ] 4.4 Implement `AppPaths::cacheDir()` with platform-specific paths and auto-creation
- [ ] 4.5 Implement `AppPaths::tempDir()` with auto-creation
- [ ] 4.6 Implement `FileUtils::ensureDir()` for recursive directory creation
- [ ] 4.7 Implement `FileUtils::readFile()` returning QByteArray
- [ ] 4.8 Implement `FileUtils::writeFile()` with atomic write (temp + rename)
- [ ] 4.9 Implement `FileWatcher` class with fileChanged signal

## 5. Crypto Module

- [ ] 5.1 Create `src/core/crypto/` directory structure
- [ ] 5.2 Add OpenSSL dependency check to CMakeLists.txt
- [ ] 5.3 Implement `Encryptor` class with AES-256-GCM encrypt (12-byte random nonce prepended)
- [ ] 5.4 Implement `Decryptor` class with AES-256-GCM decrypt and tag verification
- [ ] 5.5 Define `CryptoError` exception type
- [ ] 5.6 Implement `CryptoUtils::generateKey()` for 32-byte random key
- [ ] 5.7 Implement `CryptoUtils::sha256()` returning hex string
- [ ] 5.8 Implement `SecureStorage` class with encrypted key-value persistence on disk
- [ ] 5.9 Implement encryption key derivation/storage using OS secure storage (Keychain/DPAPI/Secret Service)

## 6. App Bootstrap Integration

- [ ] 6.1 Add getter methods to `ServiceLocator` for DatabaseManager, Logger, AppPaths, SecureStorage
- [ ] 6.2 Update `NeriPlayerApplication::initializeCoreServices()` to create and register all new services
- [ ] 6.3 Ensure initialization order: Logger → FileSystem → Database → Crypto
- [ ] 6.4 Implement graceful shutdown in `NeriPlayerApplication` destructor (close DB, flush logs)
- [ ] 6.5 Update CMakeLists.txt to include all new source files

## 7. Verification

- [ ] 7.1 Build the project and verify no compilation errors
- [ ] 7.2 Run the application and verify all services initialize without error
- [ ] 7.3 Verify database file is created on first run with correct schema
- [ ] 7.4 Verify log files are created with expected format and rotation
- [ ] 7.5 Verify SecureStorage can store and retrieve encrypted values
