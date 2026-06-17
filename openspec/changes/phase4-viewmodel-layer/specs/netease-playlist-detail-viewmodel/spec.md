## ADDED Requirements

### Requirement: NeteasePlaylistDetailViewModel loads remote playlist
The system SHALL define `NeteasePlaylistDetailViewModel` as a `QObject` that fetches a NetEase playlist or album from `NeteaseClient` and displays it.

#### Scenario: Load playlist from API
- **WHEN** `loadPlaylist("12345")` is called
- **THEN** `NeteaseClient::getPlaylistDetail("12345")` SHALL be called and the header and songs SHALL be populated

#### Scenario: Load album from API
- **WHEN** `loadAlbum("67890")` is called
- **THEN** `NeteaseClient::getAlbumDetail("67890")` SHALL be called and the header and songs SHALL be populated

### Requirement: Header properties
`NeteasePlaylistDetailViewModel` SHALL expose:
- `headerName` (QString, read-only, notify: `headerNameChanged`)
- `headerCoverUrl` (QString, read-only, notify: `headerCoverUrlChanged`)
- `headerTrackCount` (int, read-only, notify: `headerTrackCountChanged`)

#### Scenario: Header populated from API response
- **WHEN** a playlist with name "Chill Vibes" and 50 tracks is loaded
- **THEN** `headerName` SHALL be "Chill Vibes" and `headerTrackCount` SHALL be 50

### Requirement: Songs as SongListModel
`NeteasePlaylistDetailViewModel` SHALL expose `songs` as a `SongListModel*` (read-only, parented to the VM).

#### Scenario: Songs populated after load
- **WHEN** a playlist with 50 songs is loaded
- **THEN** `songs->count()` SHALL be 50

### Requirement: Batch-fetch for large playlists
When the API returns more track IDs than tracks (e.g., 1000 IDs but only 100 tracks in the detail response), the ViewModel SHALL batch-fetch missing tracks via `NeteaseClient::getSongDetail(ids)` in pages of 300.

#### Scenario: Large playlist fully fetched
- **WHEN** `loadPlaylist` receives a response with 1000 track IDs but only 100 tracks
- **THEN** the ViewModel SHALL fetch the remaining 900 tracks in batches and `songs->count()` SHALL be 1000

### Requirement: Song caching
On successful load, `NeteasePlaylistDetailViewModel` SHALL call `ISongRepository::saveBatch(songs)` to cache all songs locally.

#### Scenario: Songs cached after load
- **WHEN** a playlist with 50 songs is loaded successfully
- **THEN** `ISongRepository::saveBatch()` SHALL be called with those 50 songs

### Requirement: saveToLocal duplicates to local playlist
`Q_INVOKABLE saveToLocal()` SHALL:
1. Create a local playlist via `IPlaylistRepository::create(headerName)`
2. Cache all songs via `ISongRepository::saveBatch(songs)`
3. Add each song to the playlist via `IPlaylistRepository::addSong()`

#### Scenario: saveToLocal creates local copy
- **WHEN** `saveToLocal()` is called on a NetEase playlist with 30 songs
- **THEN** a new local playlist SHALL be created with `headerName`, all 30 songs SHALL be in `songs_cache`, and the playlist SHALL contain all 30 songs

### Requirement: Retry on error
`Q_INVOKABLE retry()` SHALL re-execute the last `loadPlaylist()` or `loadAlbum()` call.

#### Scenario: retry reloads after error
- **WHEN** `loadPlaylist("123")` fails with a network error and `retry()` is called
- **THEN** `loadPlaylist("123")` SHALL be called again

### Requirement: Play integration
`NeteasePlaylistDetailViewModel` SHALL emit:
- `requestPlay(const Song &song)` for single song play
- `requestPlayPlaylist(const QVector<Song> &songs, int startIndex)` for play-all or context play

#### Scenario: playAll emits playlist signal
- **WHEN** `playAll()` is called
- **THEN** `requestPlayPlaylist` SHALL be emitted with all songs and `startIndex == 0`

### Requirement: Loading and error state
`NeteasePlaylistDetailViewModel` SHALL expose `isLoading` (bool) and `hasError`/`error` (ViewModelError).

#### Scenario: Network error mapped
- **WHEN** `NeteaseClient::getPlaylistDetail()` returns a network error
- **THEN** `hasError` SHALL be `true`, `error.type` SHALL be `Network`, `error.canRetry` SHALL be `true`
