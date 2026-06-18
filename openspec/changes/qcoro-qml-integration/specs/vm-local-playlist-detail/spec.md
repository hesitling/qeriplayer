## MODIFIED Requirements

### Requirement: LocalPlaylistDetailViewModel loads local playlist
The system SHALL define `LocalPlaylistDetailViewModel` as a `QObject` that loads and manages a single local playlist from `IPlaylistRepository::findById()`.

#### Scenario: Load playlist by ID
- **WHEN** `loadPlaylist("abc")` is called
- **THEN** `IPlaylistRepository::findById("abc")` SHALL be called and the playlist name and songs SHALL be populated

### Requirement: Song management
`LocalPlaylistDetailViewModel` SHALL expose:
- `Q_INVOKABLE QCoro::QmlTask addSong(const QString &songId)` → adds via `IPlaylistRepository::addSong()`
- `Q_INVOKABLE QCoro::QmlTask removeSong(const QString &songId)` → removes via `IPlaylistRepository::removeSong()`
- `Q_INVOKABLE QCoro::QmlTask reorderSongs(const QStringList &songIds)` → reorders via `IPlaylistRepository::reorderSongs()`

#### Scenario: addSong adds to playlist
- **WHEN** `addSong("song123")` is called
- **THEN** `IPlaylistRepository::addSong(playlistId, "song123")` SHALL be called and `songs` SHALL be refreshed

#### Scenario: QML awaits song addition
- **WHEN** QML calls `vm.addSong("song123").then(callback)`
- **THEN** the callback SHALL fire after the song is added and the model is refreshed

### Requirement: Playlist management
`LocalPlaylistDetailViewModel` SHALL expose:
- `Q_INVOKABLE QCoro::QmlTask rename(const QString &newName)` → updates via `IPlaylistRepository::updateMetadata()`
- `Q_INVOKABLE QCoro::QmlTask deletePlaylist()` → deletes via `IPlaylistRepository::remove()` and emits `playlistDeleted()`

#### Scenario: deletePlaylist removes and signals
- **WHEN** `deletePlaylist()` is called
- **THEN** `IPlaylistRepository::remove(playlistId)` SHALL be called and `playlistDeleted()` SHALL be emitted

#### Scenario: QML awaits playlist deletion
- **WHEN** QML calls `vm.deletePlaylist().then(() => stackView.pop())`
- **THEN** the navigation SHALL occur after the playlist is deleted

## ADDED Requirements

### Requirement: loadPlaylist returns QmlTask
`Q_INVOKABLE QCoro::QmlTask loadPlaylist(const QString &id)` SHALL load the playlist and return a QmlTask that completes when the load finishes.

#### Scenario: QML awaits playlist load
- **WHEN** QML calls `vm.loadPlaylist("abc").then(callback)`
- **THEN** the callback SHALL fire after `playlistName` and `songs` are populated

### Requirement: rename returns QmlTask
`Q_INVOKABLE QCoro::QmlTask rename(const QString &newName)` SHALL update the playlist name and return a QmlTask that completes when the update finishes.

#### Scenario: QML awaits rename
- **WHEN** QML calls `vm.rename("New Name").then(callback)`
- **THEN** the callback SHALL fire after `playlistName` is updated
