# Database Module (core/database/)

## Overview

The database module provides SQLite database access using the raw sqlite3 C API. It manages database lifecycle, schema versioning with a migration system, parameterized queries, and transaction support.

## Source Files

```
src/core/database/
├── DatabaseManager.h
└── DatabaseManager.cpp
```

## DatabaseManager

Manages a single SQLite connection. Supports registering versioned migrations that run automatically on `open()`.

```cpp
class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    // Lifecycle
    bool open(const QString &path);
    void close();
    bool isOpen() const;

    // Queries (positional parameters)
    QVector<QueryRow> exec(const QString &sql, const QVariantList &params = {});

    // Queries (named parameters)
    QVector<QueryRow> execNamed(const QString &sql, const QVariantMap &params);

    // Transactions
    void beginTransaction();
    void commitTransaction();
    void rollbackTransaction();

    // Schema versioning
    int schemaVersion() const;
    void registerMigration(int version, std::function<bool(sqlite3 *)> fn);

    // Info
    int affectedRows() const;
};
```

### Key Types

```cpp
using QueryRow = QVector<QVariant>;

class DatabaseError : public std::runtime_error { ... };
```

### Design Decisions

- **Raw sqlite3 C API** instead of QSqlDatabase/QSqlQuery for minimal overhead and direct control over the SQLite connection.
- **Migration registration**: Call `registerMigration()` before `open()`. Migrations run inside `open()` in version order.
- **Exceptions on error**: `exec()`, `execNamed()`, and transaction methods throw `DatabaseError` on failure. Callers must catch or let propagate.
- **Thread safety**: Not thread-safe. Use from a single thread or external synchronization.

## Schema

The database uses a `schema_version` table to track the current version. The initial schema and subsequent migrations are registered in `QeriPlayerApplication::initializeCoreServices()`.

### Schema v2 Tables

**songs_cache** — Offline song cache with 27 columns:
`id`, `platform`, `name`, `artist`, `album`, `album_id`, `duration_ms`, `cover_url`, `media_uri`, `custom_name`, `custom_artist`, `custom_cover_url`, `original_name`, `original_artist`, `original_cover_url`, `local_file_name`, `local_file_path`, `matched_lyric_source`, `matched_song_id`, `user_lyric_offset_ms`, `lyrics_json`, `channel_id`, `audio_id`, `sub_audio_id`, `extra_json`, `cached_at`, `last_played_at`.

**playlists** — User playlists with `id`, `name`, `description`, `cover_url`, `song_count`, `platform`, `custom_cover_url`, `modified_at`.

**playlist_songs** — Join table: `playlist_id`, `song_id`, `position`. Foreign keys with `ON DELETE CASCADE`.

**settings** — Key-value store: `key` (TEXT PRIMARY KEY), `value` (TEXT).

**play_history** — Play events: `id`, `song_id`, `played_at`. Foreign key to `songs_cache`.

**player_state** — Singleton table (`CHECK(id=1)`): `playlist_json`, `current_index`, `media_url`, `position_ms`, `should_resume`, `repeat_mode`, `shuffle_enabled`, `updated_at`.

## Usage

```cpp
DatabaseManager db;
db.registerMigration(1, [](sqlite3 *handle) { /* v1 schema */ });
db.registerMigration(2, [](sqlite3 *handle) { /* v1→v2 migration */ });
if (!db.open(AppPaths::dataDir() + "/qeriplayer.db")) {
    // handle error
}

// Parameterized query
auto rows = db.exec("SELECT name FROM songs_cache WHERE platform = ?", { "NetEase" });

// Transaction
db.beginTransaction();
try {
    db.exec("INSERT INTO songs_cache (id, name) VALUES (?, ?)", { "1", "Test" });
    db.commitTransaction();
} catch (...) {
    db.rollbackTransaction();
    throw;
}
```

## Testing

See `tests/core/TestDatabase.cpp` and `tests/repo/TestSchemaV2.cpp`.
