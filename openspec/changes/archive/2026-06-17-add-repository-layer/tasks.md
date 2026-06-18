## 1. Schema Migration

- [x] 1.1 Add v2 migration to `DatabaseManager::applyInitialSchema()` — recreate `songs_cache` with 27 columns, rename `title`→`name`, `playback_url`→`media_uri`, `duration`→`duration_ms`, copy existing data
- [x] 1.2 Add `player_state` table creation to v2 migration (singleton with CHECK constraint)
- [x] 1.3 Add `custom_cover_url` TEXT and `modified_at` INTEGER columns to `playlists` table in v2 migration
- [x] 1.4 Update `TestDatabase` — update `initialSchema_createsAllTables` for new schema, add migration test for v1→v2

## 2. SQL↔Domain Mapper

- [x] 2.1 Create `src/repo/SqlRowMapper.h` — declare `toSong()`, `songToInsertParams()`, `songToUpdateParams()`, `toPlaylistSummary()`, `playerStateToJson()`, `playerStateFromJson()`
- [x] 2.2 Create `src/repo/SqlRowMapper.cpp` — implement Song↔QVariant mapping, handle lyrics_json and extra_json serialization via QJsonDocument

## 3. Repository Interfaces

- [x] 3.1 Create `src/repo/ISongRepository.h` — pure virtual interface with findById, findByIds, save, saveBatch, remove, exists, findByPlatform, search
- [x] 3.2 Create `src/repo/IPlaylistRepository.h` — pure virtual interface with findAll, findById, create, updateMetadata, remove, addSong, removeSong, reorderSongs, songCount
- [x] 3.3 Create `src/repo/IPlayHistoryRepository.h` — pure virtual interface with record, recent, clear, remove, playCount
- [x] 3.4 Create `src/repo/IPlayerStateRepository.h` — pure virtual interface with save, load, clear
- [x] 3.5 Create `src/repo/ISettingsRepository.h` — pure virtual interface with get, set, remove, getAll, getBool, getInt

## 4. SongRepository Implementation

- [x] 4.1 Create `src/repo/SongRepository.h` — class declaration inheriting ISongRepository, holding DatabaseManager pointer
- [x] 4.2 Create `src/repo/SongRepository.cpp` — implement save (INSERT OR REPLACE), findById, findByIds, remove, exists
- [x] 4.3 Implement findByPlatform and search (LIKE queries on name/artist/album) in SongRepository.cpp

## 5. SettingsRepository Implementation

- [x] 5.1 Create `src/repo/SettingsRepository.h` — class declaration inheriting ISettingsRepository
- [x] 5.2 Create `src/repo/SettingsRepository.cpp` — implement get, set, remove, getAll, getBool, getInt

## 6. PlayHistoryRepository Implementation

- [x] 6.1 Create `src/repo/PlayHistoryRepository.h` — class declaration inheriting IPlayHistoryRepository
- [x] 6.2 Create `src/repo/PlayHistoryRepository.cpp` — implement record (INSERT + update last_played_at), recent (JOIN with songs_cache), clear, remove, playCount

## 7. PlaylistRepository Implementation

- [x] 7.1 Create `src/repo/PlaylistRepository.h` — class declaration inheriting IPlaylistRepository
- [x] 7.2 Create `src/repo/PlaylistRepository.cpp` — implement findAll, findById (with JOIN), create, updateMetadata, remove
- [x] 7.3 Implement addSong, removeSong, reorderSongs, songCount in PlaylistRepository.cpp

## 8. PlayerStateRepository Implementation

- [x] 8.1 Create `src/repo/PlayerStateRepository.h` — class declaration inheriting IPlayerStateRepository
- [x] 8.2 Create `src/repo/PlayerStateRepository.cpp` — implement save (INSERT OR REPLACE), load, clear with JSON serialization

## 9. Service Wiring

- [x] 9.1 Update `QeriPlayerApplication.cpp` — register all 5 repositories in ServiceLocator after DatabaseManager
- [x] 9.2 Update `CMakeLists.txt` — add all new repo source and header files to SOURCES and HEADERS

## 10. Tests

- [x] 10.1 Create `tests/repo/TestSongRepository.cpp` — test save/load round-trip, findById, findByIds, findByPlatform, search, remove, exists
- [x] 10.2 Create `tests/repo/TestSettingsRepository.cpp` — test get, set, remove, getAll, getBool, getInt
- [x] 10.3 Create `tests/repo/TestPlayHistoryRepository.cpp` — test record, recent ordering, clear, remove, playCount
- [x] 10.4 Create `tests/repo/TestPlaylistRepository.cpp` — test create, findById with songs, addSong, removeSong, reorderSongs, remove
- [x] 10.5 Create `tests/repo/TestPlayerStateRepository.cpp` — test save/load round-trip, singleton behavior, clear
- [x] 10.6 Add test executables to CMakeLists.txt for all 5 repository tests
