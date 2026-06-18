## Context

QeriPlayer Qt has completed Phase 1 (foundation) and Phase 2 (API clients + repositories). The domain layer includes `PlaybackState`, `RepeatMode`, `Song`, `SongUrlResult`, and `PersistedPlayerState`. The `IPlayerStateRepository` persists queue/position. The `IMusicPlatformPlugin` resolves playback URLs. No audio playback code exists yet — `src/player/` is empty.

The playback engine sits between URL resolution (API layer) and the future UI/ViewModel layers. It must support multiple audio backends (Qt Multimedia default, mpv optional) and handle queue management, shuffle, repeat, and state persistence.

## Goals / Non-Goals

**Goals:**
- Abstract backend interface (`IPlayerBackend`) supporting Qt Multimedia and future mpv
- Default `QtMultimediaBackend` with zero external dependencies
- Queue with repeat modes, shuffle, and navigation
- `PlaybackController` orchestrating backend + queue + URL resolution + persistence
- Pre-resolve URLs when songs enter the queue
- Backend selection: auto-detect (prefer mpv if available) + settings toggle

**Non-Goals:**
- mpv backend implementation (docs updated, design deferred)
- UI/ViewModel layers (Phase 4–5)
- Gapless playback (best-effort only; Qt Multimedia limitation)
- Equalizer, audio effects, video

## Decisions

### 1. Backend abstraction: `IPlayerBackend` as QObject with signals

**Choice**: Abstract class inheriting `QObject`, using Qt signals for state notifications.

**Alternatives considered**:
- Pure virtual interface + callback registration → more boilerplate, no Qt property binding
- `QAbstractMediaPlayer` subclass → not extensible to mpv's different model

**Rationale**: Qt signals integrate naturally with ViewModel property binding. `QObject` inheritance enables `connect()` from any consumer. The interface mirrors `QMediaPlayer`'s shape but abstracts it.

### 2. Qt Multimedia backend: `QMediaPlayer` + `QAudioOutput`

**Choice**: Use Qt6's `QMediaPlayer` (FFmpeg backend on most systems) with `QAudioOutput` for volume.

**Alternatives considered**:
- Direct FFmpeg/libav integration → massive complexity, reinventing Qt Multimedia
- GStreamer direct → Linux-only, no cross-platform

**Rationale**: `QMediaPlayer` is already bundled with Qt6, requires no extra dependencies, and handles most audio codecs via FFmpeg. `QAudioOutput` provides device selection and volume control.

### 3. PlayQueue: separate shuffle index array

**Choice**: Maintain `m_shuffledOrder: QVector<int>` as a permutation of indices. Shuffle via Fisher-Yates on the index array, not in-place on the song list.

**Alternatives considered**:
- In-place Fisher-Yates on songs → destroys original order, can't unshuffle
- Random next with history → complex, no way to "go back" reliably

**Rationale**: Preserving original order allows clean unshuffle. Navigation in shuffle mode walks the index array. `m_shuffleCursor` tracks position in the shuffled sequence.

### 4. Pre-resolve: background coroutine with cache

**Choice**: When `PlayQueue::addSong()` is called, `PlaybackController` spawns a background `QCoro::Task` to resolve the URL via `IMusicPlatformPlugin::getSongUrl()`. Resolved URLs cached in `QHash<QString, QString>` with TTL.

**Alternatives considered**:
- Resolve on play → adds latency at playback time
- Resolve all at once → unnecessary network calls for large queues

**Rationale**: Pre-resolving eliminates playback latency for the common case (playing next song in queue). Cache with TTL handles URL expiry. Failed resolutions fall back to on-demand resolution at play time.

### 5. Backend selection: `BackendFactory` + settings

**Choice**: `BackendFactory::createBackend(type)` returns `std::unique_ptr<IPlayerBackend>`. Type stored in `SettingsRepository` as `player/backend` with values `"auto"`, `"qt"`, `"mpv"`. Auto-detect checks for libmpv availability at runtime.

**Alternatives considered**:
- Compile-time only selection → no runtime flexibility
- Plugin system → overengineered for 2 backends

**Rationale**: Runtime selection with auto-detect gives the best UX. Settings toggle lets users override. Factory pattern keeps `PlaybackController` backend-agnostic.

### 6. mpv backend: deferred but documented

**Choice**: Update docs to reference mpv instead of VLC. Leave mpv backend design blank in this change.

**Rationale**: mpv is a better fit than VLC (cleaner C API, native gapless, smaller footprint). But implementing it now adds scope without value — Qt Multimedia is sufficient for initial playback. The `IPlayerBackend` interface is designed to accommodate mpv's command-based model.

## Risks / Trade-offs

**[Qt Multimedia codec gaps]** → Qt6 uses FFmpeg by default, which covers most formats. For edge cases (e.g., some Bilibili audio formats), mpv backend will be the fallback. Mitigation: surface codec errors clearly so users can switch backends.

**[No true gapless with Qt Multimedia]** → `QMediaPlayer` has a small gap between tracks. Mitigation: best-effort (immediate load+play of next song). Users who need gapless can switch to mpv later.

**[Pre-resolve network overhead]** → Resolving URLs for every added song may cause unnecessary API calls. Mitigation: TTL-based cache, skip resolution if URL already cached, log failures without blocking.

**[mpv event loop threading]** → Future mpv backend will need careful thread management. Mitigation: `IPlayerBackend` interface is thread-agnostic; mpv implementation can use dedicated QThread or QTimer polling.
