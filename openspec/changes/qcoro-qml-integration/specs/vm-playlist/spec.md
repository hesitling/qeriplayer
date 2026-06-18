## MODIFIED Requirements

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
- `Q_INVOKABLE QCoro::QmlTask createLocalPlaylist(const QString &name)` → creates via `IPlaylistRepository::create()`
- `Q_INVOKABLE QCoro::QmlTask deleteLocalPlaylist(const QString &id)` → deletes via `IPlaylistRepository::remove()`
- `Q_INVOKABLE QCoro::QmlTask renameLocalPlaylist(const QString &id, const QString &name)` → updates via `IPlaylistRepository::updateMetadata()`

#### Scenario: Create local playlist
- **WHEN** `createLocalPlaylist("My Playlist")` is called
- **THEN** `IPlaylistRepository::create("My Playlist")` SHALL be called and `localPlaylistsChanged` SHALL be emitted

#### Scenario: QML awaits playlist creation
- **WHEN** QML calls `playlistVm.createLocalPlaylist("My Playlist").then(callback)`
- **THEN** the callback SHALL fire after the playlist is created and the list is refreshed

### Requirement: Loading state
`PlaylistViewModel` SHALL expose `isLoading` (bool, read-only, notify: `isLoadingChanged`).

#### Scenario: Loading during API fetch
- **WHEN** `loadNeteasePlaylists()` is called
- **THEN** `isLoading` SHALL be `true` until the API responds, then `false`

## ADDED Requirements

### Requirement: loadLocalPlaylists returns QmlTask
`Q_INVOKABLE QCoro::QmlTask loadLocalPlaylists()` SHALL load playlists from `IPlaylistRepository::findAll()` and return a QmlTask that completes when the load finishes.

#### Scenario: QML awaits local playlists load
- **WHEN** QML calls `playlistVm.loadLocalPlaylists().then(callback)`
- **THEN** the callback SHALL fire after `localPlaylists` is populated

### Requirement: loadNeteasePlaylists returns QmlTask
`Q_INVOKABLE QCoro::QmlTask loadNeteasePlaylists()` SHALL fetch playlists from `NeteaseClient::getUserPlaylists()` and return a QmlTask that completes when the fetch finishes.

#### Scenario: QML awaits NetEase playlists load
- **WHEN** QML calls `playlistVm.loadNeteasePlaylists().then(callback)`
- **THEN** the callback SHALL fire after `neteasePlaylists` is populated

### Requirement: loadNeteaseAlbums returns QmlTask
`Q_INVOKABLE QCoro::QmlTask loadNeteaseAlbums()` SHALL fetch albums from `NeteaseClient::getUserStarredAlbums()` and return a QmlTask that completes when the fetch finishes.

#### Scenario: QML awaits NetEase albums load
- **WHEN** QML calls `playlistVm.loadNeteaseAlbums().then(callback)`
- **THEN** the callback SHALL fire after `neteaseAlbums` is populated
