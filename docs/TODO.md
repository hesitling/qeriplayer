# NeriPlayer Qt — Long-Term Implementation Roadmap

This document tracks the full implementation plan for NeriPlayer Qt, organized by phase. Each phase builds on the previous one. Check off items as they are completed.

---

## Phase 1: Foundation Layer

Build the core infrastructure that all other modules depend on.

- [ ] **Domain Models** (`src/domain/`)
  - [ ] `Song` — id, title, artist, album, duration, coverUrl, platform, playbackUrl
  - [ ] `Album` — id, title, artist, coverUrl, song list
  - [ ] `Artist` — id, name, avatarUrl, description
  - [ ] `Playlist` — id, name, description, coverUrl, song count, owner
  - [ ] `Lyrics` — raw text, timed lines with timestamps
  - [ ] `SearchResult` — list of songs, playlists, albums, artists with pagination
  - [ ] Enums — `MusicPlatform`, `SearchType`, `PlaybackState`, `RepeatMode`, `AudioQuality`
  - [ ] Value types — `PageInfo`, `Duration`, `CoverImage`

- [ ] **Database Module** (`src/core/database/`)
  - [ ] `DatabaseManager` — open/close SQLite, connection lifecycle
  - [ ] `SchemaManager` — table creation, migrations, version tracking
  - [ ] `QueryHelper` — type-safe query builder, bind parameters
  - [ ] Schema for: songs cache, playlists, user settings, play history, download queue

- [ ] **Logger Module** (`src/core/logger/`)
  - [ ] `Logger` — spdlog wrapper with named loggers
  - [ ] File sink with daily rotation
  - [ ] Console sink with color output
  - [ ] Log level configuration via settings
  - [ ] Category-based loggers (network, player, api, ui)

- [ ] **FileSystem Module** (`src/core/filesystem/`)
  - [ ] `AppPaths` — config dir, cache dir, data dir, temp dir (cross-platform)
  - [ ] `FileUtils` — safe read/write, atomic save, directory creation
  - [ ] `FileWatcher` — watch for external changes to cached files

- [ ] **Crypto Module** (`src/core/crypto/`)
  - [ ] `Encryptor` — AES-256-GCM encrypt/decrypt for credentials
  - [ ] `SecureStorage` — store API tokens, cookies encrypted on disk
  - [ ] `HashUtils` — SHA-256 for cache keys, request signing

- [ ] **App Bootstrap Updates** (`src/app/`)
  - [ ] Register all core services in `ServiceLocator`
  - [ ] Initialize database, logger, filesystem, crypto on startup
  - [ ] Graceful shutdown and resource cleanup

---

## Phase 2: Data Access & Platform APIs

Implement API clients for each music platform and the repository layer.

- [ ] **API Common Types** (`src/api/common/`)
  - [ ] Shared request/response types for all platforms
  - [ ] Pagination helpers
  - [ ] Error handling and retry policies

- [ ] **NetEase Cloud Music API** (`src/api/netease/`)
  - [ ] `NeteaseClient` implementing `IMusicPlatformPlugin`
  - [ ] Login (phone, email, QR code)
  - [ ] Search (songs, playlists, albums, artists)
  - [ ] Song detail, playback URL, lyrics
  - [ ] Playlist detail, user playlists
  - [ ] Recommended playlists, daily recommendations

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

- [ ] **Repository Layer** (`src/repository/`)
  - [ ] `IMusicRepository` — abstract interface for song/album/artist data
  - [ ] `CachedMusicRepository` — API calls with SQLite cache
  - [ ] `PlaylistRepository` — local playlist CRUD
  - [ ] `SettingsRepository` — app settings persistence
  - [ ] `HistoryRepository` — play history tracking

---

## Phase 3: Playback Engine

Integrate audio playback with the platform APIs.

- [ ] **Player Module** (`src/player/`)
  - [ ] `PlayerBackend` — abstract interface for audio backends
  - [ ] `QtMultimediaBackend` — Qt Multimedia implementation
  - [ ] `VlcBackend` — libVLC implementation (optional, for format support)
  - [ ] `PlaybackController` — play/pause/stop/next/prev/seek
  - [ ] `AudioOutput` — volume control, device selection
  - [ ] Gapless playback support
  - [ ] Audio buffer management

- [ ] **Playback Queue** (`src/player/`)
  - [ ] `PlayQueue` — ordered list of songs to play
  - [ ] Repeat modes: off, one, all
  - [ ] Shuffle mode with Fisher-Yates
  - [ ] Queue manipulation: add, remove, reorder

---

## Phase 4: Services & ViewModels

Business logic layer connecting data access to the UI.

- [ ] **Service Layer** (`src/service/`)
  - [ ] `PlayerService` — wraps PlaybackController, manages current song, exposes state
  - [ ] `SearchService` — aggregates search across all registered platforms
  - [ ] `PlaylistService` — playlist CRUD, add/remove songs
  - [ ] `AuthService` — per-platform login state, token refresh
  - [ ] `DownloadService` — queue downloads, track progress (see Phase 6)

- [ ] **ViewModel Layer** (`src/viewmodel/`)
  - [ ] `MainViewModel` — app-wide state, navigation, service coordination
  - [ ] `PlayerViewModel` — current song, progress, play/pause state, volume
  - [ ] `SearchViewModel` — query, results, filters, platform selection
  - [ ] `PlaylistViewModel` — playlist list, detail view, editing
  - [ ] `SettingsViewModel` — settings read/write, theme selection
  - [ ] All ViewModels use `Q_PROPERTY` + signals, no UI dependencies

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

- [ ] **Testing**
  - [ ] Unit tests for domain models
  - [ ] Unit tests for core modules (database, crypto, logger)
  - [ ] Integration tests for API clients (with recorded responses)
  - [ ] ViewModel tests with mock services

---

## Notes

- Phases 1–4 are backend-heavy and can proceed without UI work.
- Phase 5 can begin in parallel once ViewModels (Phase 4) are stable.
- Each phase should be a separate OpenSpec change for traceability.
- Reference: [Architecture Design](architecture/index.md), [Module Design](modules/index.md)
