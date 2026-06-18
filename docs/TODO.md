# QeriPlayer Qt — Long-Term Implementation Roadmap

This document tracks the full implementation plan for QeriPlayer Qt, organized by phase. Each phase builds on the previous one. Check off items as they are completed.

---

## Phase 1: Foundation Layer

Build the core infrastructure that all other modules depend on.

- [x] **Domain Models** (`src/domain/`)
  - [x] `Song` — id, title, artist, album, duration, coverUrl, platform, playbackUrl
  - [x] `Album` — id, title, artist, coverUrl, song list
  - [x] `Artist` — id, name, avatarUrl, description
  - [x] `Playlist` — id, name, description, coverUrl, song count, owner
  - [x] `Lyrics` — raw text, timed lines with timestamps
  - [x] `SearchResult` — list of songs, playlists, albums, artists with pagination
  - [x] Enums — `MusicPlatform`, `SearchType`, `PlaybackState`, `RepeatMode`, `AudioQuality`
  - [ ] Value types — `PageInfo`, `Duration`, `CoverImage`

- [x] **Database Module** (`src/core/database/`)
  - [x] `DatabaseManager` — open/close SQLite, connection lifecycle
  - [ ] `SchemaManager` — table creation, migrations, version tracking
  - [ ] `QueryHelper` — type-safe query builder, bind parameters
  - [ ] Schema for: songs cache, playlists, user settings, play history, download queue

- [x] **Logger Module** (`src/core/logger/`)
  - [x] `Logger` — spdlog wrapper with named loggers
  - [x] File sink with daily rotation
  - [x] Console sink with color output
  - [ ] Log level configuration via settings
  - [x] Category-based loggers (network, player, api, ui)

- [x] **FileSystem Module** (`src/core/filesystem/`)
  - [x] `AppPaths` — config dir, cache dir, data dir, temp dir (cross-platform)
  - [x] `FileUtils` — safe read/write, atomic save, directory creation
  - [x] `FileWatcher` — watch for external changes to cached files

- [x] **Crypto Module** (`src/core/crypto/`)
  - [x] `Encryptor` — AES-256-GCM encrypt/decrypt for credentials
  - [x] `SecureStorage` — store API tokens, cookies encrypted on disk
  - [x] `CryptoUtils` — SHA-256 for cache keys, request signing

- [x] **App Bootstrap Updates** (`src/app/`)
  - [x] Register all core services in `ServiceLocator`
  - [x] Initialize database, logger, filesystem, crypto on startup
  - [x] Graceful shutdown and resource cleanup

---

## Phase 2: Data Access & Platform APIs

Implement API clients for each music platform and the repository layer.

- [x] **API Common Types** (`src/api/common/`)
  - [x] Shared request/response types for all platforms
  - [ ] Pagination helpers
  - [x] Error handling and retry policies

- [x] **NetEase Cloud Music API** (`src/api/netease/`)
  - [x] `NeteaseClient` implementing `IMusicPlatformPlugin`
  - [x] Login (phone, email, captcha)
  - [x] Search (songs, playlists, albums, artists)
  - [x] Song detail, playback URL, lyrics
  - [x] Playlist detail, user playlists
  - [x] Recommended playlists, high-quality playlists

- [ ] **Bilibili API** (`src/api/bilibili/`)
  - [ ] `BilibiliClient` implementing `IMusicPlatformPlugin`
  - [ ] Login (cookie-based)
  - [ ] Search (audio, video)
  - [ ] Audio extraction from video
  - [ ] Playlist/collection support

- [ ] **YouTube Music API** (`src/api/youtube/`)
  - [ ] `YouTubeMusicClient` implementing `IMusicPlatformPlugin`
  - [ ] OAuth2 authentication
  - [ ] Search (songs, albums, playlists, artists)
  - [ ] Playback URL extraction
  - [ ] Lyrics fetching

- [ ] **QQ Music API** (`src/api/qqmusic/`)
  - [ ] `QQMusicClient` implementing `IMusicPlatformPlugin`
  - [ ] Login (QR code, cookie)
  - [ ] Search, song detail, playback URL
  - [ ] Lyrics, playlist support

- [x] **Repository Layer** (`src/repo/`)
  - [x] `ISongRepository` / `SongRepository` — song CRUD on songs_cache
  - [x] `IPlaylistRepository` / `PlaylistRepository` — playlist CRUD + song membership
  - [x] `IPlayerStateRepository` / `PlayerStateRepository` — singleton player state
  - [x] `ISettingsRepository` / `SettingsRepository` — key-value settings
  - [x] `IPlayHistoryRepository` / `PlayHistoryRepository` — play event tracking

---

## Phase 3: Playback Engine

Integrate audio playback with the platform APIs.

- [x] **Player Module** (`src/player/`)
  - [x] `IPlayerBackend` — abstract interface for audio backends
  - [x] `QtMultimediaBackend` — Qt Multimedia implementation (QMediaPlayer + QAudioOutput)
  - [ ] `MpvBackend` — libmpv implementation (optional, for format support)
  - [x] `PlaybackController` — play/pause/stop/next/prev/seek with URL resolution, pre-resolve, persistence
  - [x] `BackendFactory` — backend selection with auto-detect (qt/mpv/auto)
  - [ ] Gapless playback support
  - [ ] Audio buffer management

- [x] **Playback Queue** (`src/player/`)
  - [x] `PlayQueue` — ordered list of songs to play
  - [x] Repeat modes: off, one, all
  - [x] Shuffle mode with Fisher-Yates
  - [x] Queue manipulation: add, remove, reorder, move

---

## Phase 4: ViewModels

Business logic layer connecting data access to the UI. No dedicated service layer — ViewModels access repositories and API clients directly, following the Android QeriPlayer pattern. `PlaybackController` (Phase 3) handles playback orchestration as a de facto service.

- [x] **ViewModel Layer** (`src/viewmodel/`)
  - [x] `MainViewModel` — app-wide state, navigation, service coordination
  - [x] `PlayerViewModel` — current song, progress, play/pause state, volume
  - [x] `SearchViewModel` — query, results, filters, platform selection
  - [x] `PlaylistViewModel` — playlist list, detail view, editing
  - [x] `LocalPlaylistDetailViewModel` — local playlist detail view
  - [x] `NeteasePlaylistDetailViewModel` — NetEase playlist/album detail view
  - [x] `SettingsViewModel` — settings read/write, theme selection
  - [x] `ViewModelError` — structured error type (Q_GADGET)
  - [x] `SongListModel` — QAbstractListModel for QML binding
  - [x] All ViewModels use `Q_PROPERTY` + signals, no UI dependencies

---

## Phase 5: User Interface

Build the Qt Widgets UI shell and all views.

- [ ] **Main Layout** (`src/ui/`)
  - [ ] Sidebar navigation (playlists, search, settings)
  - [ ] Main content area with stacked views
  - [ ] Bottom player bar (now playing, controls, progress)
  - [ ] Menu bar (File, Edit, View, Help)
  - [ ] Toolbar (search, play controls)

- [ ] **Player Controls** (`src/ui/widgets/`)
  - [ ] `PlayerBar` — compact bottom bar with song info + controls
  - [ ] `ProgressBar` — seekable playback position
  - [ ] `VolumeSlider` — volume control with mute toggle
  - [ ] `NowPlayingView` — full-screen/expanded player view

- [ ] **Search View** (`src/ui/views/`)
  - [ ] Search input with platform filter dropdown
  - [ ] Results list (songs, playlists, albums tabs)
  - [ ] Inline play / add-to-playlist actions

- [ ] **Playlist View** (`src/ui/views/`)
  - [ ] Playlist list (sidebar)
  - [ ] Playlist detail (song table with columns)
  - [ ] Drag-and-drop reordering
  - [ ] Create / edit / delete playlist dialogs

- [ ] **Settings View** (`src/ui/views/`)
  - [ ] General settings (language, startup behavior)
  - [ ] Audio settings (output device, crossfade)
  - [ ] Account settings (platform logins)
  - [ ] Storage settings (cache size, download path)
  - [ ] About page

- [ ] **Dialogs** (`src/ui/dialogs/`)
  - [ ] Login dialog (per-platform)
  - [ ] Lyrics viewer dialog
  - [ ] Equalizer dialog
  - [ ] About dialog

---

## Phase 6: Downloads & Sync

Offline playback and cloud synchronization.

- [ ] **Download Manager** (`src/download/`)
  - [ ] `DownloadQueue` — queued download jobs with priority
  - [ ] `DownloadWorker` — background download with progress reporting
  - [ ] Audio file storage organized by platform/song
  - [ ] Resume interrupted downloads
  - [ ] Download quality selection

- [ ] **Cloud Sync** (`src/sync/`)
  - [ ] `ISyncProvider` — abstract sync interface
  - [ ] `GitHubGistSync` — sync settings/playlists via GitHub Gist
  - [ ] `WebDAVSync` — sync via any WebDAV server
  - [ ] Conflict resolution (last-write-wins or merge)
  - [ ] Manual and auto-sync triggers

---

## Phase 7: Polish & Extras

Final touches and extended features.

- [ ] **Theme System** (`src/ui/themes/`)
  - [ ] Light / dark theme toggle
  - [ ] Custom accent colors
  - [ ] Theme persistence in settings

- [ ] **Keyboard Shortcuts**
  - [ ] Global media keys (play/pause, next, prev)
  - [ ] In-app shortcuts (Ctrl+F search, Ctrl+N new playlist)
  - [ ] Customizable key bindings

- [ ] **System Integration**
  - [ ] System tray icon with mini player
  - [ ] MPRIS2 support (Linux)
  - [ ] macOS Dock integration
  - [ ] Windows taskbar thumbnail buttons

- [ ] **Performance & Reliability**
  - [ ] Lazy-load playlist songs
  - [ ] Image caching with size limits
  - [ ] Network error retry with exponential backoff
  - [ ] Crash recovery (save playback state)

- [x] **Testing**
  - [x] Unit tests for domain models
  - [x] Unit tests for core modules (database, crypto, logger)
  - [x] Integration tests for API clients (with recorded responses)
  - [ ] ViewModel tests with mock services

---

## Notes

- Phases 1–4 are backend-heavy and can proceed without UI work.
- Phase 5 can begin in parallel once ViewModels (Phase 4) are stable.
- Each phase should be a separate OpenSpec change for traceability.
- Reference: [Architecture Design](architecture/index.md), [Module Design](modules/index.md)
