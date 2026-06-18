## Context

QeriPlayer Qt has completed Phases 1–3: core infrastructure, API clients (NeteaseClient), repository layer (5 repos), and the playback engine (PlaybackController + PlayQueue + QtMultimediaBackend). The next step is the ViewModel layer — the business logic bridge between data/player layers and the future QML UI.

The Android QeriPlayer uses a similar architecture: ViewModels in `ui/viewmodel/` access repos and API clients directly, with no dedicated service layer. The Qt port follows this pattern, adding `Q_PROPERTY`/`Q_INVOKABLE` for QML binding.

Existing interfaces that ViewModels depend on:
- `IMusicPlatformPlugin` — search, song detail, URL resolution, lyrics
- `ISongRepository` / `IPlaylistRepository` / `IPlayerStateRepository` / `ISettingsRepository` / `IPlayHistoryRepository`
- `PlaybackController` — playback orchestration (play, pause, seek, queue, volume)
- `NeteaseClient` — extends IMusicPlatformPlugin with login, playlists, albums, user ops
- `ApiError` / `ApiResult<T>` — existing error handling for API layer

## Goals / Non-Goals

**Goals:**
- Provide a QML-friendly business logic layer with `Q_PROPERTY` + `Q_INVOKABLE` + signals
- Structured error reporting via `ViewModelError` Q_GADGET
- Signal-based cross-VM communication (no circular dependencies)
- Debounced search with request versioning for race safety
- Cache-first playlist management (local cache, explicit sync from remote)
- Per-platform playlist detail screens (mirroring Android architecture)

**Non-Goals:**
- No UI implementation (Phase 5)
- No Bilibili/YouTube ViewModels (only NetEase for now)
- No ViewModel pooling (detail VMs created/destroyed per navigation)
- No multi-platform search aggregation (user picks one source)

## Decisions

### D1: Signal-based cross-VM communication

**Choice:** Child VMs emit `requestPlay(Song)` signals; `MainViewModel` wires them to `PlayerViewModel::play()`.

**Why:** Avoids circular dependencies between VMs. `SearchViewModel` doesn't know about `PlayerViewModel`. `MainViewModel` acts as the composition root for signal wiring, mirroring how `QeriPlayerApplication` wires core services.

**Alternative considered:** Direct `PlayerViewModel*` injection into child VMs. Rejected — creates coupling and makes testing harder (need to mock PlayerViewModel in every child VM test).

### D2: Debounce in ViewModel via QTimer

**Choice:** `SearchViewModel` owns a 300ms single-shot `QTimer`. `setQuery()` restarts the timer; timeout triggers `search()`.

**Why:** Keeps the View layer dumb. QML just binds to `query` property — no need for `LaunchedEffect` or timer logic in the UI. Matches the principle that ViewModels own business logic.

**Alternative considered:** Debounce in QML. Works but scatters search logic across VM and View.

### D3: Request versioning for search race safety

**Choice:** `SearchViewModel` maintains a `quint64 m_searchRequestVersion`. Every `search()` call increments it. When the async result arrives, it checks if the version still matches — stale results are discarded.

**Why:** Prevents fast typing from showing results from a previous query. Simpler than cancellation tokens. Matches Android's `ExploreViewModel.searchRequestVersion` pattern.

### D4: Per-platform playlist detail VMs

**Choice:** Separate `LocalPlaylistDetailViewModel` and `NeteasePlaylistDetailViewModel` classes. `PlaylistViewModel` only lists playlists; detail VMs handle individual playlist views.

**Why:** Mirrors Android architecture (`LocalPlaylistDetailViewModel`, `NeteaseCollectionDetailViewModel`). Each detail VM has different dependencies (local needs `IPlaylistRepository`; NetEase needs `NeteaseClient`). Keeps responsibilities clean.

**Alternative considered:** Single `PlaylistDetailViewModel` with a type switch. Rejected — too much conditional logic, harder to test.

### D5: ViewModelError wraps ApiError + adds categories

**Choice:** New `ViewModelError` Q_GADGET with `ErrorType` enum (Network, Auth, RateLimit, NotFound, Api, Database, Validation, Unknown). Factory method `fromApiError()` maps `ApiError` classification to `ErrorType`.

**Why:** `ApiError` only covers API failures. ViewModels also encounter database errors, validation errors, etc. A unified error type simplifies QML error handling — one pattern for all error display.

### D6: SongListModel as QAbstractListModel

**Choice:** `SongListModel` subclasses `QAbstractListModel` with roles (Id, Name, Artist, Album, DurationMs, CoverUrl, Platform, IsPlaying). Owned by the ViewModel that creates it (parented QObject).

**Why:** QML `ListView` requires a `QAbstractItemModel`. `QVariantList` works but loses type safety and performance for large lists. Roles enable clean QML delegate bindings.

### D7: Navigation enum with Q_ENUM

**Choice:** `MainViewModel::View` enum (`Home`, `Search`, `Library`, `LocalPlaylist`, `NeteasePlaylist`, `Settings`) exposed via `Q_PROPERTY`.

**Why:** Type safety, QML autocomplete, compile-time exhaustiveness checks. `Q_ENUM` makes it introspectable from QML.

### D8: Detail VMs deleted on navigation

**Choice:** `MainViewModel` creates detail VMs with `new` when navigating to a detail screen, deletes them when navigating away.

**Why:** Simple lifecycle, no stale state. Re-entering a playlist re-fetches from cache/API. Pooling is a future optimization if navigation proves slow.

### D9: saveToLocal() duplicates songs

**Choice:** `NeteasePlaylistDetailViewModel::saveToLocal()` creates a local playlist and copies all songs into `songs_cache` via `ISongRepository::saveBatch()`.

**Why:** Enables offline playback. Simple model — no need for "remote playlist reference" indirection. Songs are self-contained in the local DB.

## Risks / Trade-offs

**[Risk] Large playlists (>1000 tracks) slow to fetch and cache.**
→ Mitigation: Batch-fetch in pages of 300 (matching Android pattern). Show loading progress. Cache incrementally.

**[Risk] QCoro::Task lifetime — fire-and-forget from Q_INVOKABLE can outlive the ViewModel.**
→ Mitigation: Store running tasks as members. Cancel on destruction. Use `QPointer` self-check after co_await.

**[Risk] Signal wiring in MainViewModel grows complex as more VMs are added.**
→ Mitigation: Keep wiring in a single `connectSignals()` method. Each connection is one line. Document the pattern.

**[Risk] SongListModel role lookups could be slow with 10K+ songs.**
→ Mitigation: QVector is contiguous memory; role lookup is O(1). If needed, virtualize with fetch-on-scroll later.
