## Why

QeriPlayer Qt can search and resolve playback URLs from NetEase, but has no audio playback. Phase 3 adds the playback engine — the core user-facing feature that turns URL resolution into actual music.

## What Changes

- Add `IPlayerBackend` abstract interface for audio backends
- Add `QtMultimediaBackend` implementing `IPlayerBackend` via Qt6's `QMediaPlayer` + `QAudioOutput`
- Add `PlayQueue` with ordered song list, repeat modes (off/one/all), and shuffle (Fisher-Yates)
- Add `PlaybackController` orchestrating backend, queue, URL resolution, and state persistence
- Add `AudioOutput` for volume/mute control with settings persistence
- Add backend selection with auto-detect (prefer mpv if available) and settings toggle
- Add pre-resolve: background URL resolution when songs are added to queue
- Add queue/position persistence via existing `IPlayerStateRepository`
- Update docs to replace VLC references with mpv (mpv backend design deferred to future change)
- Add `Qt6::Multimedia` to CMake dependencies

## Non-goals

- mpv backend implementation (design deferred; docs updated to reference mpv instead of VLC)
- UI layer (Phase 5)
- Service/ViewModel layer (Phase 4)
- Gapless playback beyond best-effort (Qt Multimedia limitation)
- Audio equalizer or effects
- Video playback

## Capabilities

### New Capabilities
- `player-backend`: Abstract `IPlayerBackend` interface defining the contract for audio backends (load, play, pause, stop, seek, volume, state signals)
- `qt-multimedia-backend`: Qt6 `QMediaPlayer`-based implementation of `IPlayerBackend`; default backend with zero external dependencies
- `play-queue`: Ordered song queue with repeat modes, shuffle, navigation (next/prev), and persistence
- `playback-controller`: High-level orchestrator coordinating backend, queue, URL resolution, and state persistence

### Modified Capabilities

(none)

## Impact

- **New module**: `src/player/` — all playback code lives here
- **Dependencies**: `Qt6::Multimedia` added to CMake (already bundled with Qt6)
- **Affected layers**: repo (player-state-repository usage), core (network for URL resolution), domain (enums already exist)
- **Settings**: new `player/backend` key for backend selection
