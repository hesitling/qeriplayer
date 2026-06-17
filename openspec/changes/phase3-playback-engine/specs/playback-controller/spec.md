## ADDED Requirements

### Requirement: Play a song
The `PlaybackController` SHALL provide `play(const Song &song)` as a `QCoro::Task<void>`. It SHALL resolve the song's playback URL (from cache or via `IMusicPlatformPlugin::getSongUrl()`), load it into the backend, and start playback.

#### Scenario: Play song with cached URL
- **WHEN** `play()` is called for a song whose URL is already cached
- **THEN** the cached URL is loaded into the backend without a network call

#### Scenario: Play song requiring URL resolution
- **WHEN** `play()` is called for a song with no cached URL
- **THEN** the controller calls `IMusicPlatformPlugin::getSongUrl()`, caches the result, and loads it into the backend

#### Scenario: Play song with direct mediaUri
- **WHEN** `play()` is called for a song that has a valid `mediaUri` set
- **THEN** the controller uses `mediaUri` directly without calling the platform plugin

#### Scenario: URL resolution fails
- **WHEN** `getSongUrl()` returns an error
- **THEN** the controller emits `errorOccurred` and does not modify playback state

### Requirement: Playback control delegation
The `PlaybackController` SHALL provide `pause()`, `resume()`, `stop()`, and `seek(qint64)` methods that delegate to the current `IPlayerBackend`.

#### Scenario: Pause
- **WHEN** `pause()` is called during playback
- **THEN** the backend's `pause()` is called

#### Scenario: Resume
- **WHEN** `resume()` is called while paused
- **THEN** the backend's `play()` is called

### Requirement: Queue integration
The `PlaybackController` SHALL own a `PlayQueue`. Calling `play(song)` SHALL set the song as the current queue item. The controller SHALL provide access to the queue for external manipulation.

#### Scenario: Play from queue
- **WHEN** `play()` is called for a song already in the queue
- **THEN** the queue's current index is set to that song's position

#### Scenario: Play song not in queue
- **WHEN** `play()` is called for a song not in the queue
- **THEN** the song is inserted at the current position + 1 and becomes current

### Requirement: Auto-advance on track finish
When the backend emits `mediaFinished()`, the `PlaybackController` SHALL call `m_queue.next()`. If a next song exists, it SHALL automatically `play()` it. If no next song exists (queue exhausted, repeat off), it SHALL emit `playbackFinished()`.

#### Scenario: Auto-advance to next song
- **WHEN** the current track finishes and repeat is not off or more songs remain
- **THEN** the controller automatically plays the next song from the queue

#### Scenario: Queue exhausted with repeat off
- **WHEN** the current track finishes, it is the last song, and repeat is off
- **THEN** the controller stops and emits `playbackFinished()`

### Requirement: Pre-resolve URLs
When a song is added to the queue, the `PlaybackController` SHALL schedule a background URL resolution via `IMusicPlatformPlugin::getSongUrl()`. Resolved URLs SHALL be cached in a `QHash<QString, QString>` with a configurable TTL (default 30 minutes).

#### Scenario: Pre-resolve on add
- **WHEN** a song is added to the queue via the controller
- **THEN** a background coroutine resolves the URL and caches it

#### Scenario: Cache hit
- **WHEN** a song's URL was resolved less than TTL ago
- **THEN** `play()` uses the cached URL without a network call

#### Scenario: Pre-resolve failure
- **WHEN** background URL resolution fails
- **THEN** the failure is logged and `play()` falls back to on-demand resolution

### Requirement: State persistence
The `PlaybackController` SHALL persist playback state to `IPlayerStateRepository` on song change, pause, and stop. It SHALL restore state on construction if `PersistedPlayerState::shouldResumePlayback` is true.

#### Scenario: Persist on pause
- **WHEN** playback is paused
- **THEN** the current queue, index, position, repeat mode, and shuffle state are saved to `IPlayerStateRepository`

#### Scenario: Restore on startup
- **WHEN** the controller is constructed and saved state exists with `shouldResumePlayback == true`
- **THEN** the queue is restored and playback resumes from the saved position

### Requirement: Backend selection
The `PlaybackController` SHALL accept an `IPlayerBackend` via constructor injection. A `BackendFactory` SHALL provide `createBackend(const QString &type)` returning `std::unique_ptr<IPlayerBackend>`. Supported types: `"qt"` (Qt Multimedia), `"mpv"` (future), `"auto"` (prefer mpv if available, fallback to Qt).

#### Scenario: Auto-detect with mpv available
- **WHEN** `BackendFactory::createBackend("auto")` is called and libmpv is available
- **THEN** an `MpvBackend` instance is returned

#### Scenario: Auto-detect without mpv
- **WHEN** `BackendFactory::createBackend("auto")` is called and libmpv is not available
- **THEN** a `QtMultimediaBackend` instance is returned

#### Scenario: Explicit Qt selection
- **WHEN** `BackendFactory::createBackend("qt")` is called
- **THEN** a `QtMultimediaBackend` instance is returned

### Requirement: Controller signals
The `PlaybackController` SHALL emit: `currentSongChanged(const Song &)`, `playbackStateChanged(PlaybackState)`, `positionChanged(qint64)`, `durationChanged(qint64)`, `errorOccurred(const QString &)`, and `playbackFinished()`.

#### Scenario: State change propagation
- **WHEN** the backend emits `stateChanged(Playing)`
- **THEN** the controller emits `playbackStateChanged(Playing)`

### Requirement: Volume control delegation
The `PlaybackController` SHALL provide `setVolume(double)`, `volume()`, `setMuted(bool)`, `isMuted()` that delegate to the current backend. Volume and mute state SHALL be persisted to `ISettingsRepository` on change.

#### Scenario: Volume persistence
- **WHEN** `setVolume(0.8)` is called
- **THEN** the volume is set on the backend and saved to `ISettingsRepository` as `player/volume`

#### Scenario: Restore volume on startup
- **WHEN** the controller is constructed
- **THEN** the saved volume and mute state are loaded from `ISettingsRepository` and applied to the backend
