## MODIFIED Requirements

### Requirement: NeteasePlaylistDetailViewModel loads remote playlist
The system SHALL define `NeteasePlaylistDetailViewModel` as a `QObject` that fetches a NetEase playlist or album from `NeteaseClient` and displays it.

#### Scenario: Load playlist from API
- **WHEN** `loadPlaylist("12345")` is called
- **THEN** `NeteaseClient::getPlaylistDetail("12345")` SHALL be called and the header and songs SHALL be populated

#### Scenario: Load album from API
- **WHEN** `loadAlbum("67890")` is called
- **THEN** `NeteaseClient::getAlbumDetail("67890")` SHALL be called and the header and songs SHALL be populated

### Requirement: saveToLocal duplicates to local playlist
`Q_INVOKABLE QCoro::QmlTask saveToLocal()` SHALL:
1. Create a local playlist via `IPlaylistRepository::create(headerName)`
2. Cache all songs via `ISongRepository::saveBatch(songs)`
3. Add each song to the playlist via `IPlaylistRepository::addSong()`

#### Scenario: saveToLocal creates local copy
- **WHEN** `saveToLocal()` is called on a NetEase playlist with 30 songs
- **THEN** a new local playlist SHALL be created with `headerName`, all 30 songs SHALL be in `songs_cache`, and the playlist SHALL contain all 30 songs

#### Scenario: QML awaits saveToLocal
- **WHEN** QML calls `vm.saveToLocal().then(() => toast.show("Saved"))`
- **THEN** the toast SHALL appear after the local copy is complete

### Requirement: Retry on error
`Q_INVOKABLE QCoro::QmlTask retry()` SHALL re-execute the last `loadPlaylist()` or `loadAlbum()` call.

#### Scenario: retry reloads after error
- **WHEN** `loadPlaylist("123")` fails with a network error and `retry()` is called
- **THEN** `loadPlaylist("123")` SHALL be called again

## ADDED Requirements

### Requirement: loadPlaylist returns QmlTask
`Q_INVOKABLE QCoro::QmlTask loadPlaylist(const QString &playlistId)` SHALL fetch the playlist and return a QmlTask that completes when the fetch finishes.

#### Scenario: QML awaits playlist load
- **WHEN** QML calls `vm.loadPlaylist("12345").then(callback)`
- **THEN** the callback SHALL fire after header properties and songs are populated

### Requirement: loadAlbum returns QmlTask
`Q_INVOKABLE QCoro::QmlTask loadAlbum(const QString &albumId)` SHALL fetch the album and return a QmlTask that completes when the fetch finishes.

#### Scenario: QML awaits album load
- **WHEN** QML calls `vm.loadAlbum("67890").then(callback)`
- **THEN** the callback SHALL fire after header properties and songs are populated
