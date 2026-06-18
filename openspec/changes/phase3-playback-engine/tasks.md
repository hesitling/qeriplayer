## 1. Setup & CMake

- [x] 1.1 Add `Qt6::Multimedia` to `find_package` and `target_link_libraries` in CMakeLists.txt
- [x] 1.2 Create `src/player/` directory structure: `src/player/`, `src/player/backends/`
- [x] 1.3 Update docs: replace VLC references with mpv in `docs/architecture/index.md` and `docs/TODO.md`

## 2. IPlayerBackend Interface

- [x] 2.1 Create `src/player/IPlayerBackend.h` with abstract interface (load, play, pause, stop, seek, state queries, volume, signals)
- [x] 2.2 Register `PlaybackState` signal parameter type with `Q_DECLARE_METATYPE` if not already done

## 3. QtMultimediaBackend

- [x] 3.1 Create `src/player/backends/QtMultimediaBackend.h` class declaration
- [x] 3.2 Implement `QtMultimediaBackend.cpp`: constructor wiring `QMediaPlayer` + `QAudioOutput`
- [x] 3.3 Implement `load()` with coroutine awaiting `LoadedMedia` status
- [x] 3.4 Implement `play()`, `pause()`, `stop()`, `seek()` delegating to `QMediaPlayer`
- [x] 3.5 Implement state mapping from `QMediaPlayer` states to `PlaybackState`
- [x] 3.6 Implement signal forwarding: position, duration, mediaFinished, error
- [x] 3.7 Implement volume/mute delegation to `QAudioOutput`

## 4. PlayQueue

- [x] 4.1 Create `src/player/PlayQueue.h` class declaration
- [x] 4.2 Implement `PlayQueue.cpp`: song management (setSongs, addSong, removeAt, moveSong, clear)
- [x] 4.3 Implement navigation: next(), prev(), currentSong(), setCurrentIndex()
- [x] 4.4 Implement repeat modes: Off, One, All
- [x] 4.5 Implement shuffle: Fisher-Yates index array, shuffle/unshuffle toggle
- [x] 4.6 Implement persistence: toPersistedState(), loadFromState()
- [x] 4.7 Implement signals: currentChanged, queueChanged, shuffleChanged, repeatChanged

## 5. PlaybackController

- [x] 5.1 Create `src/player/PlaybackController.h` class declaration
- [x] 5.2 Implement constructor: accept IPlayerBackend, IMusicPlatformPlugin, IPlayerStateRepository, ISettingsRepository
- [x] 5.3 Implement play(song): URL resolution → backend load → backend play
- [x] 5.4 Implement pause(), resume(), stop(), seek() delegation
- [x] 5.5 Implement auto-advance: onMediaFinished() → queue.next() → play()
- [x] 5.6 Implement controller signal propagation from backend
- [x] 5.7 Implement volume/mute delegation with settings persistence

## 6. Pre-resolve URLs

- [x] 6.1 Add URL cache (QHash<QString, QString>) with TTL to PlaybackController
- [x] 6.2 Implement background URL resolution coroutine triggered on song add
- [x] 6.3 Integrate cache lookup into play() before falling back to on-demand resolution

## 7. Backend Selection

- [x] 7.1 Create `src/player/BackendFactory.h/cpp` with createBackend(type) factory method
- [x] 7.2 Implement auto-detect logic (check libmpv availability at runtime)
- [x] 7.3 Register backend setting key `player/backend` in settings

## 8. Persistence

- [x] 8.1 Implement save: persist state to IPlayerStateRepository on song change, pause, stop
- [x] 8.2 Implement restore: load saved state on controller construction if shouldResumePlayback
- [x] 8.3 Implement debounced save for seek operations

## 9. Tests

- [x] 9.1 Create `tests/player/TestPlayQueue.cpp`: next/prev, shuffle, repeat modes, edge cases (empty queue, single song, remove current)
- [x] 9.2 Create `tests/player/TestPlaybackController.cpp`: mock IPlayerBackend, verify play/pause/auto-advance/URL resolution flow
- [x] 9.3 Create `tests/player/TestQtMultimediaBackend.cpp`: integration test with local audio file (load, play, pause, seek, volume)
- [x] 9.4 Add test targets to CMakeLists.txt

## 10. Integration

- [x] 10.1 Register PlaybackController and BackendFactory in ServiceLocator
- [x] 10.2 Initialize playback engine in QeriPlayerApplication startup
- [x] 10.3 Update TODO.md to mark Phase 3 tasks as complete
