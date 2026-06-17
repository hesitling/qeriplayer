# Repository Layer (repo/)

## Overview

The repository layer provides SQLite-backed persistence for songs, playlists, player state, settings, and play history. Each repository has an abstract interface (`IXxxRepository`) and a concrete SQLite implementation (`XxxRepository`).

## Source Files

```text
src/repo/
├── ISongRepository.h          # Song CRUD interface
├── SongRepository.h / .cpp    # SQLite implementation
├── IPlaylistRepository.h      # Playlist CRUD interface
├── PlaylistRepository.h / .cpp
├── IPlayerStateRepository.h   # Player state persistence interface
├── PlayerStateRepository.h / .cpp
├── ISettingsRepository.h      # Key-value settings interface
├── SettingsRepository.h / .cpp
├── IPlayHistoryRepository.h   # Play history interface
├── PlayHistoryRepository.h / .cpp
└── SqlRowMapper.h / .cpp      # Shared SQL row → domain type mapping
```

## ISongRepository / SongRepository

CRUD operations on the `songs_cache` table.

```cpp
class ISongRepository {
public:
    virtual std::optional<Song> findById(const QString &id) = 0;
    virtual QVector<Song> findByIds(const QStringList &ids) = 0;
    virtual void save(const Song &song) = 0;            // INSERT OR REPLACE
    virtual void saveBatch(const QVector<Song> &songs) = 0; // Transaction
    virtual void remove(const QString &id) = 0;          // Cascades to playlist_songs
    virtual bool exists(const QString &id) = 0;
    virtual QVector<Song> findByPlatform(MusicPlatform platform) = 0;
    virtual QVector<Song> search(const QString &query, int limit = 50) = 0;
};
```

- `save()` serializes `Lyrics` to JSON in `lyrics_json` and `QVariantMap` to JSON in `extra_json`.
- `search()` does case-insensitive substring matching on `name`, `artist`, and `album`.

## IPlaylistRepository / PlaylistRepository

Playlist CRUD with song membership management.

```cpp
class IPlaylistRepository {
public:
    virtual QVector<PlaylistSummary> findAll() = 0;           // Ordered by modified_at desc
    virtual std::optional<Playlist> findById(const QString &id) = 0; // With songs
    virtual Playlist create(const QString &name, MusicPlatform platform = MusicPlatform::Unknown) = 0;
    virtual void updateMetadata(const QString &id, const QString &name,
                                const QString &description, const QString &coverUrl) = 0;
    virtual void remove(const QString &id) = 0;               // Cascades to playlist_songs
    virtual bool addSong(const QString &playlistId, const QString &songId, int position = -1) = 0;
    virtual void removeSong(const QString &playlistId, const QString &songId) = 0;
    virtual void reorderSongs(const QString &playlistId, const QStringList &songIds) = 0;
    virtual int songCount(const QString &playlistId) = 0;
};
```

- `addSong()` with `position = -1` appends. Returns `false` if song already in playlist.
- `reorderSongs()` sets positions to match the given order. Songs not in the list are appended.
- `remove()` does NOT delete songs from `songs_cache` — only the playlist and join table entries.

## IPlayerStateRepository / PlayerStateRepository

Persists playback state in a singleton `player_state` table.

```cpp
class IPlayerStateRepository {
public:
    virtual void save(const PersistedPlayerState &state) = 0; // INSERT OR REPLACE
    virtual std::optional<PersistedPlayerState> load() = 0;
    virtual void clear() = 0;
};
```

- The table has `CHECK(id=1)` to enforce singleton.
- `playlist` is serialized as JSON in `playlist_json`.

## ISettingsRepository / SettingsRepository

Key-value settings persistence.

```cpp
class ISettingsRepository {
public:
    virtual std::optional<QString> get(const QString &key) = 0;
    virtual void set(const QString &key, const QString &value) = 0; // Upsert
    virtual void remove(const QString &key) = 0;                    // No-op if missing
    virtual QVariantMap getAll() = 0;
    virtual bool getBool(const QString &key, bool defaultValue = false) = 0;
    virtual int getInt(const QString &key, int defaultValue = 0) = 0;
};
```

## IPlayHistoryRepository / PlayHistoryRepository

Records and queries song playback events.

```cpp
class IPlayHistoryRepository {
public:
    virtual void record(const QString &songId) = 0;  // Inserts row + updates last_played_at
    virtual QVector<Song> recent(int limit = 50) = 0; // Distinct songs, most recent first
    virtual void clear() = 0;
    virtual void remove(const QStringList &songIds) = 0;
    virtual int playCount(const QString &songId) = 0;
};
```

- `recent()` joins `play_history` with `songs_cache` to return full `Song` objects.
- Each song appears at most once (most recent play wins).

## SqlRowMapper

Shared utility for mapping SQL result rows to domain types. Used by all repository implementations.

## Testing

- `tests/repo/TestSongRepository.cpp`
- `tests/repo/TestPlaylistRepository.cpp`
- `tests/repo/TestPlayerStateRepository.cpp`
- `tests/repo/TestSettingsRepository.cpp`
- `tests/repo/TestPlayHistoryRepository.cpp`
- `tests/repo/TestSchemaV2.cpp`
- `tests/repo/TestSqlRowMapper.cpp`
