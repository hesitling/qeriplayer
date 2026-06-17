## ADDED Requirements

### Requirement: LocalPlaylistDetailViewModel loads local playlist
The system SHALL define `LocalPlaylistDetailViewModel` as a `QObject` that loads and manages a single local playlist from `IPlaylistRepository::findById()`.

#### Scenario: Load playlist by ID
- **WHEN** `loadPlaylist("abc")` is called
- **THEN** `IPlaylistRepository::findById("abc")` SHALL be called and the playlist name and songs SHALL be populated

### Requirement: Playlist identity properties
`LocalPlaylistDetailViewModel` SHALL expose:
- `playlistId` (QString, read/write, notify: `playlistIdChanged`)
- `playlistName` (QString, read-only, notify: `playlistNameChanged`)

#### Scenario: playlistName reflects loaded playlist
- **WHEN** a playlist with name "Road Trip" is loaded
- **THEN** `playlistName` SHALL be "Road Trip"

### Requirement: Songs as SongListModel
`LocalPlaylistDetailViewModel` SHALL expose `songs` as a `SongListModel*` (read-only, notify: `songsChanged`, parented to the VM).

#### Scenario: Songs populated after load
- **WHEN** a playlist with 15 songs is loaded
- **THEN** `songs->count()` SHALL be 15

### Requirement: Song management
`LocalPlaylistDetailViewModel` SHALL expose:
- `Q_INVOKABLE addSong(const QString &songId)` → adds via `IPlaylistRepository::addSong()`
- `Q_INVOKABLE removeSong(const QString &songId)` → removes via `IPlaylistRepository::removeSong()`
- `Q_INVOKABLE reorderSongs(const QStringList &songIds)` → reorders via `IPlaylistRepository::reorderSongs()`

#### Scenario: addSong adds to playlist
- **WHEN** `addSong("song123")` is called
- **THEN** `IPlaylistRepository::addSong(playlistId, "song123")` SHALL be called and `songs` SHALL be refreshed

### Requirement: Playlist management
`LocalPlaylistDetailViewModel` SHALL expose:
- `Q_INVOKABLE rename(const QString &newName)` → updates via `IPlaylistRepository::updateMetadata()`
- `Q_INVOKABLE deletePlaylist()` → deletes via `IPlaylistRepository::remove()` and emits `playlistDeleted()`

#### Scenario: deletePlaylist removes and signals
- **WHEN** `deletePlaylist()` is called
- **THEN** `IPlaylistRepository::remove(playlistId)` SHALL be called and `playlistDeleted()` SHALL be emitted

### Requirement: Play integration
`LocalPlaylistDetailViewModel` SHALL emit:
- `requestPlay(const Song &song)` when a single song is selected
- `requestPlayPlaylist(const QVector<Song> &songs, int startIndex)` when "play all" or a song-in-context is selected

#### Scenario: Play all emits playlist signal
- **WHEN** `playAll()` is called on a playlist with 10 songs
- **THEN** `requestPlayPlaylist` SHALL be emitted with all 10 songs and `startIndex == 0`

### Requirement: Loading and error state
`LocalPlaylistDetailViewModel` SHALL expose `isLoading` (bool) and `hasError`/`error` (ViewModelError).

#### Scenario: Error when playlist not found
- **WHEN** `loadPlaylist("nonexistent")` is called and `findById` returns empty
- **THEN** `hasError` SHALL be `true` and `error.type` SHALL be `NotFound`
