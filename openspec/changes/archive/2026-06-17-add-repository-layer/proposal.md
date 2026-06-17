## Why

The database module (`DatabaseManager`) provides raw SQLite access, but there is no data access layer between raw SQL and domain models. Callers must hand-write SQL, manually map `QVariant` rows to domain objects, and know the table schemas. This blocks playlist management, offline song caching, play history, and player state persistence — all features that need to read and write domain objects to the database.

## What Changes

- **Schema v2**: Expand `songs_cache` with 15+ new columns (customizations, local file paths, lyrics, platform identifiers, timestamps). Rename `title`→`name`, `playback_url`→`media_uri`, `duration`→`duration_ms` to match domain models. Add `custom_cover_url` and `modified_at` to `playlists`. Add new `player_state` singleton table for playback persistence.
- **Repository interfaces**: Five abstract interfaces (`ISongRepository`, `IPlaylistRepository`, `IPlayHistoryRepository`, `IPlayerStateRepository`, `ISettingsRepository`) defining the data access contract.
- **Repository implementations**: Concrete SQLite-backed implementations using `DatabaseManager`.
- **SQL↔Domain mapper**: `SqlRowMapper` utility that converts between `QVariant` rows and domain types, handling JSON serialization for lyrics and extra fields.
- **Service wiring**: Register all repositories in `ServiceLocator` during application startup.

## Capabilities

### New Capabilities
- `song-repository`: CRUD operations for songs in the offline cache, including upsert, batch save, platform filtering, and text search.
- `playlist-repository`: Playlist CRUD with normalized song membership (join table), including create, delete, add/remove/reorder songs.
- `play-history-repository`: Record play events and query recent history, joining with songs_cache for offline display.
- `player-state-repository`: Persist and restore player state (queue, position, repeat/shuffle) across sessions using a singleton table.
- `settings-repository`: Key-value settings persistence with typed accessors (bool, int, string).

### Modified Capabilities
- `database`: Schema migration from v1 to v2 (column renames, new columns, new table).

## Impact

- **Files created**: 17 new source files (5 interfaces, 5 implementations, 1 mapper, 6 headers) + 5 test files.
- **Files modified**: `DatabaseManager.cpp` (v2 migration), `NeriPlayerApplication.cpp` (service wiring), `CMakeLists.txt` (new files).
- **Schema**: Breaking change to `songs_cache` table structure. Existing v1 databases will be migrated automatically via table recreation.
- **Dependencies**: No new external dependencies. Uses existing SQLite, Qt, and nlohmann/json (via `QJsonDocument`).
