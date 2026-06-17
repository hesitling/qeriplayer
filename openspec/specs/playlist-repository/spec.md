## Purpose

Defines the playlist repository for managing user playlists and their song membership. Supports CRUD operations, song add/remove/reorder, and normalized song-to-playlist associations via a join table.

## Requirements

### Requirement: Playlist list
The system SHALL provide a method to list all playlists as lightweight `PlaylistSummary` objects (without songs). Results SHALL be ordered by `modified_at` descending.

#### Scenario: List playlists
- **WHEN** `findAll()` is called and 3 playlists exist
- **THEN** exactly 3 `PlaylistSummary` objects SHALL be returned

### Requirement: Playlist detail with songs
The system SHALL provide a method to load a full `Playlist` including its songs. Songs SHALL be loaded by joining `playlist_songs` with `songs_cache`, ordered by `position`.

#### Scenario: Load playlist with songs
- **WHEN** `findById()` is called for a playlist that has 5 songs
- **THEN** the returned `Playlist` SHALL have its `songs` vector populated with 5 songs in position order

#### Scenario: Load non-existent playlist
- **WHEN** `findById()` is called with an ID that does not exist
- **THEN** an empty optional SHALL be returned

### Requirement: Playlist creation
The system SHALL provide a method to create a new playlist with a name and optional platform. The method SHALL generate a unique ID and return the created `Playlist`.

#### Scenario: Create a playlist
- **WHEN** `create("My Playlist")` is called
- **THEN** a new playlist SHALL exist in the `playlists` table with the given name and `song_count = 0`

### Requirement: Playlist metadata update
The system SHALL provide a method to update a playlist's `name`, `description`, and `cover_url`. The `modified_at` timestamp SHALL be updated to the current time.

#### Scenario: Rename a playlist
- **WHEN** `updateMetadata(id, "New Name", desc, cover)` is called
- **THEN** the playlist's `name` SHALL be "New Name" and `modified_at` SHALL be updated

### Requirement: Playlist deletion
The system SHALL provide a method to delete a playlist by ID. Due to `ON DELETE CASCADE`, all related `playlist_songs` entries SHALL be removed automatically. The songs in `songs_cache` SHALL NOT be affected.

#### Scenario: Delete a playlist
- **WHEN** `remove()` is called with an existing playlist ID
- **THEN** the playlist and all its `playlist_songs` entries SHALL be removed, but the songs SHALL still exist in `songs_cache`

### Requirement: Add song to playlist
The system SHALL provide a method to add a song to a playlist at a given position. If position is -1 or omitted, the song SHALL be appended at the end. If the song already exists in the playlist, the operation SHALL be a no-op.

#### Scenario: Append song to playlist
- **WHEN** `addSong(playlistId, songId)` is called and the playlist has 3 songs
- **THEN** the song SHALL be added at position 3 (0-indexed) and `song_count` SHALL be 4

#### Scenario: Insert song at specific position
- **WHEN** `addSong(playlistId, songId, 1)` is called and the playlist has 3 songs
- **THEN** the song SHALL be at position 1 and existing songs at positions 1+ SHALL be shifted

#### Scenario: Add duplicate song
- **WHEN** `addSong(playlistId, songId)` is called and the song already exists in the playlist
- **THEN** the operation SHALL be a no-op and the playlist SHALL be unchanged

### Requirement: Remove song from playlist
The system SHALL provide a method to remove a song from a playlist. Remaining songs SHALL have their positions adjusted to be contiguous.

#### Scenario: Remove a song
- **WHEN** `removeSong(playlistId, songId)` is called
- **THEN** the song SHALL no longer be in the playlist and `song_count` SHALL be decremented

### Requirement: Reorder songs in playlist
The system SHALL provide a method to reorder all songs in a playlist given an ordered list of song IDs. Songs not in the list SHALL be appended at the end.

#### Scenario: Reorder songs
- **WHEN** `reorderSongs(playlistId, [id3, id1, id2])` is called
- **THEN** the songs SHALL have positions 0, 1, 2 matching the given order

### Requirement: Song count
The system SHALL provide a method to return the number of songs in a playlist without loading the full song data.

#### Scenario: Count songs
- **WHEN** `songCount(playlistId)` is called for a playlist with 10 songs
- **THEN** the result SHALL be 10
