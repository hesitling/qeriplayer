## ADDED Requirements

### Requirement: MainViewModel owns child ViewModels
The system SHALL define `MainViewModel` as a `QObject` that creates and owns `PlayerViewModel`, `SearchViewModel`, `PlaylistViewModel`, and `SettingsViewModel`. It exposes them as read-only `Q_PROPERTY` (type: `QObject*`).

#### Scenario: Child VMs accessible from QML
- **WHEN** QML accesses `mainViewModel.playerViewModel`, `mainViewModel.searchViewModel`, `mainViewModel.playlistViewModel`, `mainViewModel.settingsViewModel`
- **THEN** each SHALL return a valid `QObject*`

### Requirement: View navigation enum
`MainViewModel` SHALL define `enum class View : quint8 { Home, Search, Library, LocalPlaylist, NeteasePlaylist, Settings }` as a `Q_ENUM`. It SHALL expose `currentView` (View, read/write, notify: `currentViewChanged`).

#### Scenario: Navigation changes currentView
- **WHEN** `navigateTo(View::Search)` is called
- **THEN** `currentView` SHALL be `Search` and `currentViewChanged` SHALL be emitted

#### Scenario: currentView accessible from QML
- **WHEN** QML reads `mainViewModel.currentView`
- **THEN** it SHALL resolve to the correct enum value

### Requirement: Detail ViewModel lifecycle
`MainViewModel` SHALL manage detail ViewModels (`LocalPlaylistDetailViewModel`, `NeteasePlaylistDetailViewModel`) as lazy-created, owned children. They SHALL be created when navigating to a detail screen and deleted when navigating away.

#### Scenario: LocalPlaylist detail VM created on demand
- **WHEN** `openLocalPlaylist("abc")` is called
- **THEN** a `LocalPlaylistDetailViewModel` SHALL be created, `loadPlaylist("abc")` SHALL be called, `currentView` SHALL become `LocalPlaylist`, and `localPlaylistDetail` property SHALL emit changed

#### Scenario: Detail VM deleted on navigation away
- **WHEN** `navigateTo(View::Library)` is called while `currentView == LocalPlaylist`
- **THEN** the `LocalPlaylistDetailViewModel` SHALL be deleted and `localPlaylistDetail` SHALL become `nullptr`

### Requirement: Navigation from PlaylistViewModel signals
`MainViewModel` SHALL connect:
- `PlaylistViewModel::localPlaylistSelected(id)` → `openLocalPlaylist(id)`
- `PlaylistViewModel::neteasePlaylistSelected(summary)` → `openNeteasePlaylist(summary)`
- `PlaylistViewModel::neteaseAlbumSelected(summary)` → `openNeteaseAlbum(summary)`

#### Scenario: NetEase playlist selection navigates
- **WHEN** `PlaylistViewModel` emits `neteasePlaylistSelected` with a summary
- **THEN** a `NeteasePlaylistDetailViewModel` SHALL be created, `loadPlaylist` SHALL be called, and `currentView` SHALL become `NeteasePlaylist`

### Requirement: Cross-VM play signal wiring
`MainViewModel` SHALL connect:
- `SearchViewModel::requestPlay(song)` → `PlayerViewModel::play(song)`
- `LocalPlaylistDetailViewModel::requestPlay(song)` → `PlayerViewModel::play(song)`
- `LocalPlaylistDetailViewModel::requestPlayPlaylist(songs, index)` → `PlayerViewModel::loadQueueAndPlay(songs, index)`
- `NeteasePlaylistDetailViewModel::requestPlay(song)` → `PlayerViewModel::play(song)`
- `NeteasePlaylistDetailViewModel::requestPlayPlaylist(songs, index)` → `PlayerViewModel::loadQueueAndPlay(songs, index)`

#### Scenario: Search play signal reaches PlayerViewModel
- **WHEN** `SearchViewModel` emits `requestPlay(song)`
- **THEN** `PlayerViewModel::play(song)` SHALL be called

### Requirement: App initialization
`Q_INVOKABLE initialize()` SHALL:
1. Call `SettingsViewModel::loadSettings()`
2. Call `PlaybackController::restoreState()` (via PlayerViewModel)
3. Call `PlaylistViewModel::loadLocalPlaylists()`

#### Scenario: initialize restores state
- **WHEN** `initialize()` is called
- **THEN** settings SHALL be loaded, playback state SHALL be restored, and local playlists SHALL be loaded

### Requirement: Detail VM properties
`MainViewModel` SHALL expose:
- `localPlaylistDetail` (QObject*, read-only, notify: `localPlaylistDetailChanged`)
- `neteasePlaylistDetail` (QObject*, read-only, notify: `neteasePlaylistDetailChanged`)

These SHALL be `nullptr` when no detail screen is active.

#### Scenario: localPlaylistDetail is nullptr initially
- **WHEN** the app starts
- **THEN** `localPlaylistDetail` SHALL be `nullptr`
