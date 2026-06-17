## ADDED Requirements

### Requirement: Backend load media
The system SHALL define an `IPlayerBackend` interface with an asynchronous `load(const QUrl &url)` method that returns `QCoro::Task<void>`. Loading SHALL transition the backend to a ready state before playback can begin.

#### Scenario: Load valid audio URL
- **WHEN** `load()` is called with a valid audio URL
- **THEN** the backend prepares the media for playback and transitions to a ready state

#### Scenario: Load invalid URL
- **WHEN** `load()` is called with an invalid or unreachable URL
- **THEN** the backend emits `errorOccurred` with a descriptive message

### Requirement: Backend playback control
The `IPlayerBackend` interface SHALL provide `play()`, `pause()`, and `stop()` methods. `play()` SHALL start or resume playback. `pause()` SHALL suspend playback at the current position. `stop()` SHALL halt playback and reset position to zero.

#### Scenario: Play after load
- **WHEN** `play()` is called after a successful `load()`
- **THEN** the backend emits `stateChanged(PlaybackState::Playing)`

#### Scenario: Pause during playback
- **WHEN** `pause()` is called while playing
- **THEN** the backend emits `stateChanged(PlaybackState::Paused)` and preserves the current position

#### Scenario: Stop during playback
- **WHEN** `stop()` is called while playing
- **THEN** the backend emits `stateChanged(PlaybackState::Stopped)` and resets position to 0

### Requirement: Backend seek
The `IPlayerBackend` interface SHALL provide `seek(qint64 positionMs)` that moves playback to the specified position in milliseconds. The backend SHALL emit `positionChanged` after seeking.

#### Scenario: Seek within duration
- **WHEN** `seek()` is called with a position within the media duration
- **THEN** playback continues from the new position and `positionChanged` is emitted

#### Scenario: Seek beyond duration
- **WHEN** `seek()` is called with a position beyond the media duration
- **THEN** the backend clamps to the end of the media

### Requirement: Backend state queries
The `IPlayerBackend` interface SHALL provide synchronous accessors: `state()`, `positionMs()`, `durationMs()`, and `isSeekable()`. These SHALL return cached values without blocking.

#### Scenario: Query state during playback
- **WHEN** `state()` is called while audio is playing
- **THEN** the return value is `PlaybackState::Playing`

### Requirement: Backend volume control
The `IPlayerBackend` interface SHALL provide `setVolume(double normalized)` and `volume()` with a range of 0.0 to 1.0. It SHALL also provide `setMuted(bool)` and `isMuted()`.

#### Scenario: Set volume
- **WHEN** `setVolume(0.5)` is called
- **THEN** the audio output level is set to 50%

#### Scenario: Mute
- **WHEN** `setMuted(true)` is called
- **THEN** audio output is silenced without changing the volume level

### Requirement: Backend signals
The `IPlayerBackend` interface SHALL emit the following signals: `stateChanged(PlaybackState)`, `positionChanged(qint64)`, `durationChanged(qint64)`, `mediaFinished()`, `errorOccurred(const QString &)`.

#### Scenario: Media finishes naturally
- **WHEN** playback reaches the end of the media
- **THEN** the backend emits `mediaFinished()` and transitions to `Stopped`

#### Scenario: Backend encounters error
- **WHEN** an unrecoverable error occurs during playback
- **THEN** the backend emits `errorOccurred` with a human-readable message and transitions to `PlaybackState::Error`

### Requirement: Backend identification
The `IPlayerBackend` interface SHALL provide `backendName()` returning a human-readable identifier (e.g., `"Qt Multimedia"`, `"mpv"`).

#### Scenario: Query backend name
- **WHEN** `backendName()` is called
- **THEN** the return value identifies the backend implementation
