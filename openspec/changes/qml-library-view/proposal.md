## Why

Phase 5's QML migration has a shell, player bar, and search page, but the Library sidebar entry still leads to a placeholder. The ViewModel layer already exposes library overview and playlist detail data, so PR 4 should turn that existing backend surface into a usable library experience with lazy remote loading, detail navigation, and playlist management flows.

## What Changes

- **New `LibraryView.qml`** — library overview page with tabs for Local, NetEase Playlists, and NetEase Albums; lazy loading for remote tabs; loading, empty, and retry states; and playlist card browsing.
- **New playlist detail pages** — `LocalPlaylistDetailView.qml` and `NeteasePlaylistDetailView.qml` with explicit back navigation, header actions, and song lists reusing `SongDelegate.qml`.
- **Playlist management dialogs** — create local playlist from the library view, rename/delete dialogs for local playlist detail, and save-to-local action for NetEase detail.
- **VM additions for QML** — `PlaylistViewModel` gains index-based open helpers so QML can trigger existing selection signals without constructing summary objects.
- **Summary models become QML-readable** — `PlaylistSummary` and `AlbumSummary` gain QML property access so delegates can bind to name, cover, and counts directly.
- **Shell wiring** — `main.qml` replaces the Library placeholder, routes Library/LocalPlaylist/NeteasePlaylist views, and wires library/detail errors to Toast.

## Non-goals

- Drag-and-drop song reordering in local playlists
- Add-song picker or context menus for playlist songs
- Non-NetEase remote library sources (Bilibili, YouTube, QQ Music)
- Queue management, lyrics, or polished animations

## Capabilities

### New Capabilities
- `qml-library-view`: QML library overview page with tabbed sections, lazy remote loading, playlist cards, and create-playlist flow.
- `qml-playlist-detail-view`: QML local and NetEase playlist detail pages with back navigation, header actions, and song list playback.

### Modified Capabilities
- `core-domain-models`: `PlaylistSummary` and `AlbumSummary` expose QML-readable properties for overview delegates.
- `vm-playlist`: Add QML-friendly index-based open helpers that emit the existing playlist selection signals.

## Impact

| Layer | Files |
|-------|-------|
| domain | `PlaylistSummary.h` (+ moc compilation unit if needed) for QML-readable summary types |
| viewmodel | `PlaylistViewModel.h/cpp` for QML open helpers |
| ui (qml) | `LibraryView.qml`, playlist card/delegate component, `LocalPlaylistDetailView.qml`, `NeteasePlaylistDetailView.qml`, `main.qml`, `qml.qrc` |
| tests | domain and viewmodel tests for summary QML access and playlist open helpers |
