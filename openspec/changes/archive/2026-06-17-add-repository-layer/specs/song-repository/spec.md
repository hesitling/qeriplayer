## ADDED Requirements

### Requirement: Song upsert
The system SHALL provide a method to insert or update a song in the `songs_cache` table. If a song with the same `id` already exists, all fields SHALL be updated. The method SHALL serialize `Lyrics` to JSON in the `lyrics_json` column and `QVariantMap` to JSON in the `extra_json` column.

#### Scenario: Insert a new song
- **WHEN** `save()` is called with a `Song` whose `id` does not exist in `songs_cache`
- **THEN** a new row SHALL be inserted with all fields populated

#### Scenario: Update an existing song
- **WHEN** `save()` is called with a `Song` whose `id` already exists in `songs_cache`
- **THEN** all columns SHALL be updated to match the new `Song` fields

#### Scenario: Song with lyrics
- **WHEN** `save()` is called with a `Song` that has a non-empty `Lyrics` object
- **THEN** the lyrics SHALL be serialized as JSON and stored in `lyrics_json`

### Requirement: Song batch save
The system SHALL provide a method to insert or update multiple songs in a single transaction.

#### Scenario: Batch save 100 songs
- **WHEN** `saveBatch()` is called with 100 songs
- **THEN** all 100 songs SHALL be present in `songs_cache` after the call, and the operation SHALL be wrapped in a single transaction

### Requirement: Song lookup by ID
The system SHALL provide a method to find a song by its `id` column. The method SHALL deserialize `lyrics_json` and `extra_json` back into domain objects.

#### Scenario: Find an existing song
- **WHEN** `findById()` is called with an `id` that exists
- **THEN** the returned `Song` SHALL have all fields populated, including deserialized lyrics and extra data

#### Scenario: Find a non-existent song
- **WHEN** `findById()` is called with an `id` that does not exist
- **THEN** an empty optional SHALL be returned

### Requirement: Song lookup by multiple IDs
The system SHALL provide a method to find multiple songs by a list of IDs. Songs not found SHALL be silently omitted from the result.

#### Scenario: Find 3 songs, 2 exist
- **WHEN** `findByIds()` is called with 3 IDs where 2 exist
- **THEN** the result SHALL contain exactly 2 songs

### Requirement: Song deletion
The system SHALL provide a method to delete a song by its `id`. Due to foreign key constraints with `ON DELETE CASCADE`, deleting a song SHALL also remove it from all `playlist_songs` entries.

#### Scenario: Delete a song
- **WHEN** `remove()` is called with an existing song `id`
- **THEN** the song SHALL no longer exist in `songs_cache` and all related `playlist_songs` entries SHALL be removed

### Requirement: Song existence check
The system SHALL provide a method to check whether a song exists by `id` without loading the full record.

#### Scenario: Check existing song
- **WHEN** `exists()` is called with an existing `id`
- **THEN** the result SHALL be `true`

#### Scenario: Check non-existent song
- **WHEN** `exists()` is called with a non-existent `id`
- **THEN** the result SHALL be `false`

### Requirement: Song filter by platform
The system SHALL provide a method to find all songs matching a given `MusicPlatform` enum value.

#### Scenario: Find NetEase songs
- **WHEN** `findByPlatform(MusicPlatform::NetEase)` is called
- **THEN** only songs with `platform = 'NetEase'` SHALL be returned

### Requirement: Song text search
The system SHALL provide a method to search songs by a text query. The search SHALL match against `name`, `artist`, and `album` columns using case-insensitive substring matching. Results SHALL be limited to a configurable count.

#### Scenario: Search by artist name
- **WHEN** `search("Beatles")` is called and 5 songs have "Beatles" in their `artist` field
- **THEN** those 5 songs SHALL be in the results

#### Scenario: Search with limit
- **WHEN** `search("a", 10)` is called and 50 songs match
- **THEN** exactly 10 songs SHALL be returned

### Requirement: Song field mapping
The system SHALL correctly map between domain `Song` fields and database columns:
- `Song.name` ↔ `name` column
- `Song.mediaUri` ↔ `media_uri` column (stored as string)
- `Song.durationMs` ↔ `duration_ms` column
- `Song.coverUrl` ↔ `cover_url` column (stored as string)
- `Song.platform` ↔ `platform` column (stored as enum string)
- `Song.customName` ↔ `custom_name` column
- `Song.localFilePath` ↔ `local_file_path` column
- `Song.lyrics` ↔ `lyrics_json` column (JSON serialized)
- `Song.extra` ↔ `extra_json` column (JSON serialized)

#### Scenario: Round-trip Song through database
- **WHEN** a `Song` with all fields populated is saved and then loaded by ID
- **THEN** the loaded `Song` SHALL be equal to the original in all mapped fields
