## ADDED Requirements

### Requirement: LibraryView page layout
The system SHALL define `LibraryView.qml` as a QML Page containing tabbed sections for local playlists, NetEase playlists, and NetEase albums.

#### Scenario: Page structure
- **WHEN** LibraryView is loaded
- **THEN** it SHALL display tabs for Local, NetEase Playlists, and NetEase Albums

### Requirement: Local library section
The Local tab SHALL display `playlistVm.localPlaylists` and allow creating a new local playlist.

#### Scenario: Local playlists visible
- **WHEN** `playlistVm.localPlaylists` contains entries
- **THEN** the Local tab SHALL render those entries as cards or rows

#### Scenario: Create playlist dialog
- **WHEN** the user activates the create action in the Local tab
- **THEN** a dialog SHALL prompt for a playlist name and call `playlistVm.createLocalPlaylist(name)` on confirm

### Requirement: Lazy remote loading
The NetEase tabs SHALL load their data only when first activated.

#### Scenario: NetEase playlists lazy load
- **WHEN** the user first opens the NetEase Playlists tab
- **THEN** `playlistVm.loadNeteasePlaylists()` SHALL be called

#### Scenario: NetEase albums lazy load
- **WHEN** the user first opens the NetEase Albums tab
- **THEN** `playlistVm.loadNeteaseAlbums()` SHALL be called

### Requirement: Remote loading state and retry
The view SHALL show a loading indicator while the active remote section is loading and SHALL provide a retry action when the active section has an error.

#### Scenario: Loading indicator visible
- **WHEN** a NetEase tab is active and `playlistVm.isLoading` is true
- **THEN** the tab content SHALL show a BusyIndicator

#### Scenario: Retry after error
- **WHEN** the active NetEase tab has an error and the user selects retry
- **THEN** the corresponding NetEase load method SHALL be called again

### Requirement: Playlist and album cards
Each library entry SHALL display summary metadata via QML-readable properties from `PlaylistSummary` or `AlbumSummary`.

#### Scenario: Card metadata shown
- **WHEN** a card is rendered for a playlist summary
- **THEN** it SHALL show the summary name and cover image

### Requirement: Open playlist detail
Selecting a local playlist SHALL open the local playlist detail view. Selecting a NetEase playlist or album SHALL open the NetEase detail view.

#### Scenario: Open local playlist
- **WHEN** the user activates a local playlist card at index 0
- **THEN** the app SHALL navigate to `LocalPlaylistDetailView`

#### Scenario: Open NetEase playlist
- **WHEN** the user activates a NetEase playlist card at index 0
- **THEN** the app SHALL navigate to `NeteasePlaylistDetailView`
