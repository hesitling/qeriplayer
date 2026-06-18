## MODIFIED Requirements

### Requirement: MainViewModel owns child ViewModels
The system SHALL define `MainViewModel` as a `QObject` that creates and owns `PlayerViewModel`, `SearchViewModel`, `PlaylistViewModel`, and `SettingsViewModel`. It exposes them as read-only `Q_PROPERTY` (type: `QObject*`). The constructor SHALL accept `ISongRepository*`, `IPlaylistRepository*`, and `NeteaseClient*` in addition to the four child VMs, for use when creating detail VMs on demand.

#### Scenario: Child VMs accessible from QML
- **WHEN** QML accesses `mainViewModel.playerViewModel`, `mainViewModel.searchViewModel`, `mainViewModel.playlistViewModel`, `mainViewModel.settingsViewModel`
- **THEN** each SHALL return a valid `QObject*`

#### Scenario: Constructor accepts repo dependencies
- **WHEN** `MainViewModel` is constructed with 7 arguments (4 VMs + 3 repo/client pointers)
- **THEN** it SHALL store the repo/client pointers for later use in detail VM creation

### Requirement: Detail ViewModel lifecycle
`MainViewModel` SHALL manage detail ViewModels (`LocalPlaylistDetailViewModel`, `NeteasePlaylistDetailViewModel`) as lazy-created, owned children. They SHALL be created when navigating to a detail screen and deleted when navigating away. Detail VMs SHALL receive the stored `ISongRepository*` and `IPlaylistRepository*` (and `NeteaseClient*` for NetEase detail) at construction time.

#### Scenario: LocalPlaylist detail VM created with real repos
- **WHEN** `openLocalPlaylist("abc")` is called
- **THEN** a `LocalPlaylistDetailViewModel` SHALL be created with the stored `IPlaylistRepository*` and `ISongRepository*`, `loadPlaylist("abc")` SHALL be called, `currentView` SHALL become `LocalPlaylist`, and `localPlaylistDetail` property SHALL emit changed

#### Scenario: NeteasePlaylist detail VM created with real repos
- **WHEN** `openNeteasePlaylist(summary)` is called
- **THEN** a `NeteasePlaylistDetailViewModel` SHALL be created with the stored `NeteaseClient*`, `ISongRepository*`, and `IPlaylistRepository*`, `loadPlaylist(summary.id)` SHALL be called, and `currentView` SHALL become `NeteasePlaylist`

#### Scenario: Detail VM deleted on navigation away
- **WHEN** `navigateTo(View::Library)` is called while `currentView == LocalPlaylist`
- **THEN** the `LocalPlaylistDetailViewModel` SHALL be deleted and `localPlaylistDetail` SHALL become `nullptr`
