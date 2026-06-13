## ADDED Requirements

### Requirement: DatabaseManager lifecycle
The system SHALL provide a `DatabaseManager` class that opens an SQLite database file at a configurable path. Opening SHALL create the file if it does not exist. Closing SHALL flush all pending writes and release the file handle.

#### Scenario: Open a new database
- **WHEN** `DatabaseManager::open(path)` is called with a path to a non-existent file
- **THEN** the file SHALL be created and the database SHALL be opened successfully

#### Scenario: Open an existing database
- **WHEN** `DatabaseManager::open(path)` is called with a path to a valid SQLite file
- **THEN** the database SHALL be opened and existing tables SHALL be accessible

#### Scenario: Close a database
- **WHEN** `DatabaseManager::close()` is called after a successful open
- **THEN** the file handle SHALL be released and subsequent operations SHALL fail gracefully

### Requirement: Schema versioning
The system SHALL maintain a `schema_version` table with a single integer column. On open, `DatabaseManager` SHALL read the current version. If the version is lower than the expected version, migrations SHALL be applied sequentially.

#### Scenario: First-time database creation
- **WHEN** a database is opened for the first time
- **THEN** the `schema_version` table SHALL be created with version 0, then all migrations up to the current version SHALL be applied

#### Scenario: Database at current version
- **WHEN** a database is opened and its version matches the current expected version
- **THEN** no migrations SHALL be applied

#### Scenario: Database behind by two versions
- **WHEN** a database is at version N and the current version is N+2
- **THEN** migration N+1 SHALL be applied first, then migration N+2

### Requirement: Migration registration
The system SHALL allow registering migration functions keyed by version number. Each migration SHALL receive a raw SQLite handle and SHALL execute DDL/DML statements to bring the schema to that version.

#### Scenario: Register and run a migration
- **WHEN** a migration for version 1 is registered that creates a `songs` table
- **THEN** after opening a version-0 database, the `songs` table SHALL exist

### Requirement: Query execution
The system SHALL provide methods to execute SQL statements with bound parameters. Parameters SHALL be bindable by position (1-indexed) or by name (`:param`). The system SHALL return results as rows of QVariant values.

#### Scenario: Execute a SELECT with positional parameters
- **WHEN** `exec("SELECT * FROM songs WHERE id = ?", {"abc"})` is called
- **THEN** all matching rows SHALL be returned with column values as QVariant

#### Scenario: Execute an INSERT
- **WHEN** `exec("INSERT INTO songs (id, title) VALUES (?, ?)", {"abc", "Test"})` is called
- **THEN** a new row with id="abc" and title="Test" SHALL exist in the songs table

#### Scenario: Execute with a named parameter
- **WHEN** `exec("SELECT * FROM songs WHERE id = :id", {{":id", "abc"}})` is called
- **THEN** the query SHALL return the same results as the positional equivalent

### Requirement: Transaction support
The system SHALL provide `beginTransaction()`, `commitTransaction()`, and `rollbackTransaction()` methods. Within a transaction, all statements SHALL be part of the same atomic unit.

#### Scenario: Commit a transaction
- **WHEN** a transaction is begun, two inserts are executed, and the transaction is committed
- **THEN** both rows SHALL be visible after the commit

#### Scenario: Rollback a transaction
- **WHEN** a transaction is begun, an insert is executed, and the transaction is rolled back
- **THEN** the inserted row SHALL NOT be visible

### Requirement: Error handling
All database operations SHALL return a result type or throw a `DatabaseError` exception on failure. The error message SHALL include the SQLite error string.

#### Scenario: Execute invalid SQL
- **WHEN** `exec("INVALID SQL", {})` is called
- **THEN** a `DatabaseError` SHALL be thrown with a message containing the SQLite error description

### Requirement: Initial schema tables
The initial schema (version 1) SHALL create the following tables:
- `songs_cache`: id (TEXT PK), platform (TEXT), title, artist, album, duration (INTEGER), cover_url, playback_url, extra_json, cached_at (TIMESTAMP)
- `playlists`: id (TEXT PK), platform (TEXT), name, description, cover_url, song_count (INTEGER), owner
- `playlist_songs`: playlist_id (TEXT FK), song_id (TEXT FK), position (INTEGER), PRIMARY KEY (playlist_id, song_id)
- `settings`: key (TEXT PK), value (TEXT)
- `play_history`: id INTEGER PRIMARY KEY AUTOINCREMENT, song_id (TEXT), played_at (TIMESTAMP)

#### Scenario: Schema creation on fresh database
- **WHEN** a fresh database is opened with the initial migration
- **THEN** all five tables SHALL exist with the specified columns and constraints
