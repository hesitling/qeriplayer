## Context

The `DatabaseManager` class provides low-level SQLite access with schema migrations, parameterized queries, and transactions. Domain models (`Song`, `Playlist`, `Album`, etc.) are rich value types with 20-30 fields each. There is no mapping layer between them — callers must write raw SQL and manually unpack `QVariant` rows.

The current schema (v1) has 5 tables with minimal columns. `songs_cache` has ~10 columns but `Song` has ~30 fields. Column names (`title`, `playback_url`, `duration`) don't match domain field names (`name`, `media_uri`, `durationMs`).

Multiple features are blocked: playlist management, offline caching, play history, player state persistence.

## Goals / Non-Goals

**Goals:**
- Provide type-safe repository interfaces for all persistent domain objects
- Expand the schema to store full `Song` data (customizations, lyrics, local files, platform identifiers)
- Enable offline song caching with local file path tracking
- Enable playlist CRUD with normalized song membership
- Enable play history with offline display capability
- Enable player state persistence across sessions

**Non-Goals:**
- Async/QCoro wrappers (sync for now, add later if needed)
- Cache eviction / LRU (let it grow, add later)
- Cloud sync integration (Android has it, Qt port doesn't yet)
- Reactive change notifications (no signals/observers on repositories)
- Generic ORM or code generation

## Decisions

### 1. Schema: Hybrid columns + JSON blobs

**Decision**: Store queryable fields as proper columns. Store large/heterogeneous data (lyrics, platform extras) as JSON text columns.

**Rationale**: Fields needed for display and filtering (name, artist, platform, local_file_path) must be columns for efficient querying. Lyrics are large and rarely queried directly — JSON blob avoids schema bloat. Platform-specific extras vary by platform — JSON avoids NULL-heavy wide rows.

**Alternatives considered**:
- *Wide table (all columns)*: Rejected — many NULLs for platform-specific fields, migrations for every new field.
- *All JSON blob*: Rejected — can't query or index JSON fields efficiently in SQLite.

### 2. Column naming: Rename to match domain

**Decision**: Rename `title`→`name`, `playback_url`→`media_uri`, `duration`→`duration_ms` in the migration.

**Rationale**: The database is new with no production data. Consistent naming between domain models and schema reduces cognitive load and mapping complexity.

**Alternatives considered**:
- *Keep old names, map in SqlRowMapper*: Rejected — creates permanent naming inconsistency.

### 3. Migration strategy: Table recreation

**Decision**: Create `songs_cache_v2` with new schema, copy data from `songs_cache`, drop old table, rename new table.

**Rationale**: SQLite's `ALTER TABLE` cannot rename columns or change types. Table recreation is the standard SQLite migration pattern for structural changes.

### 4. Playlist songs: Normalized join table

**Decision**: Keep `playlist_songs` as a join table referencing `songs_cache(id)`.

**Rationale**: Less data duplication. Songs appear in multiple playlists without duplicating 30+ fields. Proper foreign key constraints ensure referential integrity.

**Alternatives considered**:
- *Denormalized (embed songs in playlist)*: Rejected — duplicates data, harder to update song metadata globally.

### 5. Sync over async

**Decision**: All repository methods are synchronous.

**Rationale**: SQLite local file queries are µs-scale. The UI won't block noticeably. Avoids QCoro complexity for now. Can wrap in async later if needed.

**Alternatives considered**:
- *QCoro from day one*: Rejected — premature complexity. SQLite latency is negligible for desktop.

### 6. Repository interfaces as abstract classes

**Decision**: Define pure virtual interfaces (`ISongRepository`, etc.) with concrete implementations.

**Rationale**: Enables testing with mock implementations. ViewModels depend on interfaces, not concrete classes. ServiceLocator provides the concrete instance.

### 7. Audio file storage: Cache directory + DB path

**Decision**: Store downloaded audio files in the app's cache directory. Record the file path in `songs_cache.local_file_path`.

**Rationale**: Simple approach for desktop. No scoped storage complexity like Android. The path is just a string in the DB.

### 8. Player state: Singleton table

**Decision**: `player_state` table with `CHECK(id=1)` constraint — only one row ever exists.

**Rationale**: There's only one player. A singleton table is simpler than a KV approach for structured data with multiple fields.

## Risks / Trade-offs

**[Risk] Schema migration on existing v1 databases** → Mitigation: The migration copies existing data from `songs_cache` to the new table. Since v1 is new and likely has minimal data, data loss risk is low. The migration is wrapped in a transaction.

**[Risk] No cache eviction** → Mitigation: Defer to later. Desktop storage is abundant. Add LRU based on `last_played_at` when needed.

**[Risk] Sync blocking on large queries** → Mitigation: For typical use (hundreds of songs, not millions), sync queries are fast. If a specific operation proves slow (e.g., full-text search on 10k songs), wrap that one in async.

**[Risk] Lyrics JSON blob size** → Mitigation: Lyrics are typically 5-50KB. Even with 1000 cached songs, total is ~50MB — acceptable for SQLite.

**[Trade-off] No reactive notifications** → Consumers must re-query after mutations. Acceptable for now; can add Qt signals later if needed.
