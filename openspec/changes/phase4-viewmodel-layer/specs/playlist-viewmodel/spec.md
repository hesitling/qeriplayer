## ADDED Requirements

### Requirement: PlaylistViewModel manages library overview
The system SHALL define `PlaylistViewModel` as a `QObject` that lists playlists from multiple sources (local, NetEase) for the library screen.

#### Scenario: PlaylistViewModel exposes playlist lists
- **WHEN** QML accesses `playlistViewModel.localPlaylists` and `playlistViewModel.neteasePlaylists`
- **THEN** each SHALL return the respective playlist list

### Requirement: Local playlists property
`PlaylistViewModel` SHALL expose `localPlaylists` (QVariantList of PlaylistSummary, read-only, notify: `localPlaylistsChanged`). Loaded via `IPlaylistRepository::findAll()`.

#### Scenario: Local playlists loaded from repository
- **WHEN** `loadLocalPlaylists()` is called and the repo has 5 playlists
- **THEN** `localPlaylists` SHALL contain 5 entries and `localPlaylistsChanged` SHALL be emitted

### Requirement: NetEase playlists property
`PlaylistViewModel` SHALL expose `neteasePlaylists` (QVariantList of PlaylistSummary, read-only, notify: `neteasePlaylistsChanged`). Loaded via `NeteaseClient::getUserPlaylists()`.

#### Scenario: NetEase playlists loaded from API
- **WHEN** `loadNeteasePlaylists()` is called and the API returns 10 playlists
- **THEN** `neteasePlaylists` SHALL contain 10 entries

### Requirement: NetEase albums property
`PlaylistViewModel` SHALL expose `neteaseAlbums` (QVariantList of AlbumSummary, read-only, notify: `neteaseAlbumsChanged`). Loaded via `NeteaseClient::getUserStarredAlbums()`.

#### Scenario: NetEase albums loaded from API
- **WHEN** `loadNeteaseAlbums()` is called and the API returns 8 albums
- **THEN** `neteaseAlbums` SHALL contain 8 entries

### Requirement: Local playlist CRUD
`PlaylistViewModel` SHALL expose:
- `Q_INVOKABLE createLocalPlaylist(const QString &name)` → creates via `IPlaylistRepository::create()`
- `Q_INVOKABLE deleteLocalPlaylist(const QString &id)` → deletes via `IPlaylistRepository::remove()`
- `Q_INVOKABLE renameLocalPlaylist(const QString &id, const QString &name)` → updates via `IPlaylistRepository::updateMetadata()`

#### Scenario: Create local playlist
- **WHEN** `createLocalPlaylist("My Playlist")` is called
- **THEN** `IPlaylistRepository::create("My Playlist")` SHALL be called and `localPlaylistsChanged` SHALL be emitted

### Requirement: Loading state
`PlaylistViewModel` SHALL expose `isLoading` (bool, read-only, notify: `isLoadingChanged`).

#### Scenario: Loading during API fetch
- **WHEN** `loadNeteasePlaylists()` is called
- **THEN** `isLoading` SHALL be `true` until the API responds, then `false`

### Requirement: Error state
`PlaylistViewModel` SHALL expose `hasError` (bool, notify: `errorChanged`) and `error` (ViewModelError, notify: `errorChanged`).

#### Scenario: API error surfaced
- **WHEN** `NeteaseClient::getUserPlaylists()` returns an error
- **THEN** `hasError` SHALL be `true` and `error` SHALL contain the mapped error

### Requirement: Selection signals
`PlaylistViewModel` SHALL emit:
- `localPlaylistSelected(const QString &id)` when user selects a local playlist
- `neteasePlaylistSelected(const PlaylistSummary &summary)` when user selects a NetEase playlist
- `neteaseAlbumSelected(const AlbumSummary &summary)` when user selects a NetEase album

#### Scenario: Local playlist selection
- **WHEN** the user clicks local playlist with id "abc"
- **THEN** `localPlaylistSelected("abc")` SHALL be emitted

### Requirement: clearError
`Q_INVOKABLE clearError()` SHALL clear the error state.

#### Scenario: clearError resets error
- **WHEN** `clearError()` is called when `hasError` is `true`
- **THEN** `hasError` SHALL become `false`
