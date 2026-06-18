## Why

Phase 4 ViewModels expose async operations as `void` Q_INVOKABLE methods, forcing QML to use awkward `Connections` + signal handlers to detect completion. QCoro's `QmlTask` type allows `Q_INVOKABLE` methods to return `QCoro::Task<T>` directly to QML, enabling clean `.then()` callbacks and `.await()` property binding. This is the bridge that makes the VM layer truly QML-native — prerequisite for Phase 5 (UI).

## What Changes

- **Enable `QCoro::Qml` in CMake** — add `find_package(QCoro6 COMPONENTS Qml)` and link `QCoro::Qml` to the viewmodel library.
- **Convert void async VM methods to return `QCoro::QmlTask`** — methods that perform async work (API calls, DB queries) and currently return `void` will return `QCoro::QmlTask` instead. QML can then use `.then(callback)` or `.await().value`.
- **Keep `Q_PROPERTY` for continuous state** — properties like `isPlaying`, `currentSong`, `query`, `results`, `isLoading` remain as-is. `QmlTask` is for one-shot operations only.

**Methods to convert (by ViewModel):**

| ViewModel | Method | Current | New Return |
|-----------|--------|---------|------------|
| `SearchViewModel` | `search()` | `void` | `QCoro::QmlTask` |
| `SearchViewModel` | `loadMore()` | `void` | `QCoro::QmlTask` |
| `PlaylistViewModel` | `loadLocalPlaylists()` | `void` | `QCoro::QmlTask` |
| `PlaylistViewModel` | `loadNeteasePlaylists()` | `void` | `QCoro::QmlTask` |
| `PlaylistViewModel` | `loadNeteaseAlbums()` | `void` | `QCoro::QmlTask` |
| `PlaylistViewModel` | `createLocalPlaylist(name)` | `void` | `QCoro::QmlTask` |
| `PlaylistViewModel` | `deleteLocalPlaylist(id)` | `void` | `QCoro::QmlTask` |
| `PlaylistViewModel` | `renameLocalPlaylist(id, name)` | `void` | `QCoro::QmlTask` |
| `LocalPlaylistDetailViewModel` | `loadPlaylist(id)` | `void` | `QCoro::QmlTask` |
| `LocalPlaylistDetailViewModel` | `addSong(id)` | `void` | `QCoro::QmlTask` |
| `LocalPlaylistDetailViewModel` | `removeSong(id)` | `void` | `QCoro::QmlTask` |
| `LocalPlaylistDetailViewModel` | `reorderSongs(ids)` | `void` | `QCoro::QmlTask` |
| `LocalPlaylistDetailViewModel` | `rename(name)` | `void` | `QCoro::QmlTask` |
| `LocalPlaylistDetailViewModel` | `deletePlaylist()` | `void` | `QCoro::QmlTask` |
| `NeteasePlaylistDetailViewModel` | `loadPlaylist(id)` | `void` | `QCoro::QmlTask` |
| `NeteasePlaylistDetailViewModel` | `loadAlbum(id)` | `void` | `QCoro::QmlTask` |
| `NeteasePlaylistDetailViewModel` | `retry()` | `void` | `QCoro::QmlTask` |
| `NeteasePlaylistDetailViewModel` | `saveToLocal()` | `void` | `QCoro::QmlTask` |
| `MainViewModel` | `initialize()` | `void` | `QCoro::QmlTask` |

**Methods that stay `void` (fire-and-forget, no async):**

| ViewModel | Method | Reason |
|-----------|--------|--------|
| `PlayerViewModel` | `pause()`, `resume()`, `stop()`, `seek()`, `next()`, `prev()` | Instant, no await |
| `PlayerViewModel` | `toggleMute()`, `cycleRepeatMode()`, `toggleShuffle()` | Instant |
| `PlayerViewModel` | `loadQueueAndPlay()` | Instant setup |
| `SearchViewModel` | `clearResults()`, `clearError()` | Instant |
| `SettingsViewModel` | `setTheme()`, `setAudioQuality()`, `setDownloadPath()` | Sync write |
| `SettingsViewModel` | `clearPlayHistory()`, `clearError()` | Sync or fire-and-forget |
| `PlaylistViewModel` | `clearError()` | Instant |
| `MainViewModel` | `navigateTo()`, `openLocalPlaylist()`, `openNeteasePlaylist()`, `openNeteaseAlbum()` | Navigation, no await |

**Already `QCoro::Task<T>` (no change needed):**

| ViewModel | Method |
|-----------|--------|
| `PlayerViewModel` | `play(Song)` — already `QCoro::Task<void>` |
| `SettingsViewModel` | `loginNetease()` — already `QCoro::Task<void>` |
| `SettingsViewModel` | `logoutNetease()` — already `QCoro::Task<void>` |

## Non-goals

- **No QML UI implementation** — this change only modifies the VM layer and build system. QML views are Phase 5.
- **No new ViewModels** — only modifying return types of existing methods.
- **No Q_PROPERTY changes** — continuous state binding is unchanged.
- **No QCoro::Qml context property registration** — that's Phase 5 (PR 1).
- **No breaking signal removal** — completion signals (e.g., `searchCompleted`) remain for C++ consumers. QML can use either `.then()` or signals.

## Capabilities

### New Capabilities

- `vm-qmltask`: Pattern for exposing `QCoro::QmlTask` return types from ViewModel Q_INVOKABLE methods, enabling QML `.then()` and `.await()` usage.

### Modified Capabilities

- `vm-search`: `search()` and `loadMore()` return `QCoro::QmlTask` instead of `void`.
- `vm-playlist`: `loadLocalPlaylists()`, `loadNeteasePlaylists()`, `loadNeteaseAlbums()`, `createLocalPlaylist()`, `deleteLocalPlaylist()`, `renameLocalPlaylist()` return `QCoro::QmlTask`.
- `vm-local-playlist-detail`: `loadPlaylist()`, `addSong()`, `removeSong()`, `reorderSongs()`, `rename()`, `deletePlaylist()` return `QCoro::QmlTask`.
- `vm-netease-playlist-detail`: `loadPlaylist()`, `loadAlbum()`, `retry()`, `saveToLocal()` return `QCoro::QmlTask`.
- `vm-main`: `initialize()` returns `QCoro::QmlTask`.

## Impact

- **Affected layers**: `viewmodel/` (return type changes), `CMakeLists.txt` (QCoro::Qml dependency)
- **Dependencies**: `QCoro::Qml` library (already available in `lib/qcoro/`, currently disabled)
- **Files modified**: ~9 VM headers, ~9 VM source files, CMakeLists.txt, ~9 test files
- **Build**: CMakeLists.txt — enable `QCORO_WITH_QML ON`, add `QCoro::Qml` to viewmodel library link
- **Test updates**: Tests calling void methods need to handle `QCoro::QmlTask` return (use `QCoro::waitFor()` or adjust coroutine test patterns)
