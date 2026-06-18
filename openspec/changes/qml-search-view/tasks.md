## 1. ViewModel — SearchViewModel helpers + debounce

- [x] 1.1 Add `Q_INVOKABLE void selectPlatformByName(const QString &name)` to `SearchViewModel.h`
- [x] 1.2 Implement `selectPlatformByName()` in `SearchViewModel.cpp` — iterate `m_plugins`, match `platformName()`, call `setSelectedPlatform()`
- [x] 1.3 Add `Q_INVOKABLE void playSong(int index)` to `SearchViewModel.h`
- [x] 1.4 Implement `playSong()` in `SearchViewModel.cpp` — `songAt(index)`, emit `requestPlay(song)` if id non-empty
- [x] 1.5 Change debounce interval from 300ms to 500ms in `SearchViewModel` constructor
- [x] 1.6 Add tests for `selectPlatformByName` in `tests/viewmodel/TestSearchViewModel.cpp`
- [x] 1.7 Add tests for `playSong` in `tests/viewmodel/TestSearchViewModel.cpp`

## 2. SongDelegate.qml

- [x] 2.1 Create `src/qml/SongDelegate.qml` — ItemDelegate with cover art, song name, artist, album, platform badge, duration
- [x] 2.2 Implement cover art (48×48, rounded, Image with placeholder 🎵)
- [x] 2.3 Implement song name (Font.Medium, elide right) and artist+album line (hint color, elide right)
- [x] 2.4 Implement platform badge — right-aligned rounded pill with `platformDisplayName(model.platform)` helper
- [x] 2.5 Implement duration label — right-aligned, `m:ss` format from `model.durationMs`
- [x] 2.6 Implement playing highlight — accent tint when `model.id === playerVm.currentSong.id`
- [x] 2.7 Add `SongDelegate.qml` to `src/qml/qml.qrc`

## 3. SearchView.qml

- [x] 3.1 Create `src/qml/SearchView.qml` — Page with ColumnLayout containing search bar and results list
- [x] 3.2 Implement TextField bound to `searchVm.query` with placeholder text
- [x] 3.3 Implement ComboBox bound to `searchVm.availablePlatforms`, `onActivated` calls `searchVm.selectPlatformByName()`
- [x] 3.4 Implement ListView with `model: searchVm.results` and `delegate: SongDelegate`
- [x] 3.5 Implement double-click handler — `searchVm.playSong(index)`
- [x] 3.6 Implement infinite scroll — `onAtYEndChanged` calls `searchVm.loadMore()` when `hasMore && !isLoading`
- [x] 3.7 Implement footer BusyIndicator — visible when `isLoading && hasMore`
- [x] 3.8 Implement empty states — initial hint (no query) and no-results message
- [x] 3.9 Add `SearchView.qml` to `src/qml/qml.qrc`

## 4. Wire into main.qml

- [x] 4.1 Replace `searchPage` Component in `main.qml` with `SearchView {}`
- [x] 4.2 Add `Connections` block in `main.qml` wiring `searchVm.errorChanged` to `toast.show()`
