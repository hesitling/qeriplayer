## Why

Phase 4 of the implementation roadmap: the ViewModel layer bridges the data/player layers (repos, APIs, PlaybackController) to the future QML UI. Without ViewModels, there's no structured way for the UI to observe playback state, trigger searches, manage playlists, or configure settings. This is the last backend-heavy phase before UI work can begin (Phase 5).

## What Changes

- New `src/viewmodel/` module with 9 items: `MainViewModel`, `PlayerViewModel`, `SearchViewModel`, `PlaylistViewModel`, `LocalPlaylistDetailViewModel`, `NeteasePlaylistDetailViewModel`, `SettingsViewModel`, plus shared types `ViewModelError` and `SongListModel`.
- All ViewModels expose state via `Q_PROPERTY` + signals, methods via `Q_INVOKABLE` — QML-friendly contract.
- `ViewModelError` Q_GADGET provides structured error reporting across all ViewModels (wraps `ApiError` + adds database/validation categories).
- `SongListModel` QAbstractListModel adapts `QVector<Song>` for QML `ListView` binding.
- Cross-ViewModel communication uses signal wiring (no circular deps) — `MainViewModel` connects child VM signals.
- `SearchViewModel` dispatches to `IMusicPlatformPlugin` per selected platform, with debounce timer and request versioning for race safety.
- Playlist architecture mirrors Android: `PlaylistViewModel` lists playlists from all sources; detail screens (`LocalPlaylistDetailViewModel`, `NeteasePlaylistDetailViewModel`) handle individual playlist views.
- `NeteasePlaylistDetailViewModel` fetches remote playlists/albums, batch-fetches large track lists, and caches songs locally.
- `SettingsViewModel` manages settings persistence and platform auth (NetEase login/logout).

## Non-goals

- **No UI implementation** — this phase only creates ViewModels. QML screens are Phase 5.
- **No Bilibili/YouTube ViewModels** — only NetEase is implemented now. Other platform detail VMs are added when those API clients exist.
- **No ViewModel pooling** — detail VMs are created on navigation and deleted on exit. Pooling is a future optimization.
- **No multi-platform search aggregation** — user selects one search source at a time (matching Android pattern).

## Capabilities

### New Capabilities

- `viewmodel-error`: Structured error Q_GADGET for ViewModel layer (ErrorType enum, factory from ApiError, canRetry classification).
- `song-list-model`: QAbstractListModel adapting QVector<Song> for QML ListView binding with role-based access and playing-index highlighting.
- `player-viewmodel`: Wraps PlaybackController into Q_PROPERTY-based UI contract (current song, state, progress, volume, shuffle, repeat, queue).
- `search-viewmodel`: Search with platform dispatch, 300ms debounce, request versioning, pagination, result caching.
- `playlist-viewmodel`: Library overview listing local/NetEase playlists and albums with CRUD for local playlists.
- `local-playlist-detail-viewmodel`: Detail view for local playlists (load, add/remove/reorder songs, rename/delete, play integration).
- `netease-playlist-detail-viewmodel`: Detail view for NetEase playlists/albums (fetch from API, batch-fetch large playlists, cache locally, saveToLocal).
- `settings-viewmodel`: Settings read/write via ISettingsRepository, NetEase auth management, play history clearing.
- `main-viewmodel`: App coordinator owning child VMs, wiring cross-VM signals, managing navigation state via enum.

### Modified Capabilities

None. This phase adds new code only; existing module interfaces are unchanged.

## Impact

- **Affected layers**: `viewmodel/` (new), `app/` (minor: register VMs in ServiceLocator or create in MainWindow).
- **Dependencies**: ViewModels depend on `repo/`, `api/`, `player/`, `domain/` — all already implemented.
- **New files**: ~18 source files (9 header + 9 source) in `src/viewmodel/`, ~9 test files in `tests/viewmodel/`.
- **Build**: CMakeLists.txt update to add viewmodel library target and link to existing targets.
