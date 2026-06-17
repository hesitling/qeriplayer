## ADDED Requirements

### Requirement: Schema v2 migration
The system SHALL provide a migration from schema version 1 to version 2. The migration SHALL recreate the `songs_cache` table with renamed and new columns, preserving existing data. The migration SHALL be wrapped in a transaction.

#### Scenario: Migrate v1 database to v2
- **WHEN** a database at schema version 1 is opened
- **THEN** the migration SHALL execute, creating the new `songs_cache` schema with all v1 data copied to the new column names, and the schema version SHALL be set to 2

#### Scenario: Migration preserves existing songs
- **WHEN** a v1 database has 10 songs in `songs_cache`
- **THEN** after migration, all 10 songs SHALL exist in the new `songs_cache` with `title` copied to `name`, `playback_url` copied to `media_uri`, and `duration` copied to `duration_ms`

#### Scenario: Migration creates player_state table
- **WHEN** a v1 database is migrated to v2
- **THEN** the `player_state` table SHALL exist with the singleton constraint

#### Scenario: Migration adds columns to playlists
- **WHEN** a v1 database is migrated to v2
- **THEN** the `playlists` table SHALL have `custom_cover_url` and `modified_at` columns

### Requirement: songs_cache v2 schema
The `songs_cache` table SHALL have the following columns after v2 migration:
- `id` TEXT PRIMARY KEY
- `platform` TEXT
- `name` TEXT
- `artist` TEXT
- `album` TEXT
- `album_id` TEXT
- `duration_ms` INTEGER
- `cover_url` TEXT
- `media_uri` TEXT
- `custom_name` TEXT
- `custom_artist` TEXT
- `custom_cover_url` TEXT
- `original_name` TEXT
- `original_artist` TEXT
- `original_cover_url` TEXT
- `local_file_name` TEXT
- `local_file_path` TEXT
- `matched_lyric_source` TEXT
- `matched_song_id` TEXT
- `user_lyric_offset_ms` INTEGER DEFAULT 0
- `lyrics_json` TEXT
- `channel_id` TEXT
- `audio_id` TEXT
- `sub_audio_id` TEXT
- `extra_json` TEXT
- `cached_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP
- `last_played_at` TIMESTAMP

#### Scenario: Verify v2 songs_cache columns
- **WHEN** a v2 database is queried for `songs_cache` column info
- **THEN** all 27 columns SHALL exist with the specified types

### Requirement: player_state table schema
The `player_state` table SHALL have the following columns:
- `id` INTEGER PRIMARY KEY CHECK(id = 1)
- `playlist_json` TEXT
- `current_index` INTEGER DEFAULT 0
- `media_url` TEXT
- `position_ms` INTEGER DEFAULT 0
- `should_resume` INTEGER DEFAULT 0
- `repeat_mode` INTEGER DEFAULT 0
- `shuffle_enabled` INTEGER DEFAULT 0
- `updated_at` TIMESTAMP

#### Scenario: Singleton constraint
- **WHEN** an INSERT is attempted with `id=2`
- **THEN** the INSERT SHALL fail due to the CHECK constraint
