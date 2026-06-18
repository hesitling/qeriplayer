## PR 4: Library view (depends on PR 3: qml-search-view)

Delivers the full Library vertical slice for the Phase 5 QML migration: QML-readable playlist summaries, playlist open helpers, the tabbed library overview, detail pages, and shell routing.

## 1. Domain + ViewModel QML bridge

- [x] 1.1 Add `Q_GADGET` + `Q_PROPERTY(READ ... CONSTANT)` support for `PlaylistSummary` and `AlbumSummary` in `src/domain/PlaylistSummary.h`
- [x] 1.2 Add any required moc compilation wiring for summary Q_GADGET support and extend domain tests for QML-readable summary properties
- [x] 1.3 Add `Q_INVOKABLE void openLocalPlaylist(int index)` to `PlaylistViewModel.h`
- [x] 1.4 Add `Q_INVOKABLE void openNeteasePlaylist(int index)` and `Q_INVOKABLE void openNeteaseAlbum(int index)` to `PlaylistViewModel.h`
- [x] 1.5 Implement the new open helpers in `PlaylistViewModel.cpp` with bounds checks and signal emission
- [x] 1.6 Extend `tests/viewmodel/TestPlaylistViewModel.cpp` for valid and invalid index helper behavior

## 2. Library overview QML

Commit group goal: make the Library tab usable with local playlists first, then lazy remote sections.

- [x] 2.1 Create `src/qml/LibraryView.qml` with tabs for Local, NetEase Playlists, and NetEase Albums
- [x] 2.2 Create a reusable playlist/albums card delegate component for overview lists
- [x] 2.3 Bind the Local tab to `playlistVm.localPlaylists` and implement empty-state messaging
- [x] 2.4 Implement create-local-playlist dialog flow in `LibraryView.qml` using `playlistVm.createLocalPlaylist()`
- [x] 2.5 Implement one-time lazy loading for NetEase playlists and albums when their tabs are first activated
- [x] 2.6 Implement loading, error, retry, and empty states for active remote tabs
- [x] 2.7 Wire card activation to the new `PlaylistViewModel` open helpers
- [x] 2.8 Add new overview QML files to `src/qml/qml.qrc`

## 3. Detail pages + shell navigation

Commit group goal: complete end-to-end navigation from library cards into local and NetEase detail views.

- [x] 3.1 Create `src/qml/LocalPlaylistDetailView.qml` bound to `mainVm.localPlaylistDetail`
- [x] 3.2 Create `src/qml/NeteasePlaylistDetailView.qml` bound to `mainVm.neteasePlaylistDetail`
- [x] 3.3 Reuse `SongDelegate.qml` in both detail pages with double-click play and Play All actions
- [x] 3.4 Add explicit Back actions in both detail pages that navigate back to Library through `mainVm`
- [x] 3.5 Add rename and delete dialog flows to `LocalPlaylistDetailView.qml`
- [x] 3.6 Add save-to-local action and loading/error handling to `NeteasePlaylistDetailView.qml`
- [x] 3.7 Replace the Library placeholder in `src/qml/main.qml` with `LibraryView {}`
- [x] 3.8 Extend `src/qml/main.qml` route switching to render `LocalPlaylistDetailView` and `NeteasePlaylistDetailView`
- [x] 3.9 Add Toast error wiring in `src/qml/main.qml` for `playlistVm`, `mainVm.localPlaylistDetail`, and `mainVm.neteasePlaylistDetail`
- [x] 3.10 Add new detail QML files to `src/qml/qml.qrc`

## 4. Navigation and regression coverage

Commit group goal: lock the new navigation contracts in with viewmodel/QML regression coverage and full validation.

- [x] 4.1 Extend `tests/viewmodel/TestMainViewModel.cpp` to cover library-detail navigation and detail VM lifecycle
- [x] 4.2 Add or update QML tests for new overview/detail page loading and key interactions if the existing QML test harness supports them
- [x] 4.3 Run targeted tests for domain/viewmodel/QML library coverage
- [x] 4.4 Run `just format` and `just test` before marking the change ready for implementation
