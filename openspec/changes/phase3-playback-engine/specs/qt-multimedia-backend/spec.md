## ADDED Requirements

### Requirement: Qt Multimedia backend implementation
The `QtMultimediaBackend` class SHALL implement `IPlayerBackend` using Qt6's `QMediaPlayer` for audio decoding and `QAudioOutput` for volume/device control.

#### Scenario: Backend name
- **WHEN** `backendName()` is called on a `QtMultimediaBackend` instance
- **THEN** the return value is `"Qt Multimedia"`

### Requirement: Media loading via QMediaPlayer
The `QtMultimediaBackend` SHALL call `QMediaPlayer::setSource()` during `load()`. Loading SHALL be considered complete when `QMediaPlayer::mediaStatus()` transitions to `LoadedMedia` or `BufferedMedia`.

#### Scenario: Load completes successfully
- **WHEN** `load()` is called with a valid audio URL
- **THEN** the backend waits for `QMediaPlayer` to reach `LoadedMedia` status and completes the coroutine

#### Scenario: Load fails
- **WHEN** `load()` is called and `QMediaPlayer` reports `InvalidMedia`
- **THEN** the backend throws or emits `errorOccurred` with the player's error string

### Requirement: State mapping
The `QtMultimediaBackend` SHALL map `QMediaPlayer` states to `PlaybackState`:
- `QMediaPlayer::PlayingState` → `PlaybackState::Playing`
- `QMediaPlayer::PausedState` → `PlaybackState::Paused`
- `QMediaPlayer::StoppedState` → `PlaybackState::Stopped`
- `QMediaPlayer::LoadingState` → `PlaybackState::Loading`

#### Scenario: State signal on play
- **WHEN** `QMediaPlayer` transitions to `PlayingState`
- **THEN** `QtMultimediaBackend` emits `stateChanged(PlaybackState::Playing)`

### Requirement: Position and duration tracking
The `QtMultimediaBackend` SHALL forward `QMediaPlayer::positionChanged` and `durationChanged` signals. `positionMs()` and `durationMs()` SHALL return the current cached values from `QMediaPlayer`.

#### Scenario: Position updates during playback
- **WHEN** audio is playing
- **THEN** `positionChanged` is emitted periodically with the current position in milliseconds

### Requirement: Media finished detection
The `QtMultimediaBackend` SHALL detect `QMediaPlayer::mediaStatus() == EndOfMedia` and emit `mediaFinished()`.

#### Scenario: Track ends
- **WHEN** `QMediaPlayer` reaches `EndOfMedia`
- **THEN** the backend emits `mediaFinished()` and transitions to `PlaybackState::Stopped`

### Requirement: Volume delegation
The `QtMultimediaBackend` SHALL delegate `setVolume()` and `volume()` to `QAudioOutput`. `setMuted()` and `isMuted()` SHALL delegate to `QAudioOutput::setMuted()`.

#### Scenario: Volume change
- **WHEN** `setVolume(0.75)` is called
- **THEN** `QAudioOutput::setVolume()` is called with 0.75

### Requirement: Seek delegation
The `QtMultimediaBackend::seek()` SHALL call `QMediaPlayer::setPosition()` with the given millisecond value.

#### Scenario: Seek
- **WHEN** `seek(30000)` is called
- **THEN** `QMediaPlayer::setPosition(30000)` is called

### Requirement: Error forwarding
The `QtMultimediaBackend` SHALL forward `QMediaPlayer::errorOccurred` as the backend's `errorOccurred` signal with the error string.

#### Scenario: Playback error
- **WHEN** `QMediaPlayer` reports `errorOccurred`
- **THEN** `QtMultimediaBackend` emits `errorOccurred` with the corresponding error message
