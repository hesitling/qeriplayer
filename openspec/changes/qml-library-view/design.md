## Context

QeriPlayer Qt's Phase 5 QML work already provides the application shell (`main.qml`), PlayerBar, Toast, and SearchView. `PlaylistViewModel` exposes three overview lists (`localPlaylists`, `neteasePlaylists`, `neteaseAlbums`) plus create/rename/delete operations, while `MainViewModel` already supports detail navigation via `openLocalPlaylist()`, `openNeteasePlaylist()`, and `openNeteaseAlbum()`.

The current gap is in the UI contract, not the data pipeline:
- `main.qml` still renders a placeholder for `Library` and does not render `LocalPlaylist` / `NeteasePlaylist` detail routes.
- `PlaylistSummary` and `AlbumSummary` are `Q_DECLARE_METATYPE` value types, but unlike `Song` they are not QML-readable via property syntax.
- QML can react to C++ signals but cannot emit `PlaylistViewModel` selection signals or easily construct `PlaylistSummary` / `AlbumSummary` values to pass back into C++.

PR 4 therefore needs to complete the library vertical slice across `src/domain/`, `src/viewmodel/`, `src/qml/`, and the existing `MainViewModel` navigation surface.

## Goals / Non-Goals

**Goals:**
- Replace the library placeholder with a functional `LibraryView.qml`.
- Load NetEase playlists and albums lazily when their tabs are first activated.
- Reuse existing `LocalPlaylistDetailViewModel` and `NeteasePlaylistDetailViewModel` in QML detail pages.
- Support playlist management flows in QML: create, rename, delete, save-to-local.
- Keep QML ↔ ViewModel bindings simple and type-safe enough for existing C++ contracts.
- Preserve current interfaces (`IPlaylistRepository`, `ISongRepository`, `NeteaseClient`) without adding a new service layer.

**Non-Goals:**
- Reworking repository or API contracts.
- Parallel per-tab loading state in `PlaylistViewModel`.
- Reorder/add-song UI for local playlists.
- New remote providers beyond `NeteaseClient`.

## Decisions

### D1: Tabbed overview with one-time lazy remote loads

**Decision:** `LibraryView.qml` uses tabs for Local, NetEase Playlists, and NetEase Albums. Local content is available immediately from `MainViewModel::initialize()`. Remote tabs trigger `playlistVm.loadNeteasePlaylists()` or `playlistVm.loadNeteaseAlbums()` only on first activation.

**Why:** `PlaylistViewModel` currently exposes one shared `isLoading`/`error` state. Tabs fit that model well and avoid a larger refactor for independent per-section loading states.

**Alternatives considered:**
- Load all remote sections on Library entry — rejected because the user explicitly wants lazy loading.
- Multiple visible sections with independent spinners — rejected because `PlaylistViewModel` does not expose per-section loading/error state.

### D2: Make summary models QML-readable with `Q_GADGET`

**Decision:** Add `Q_GADGET` and `Q_PROPERTY(READ ... CONSTANT)` support to `PlaylistSummary` and `AlbumSummary`, mirroring the pattern already used for `Song`.

**Why:** Library delegates need direct bindings like `modelData.name`, `modelData.coverUrl`, and `modelData.trackCount` / `modelData.size`. Relying on opaque `QVariant` payloads would make QML brittle or force lossy `QVariantMap` conversion inside `PlaylistViewModel`.

**Alternatives considered:**
- Convert summaries to `QVariantMap` in `PlaylistViewModel` — rejected because it duplicates field mapping and weakens type safety.
- Introduce a `QAbstractListModel` for playlists — rejected for PR 4 because it is larger than needed for a small card list UI.

### D3: Add QML helper methods to `PlaylistViewModel`

**Decision:** Add `Q_INVOKABLE` helpers such as `openLocalPlaylist(int index)`, `openNeteasePlaylist(int index)`, and `openNeteaseAlbum(int index)` that validate the index and emit the existing selection signals.

**Why:** QML cannot emit C++ signals, and passing full `PlaylistSummary` / `AlbumSummary` arguments back into C++ from a delegate is awkward. Index-based helpers mirror the `SearchViewModel::playSong(int)` pattern already adopted for PR 3.

**Alternatives considered:**
- Call `mainVm.openLocalPlaylist(id)` directly from QML — works for local playlists, but bypasses the `PlaylistViewModel` selection contract and forces the UI to know more about navigation than necessary.
- Expose raw IDs only — rejected because NetEase selections already use summary objects, and keeping the signal contract intact is cleaner.

### D4: Render detail routes directly in `main.qml`

**Decision:** Extend the `mainVm.currentView` switch to render `LibraryView`, `LocalPlaylistDetailView`, and `NeteasePlaylistDetailView` directly from the existing `StackView` replacement model.

**Why:** The shell already uses `contentStack.replace(page)` for top-level routing. PR 4 can complete that pattern without introducing a nested navigation system.

**Alternatives considered:**
- Use a nested `StackView` inside `LibraryView.qml` — rejected because detail ViewModel lifecycle is already owned by `MainViewModel`.
- Use `StackView.push/pop` for full history — rejected because current shell behavior is replace-oriented and explicit back-to-library navigation is sufficient.

### D5: Back navigation is explicit and `MainViewModel`-driven

**Decision:** Detail pages expose a Back button that calls `mainVm.navigateTo(mainVm.Library)` semantics through the existing enum value, returning to the overview and letting `MainViewModel` delete active detail VMs.

**Why:** `MainViewModel::navigateTo()` already owns detail VM cleanup. Using it avoids stale detail VM instances and keeps lifecycle rules in one place.

**Alternatives considered:**
- `StackView.pop()` — rejected because the shell is not currently history-based, and it would sidestep detail cleanup rules.

### D6: Dialogs stay local to QML pages

**Decision:** Use lightweight QML `Dialog` components in the relevant pages for create, rename, and delete confirmation flows, backed by existing `PlaylistViewModel` and `LocalPlaylistDetailViewModel` methods.

**Why:** These flows are UI concerns over existing async methods (`createLocalPlaylist`, `rename`, `deletePlaylist`, `saveToLocal`). Dedicated C++ dialog controllers are unnecessary.

**Alternatives considered:**
- Separate reusable dialog files for all flows — possible, but likely overkill for PR 4's size.

### D7: Error display stays centralized through Toast

**Decision:** `main.qml` adds `Connections` for `playlistVm`, `mainVm.localPlaylistDetail`, and `mainVm.neteasePlaylistDetail` as needed, calling `toast.show(error.message)` when `hasError` becomes true.

**Why:** This matches the existing player and search error pattern and keeps pages focused on content rather than bespoke alert rendering.

## Risks / Trade-offs

- **[Risk] `Q_GADGET` summary types add moc/build wiring** → Mirror the existing `Song` pattern and cover it with a focused domain test.
- **[Risk] Shared `PlaylistViewModel::isLoading` can show generic loading across tabs** → Only trigger one remote fetch at a time and scope UI messaging to the active tab.
- **[Risk] Index-based open helpers can race stale list state** → Guard invalid indices and do nothing on out-of-bounds access.
- **[Risk] Detail view error connections may outlive replaced VMs** → Bind via `mainVm.localPlaylistDetail` / `mainVm.neteasePlaylistDetail` and rely on QObject lifetime updates when `MainViewModel` emits changed signals.
- **[Trade-off] Tabs avoid a richer grid/section layout** → Acceptable for PR 4 because they align with lazy loading and current VM state.
