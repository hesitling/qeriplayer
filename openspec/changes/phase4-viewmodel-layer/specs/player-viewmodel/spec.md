## ADDED Requirements

### Requirement: PlayerViewModel wraps PlaybackController
The system SHALL define `PlayerViewModel` as a `QObject` that exposes `PlaybackController` state via `Q_PROPERTY` and delegates user actions via `Q_INVOKABLE` methods.

#### Scenario: PlayerViewModel is QML-accessible
- **WHEN** QML binds to `playerViewModel.isPlaying`
- **THEN** it SHALL reflect the current playback state from `PlaybackController`

### Requirement: Current song property
`PlayerViewModel` SHALL expose `currentSong` as a read-only `Q_PROPERTY` (type: `Song`, notify: `currentSongChanged`). It SHALL update when `PlaybackController::currentSongChanged` is emitted.

#### Scenario: Song change propagated
- **WHEN** `PlaybackController` emits `currentSongChanged` with a new song
- **THEN** `PlayerViewModel::currentSong` SHALL reflect the new song and emit `currentSongChanged`

### Requirement: Playback state properties
`PlayerViewModel` SHALL expose:
- `playbackState` (PlaybackState, read-only, notify: `playbackStateChanged`)
- `isPlaying` (bool, read-only, derived from `playbackState == Playing`)
- `isPaused` (bool, read-only, derived from `playbackState == Paused`)
- `isLoading` (bool, read-only, derived from `playbackState == Loading`)

#### Scenario: isPlaying reflects state
- **WHEN** `PlaybackController` emits `playbackStateChanged(Playing)`
- **THEN** `isPlaying` SHALL be `true`, `isPaused` SHALL be `false`, `isLoading` SHALL be `false`

### Requirement: Position and duration properties
`PlayerViewModel` SHALL expose `positionMs` (qint64, read-only, notify: `positionChanged`) and `durationMs` (qint64, read-only, notify: `durationChanged`). They SHALL update from corresponding `PlaybackController` signals.

#### Scenario: Position updated during playback
- **WHEN** `PlaybackController` emits `positionChanged(5000)`
- **THEN** `positionMs` SHALL be 5000

### Requirement: Volume control
`PlayerViewModel` SHALL expose `volume` (double, read/write, range 0.0–1.0, notify: `volumeChanged`). The setter SHALL call `PlaybackController::setVolume()`.

#### Scenario: Volume set from QML
- **WHEN** QML sets `playerViewModel.volume = 0.5`
- **THEN** `PlaybackController::setVolume(0.5)` SHALL be called and `volumeChanged` SHALL be emitted

### Requirement: Mute control
`PlayerViewModel` SHALL expose `isMuted` (bool, read/write, notify: `mutedChanged`). `Q_INVOKABLE toggleMute()` SHALL toggle the mute state via `PlaybackController::setMuted()`.

#### Scenario: Toggle mute
- **WHEN** `toggleMute()` is called and `isMuted` is `false`
- **THEN** `PlaybackController::setMuted(true)` SHALL be called

### Requirement: Repeat mode control
`PlayerViewModel` SHALL expose `repeatMode` (RepeatMode, read/write, notify: `repeatModeChanged`). `Q_INVOKABLE cycleRepeatMode()` SHALL cycle Off → One → All → Off.

#### Scenario: Cycle repeat mode
- **WHEN** `cycleRepeatMode()` is called and `repeatMode` is `One`
- **THEN** `repeatMode` SHALL become `All` and `repeatModeChanged` SHALL be emitted

### Requirement: Shuffle control
`PlayerViewModel` SHALL expose `isShuffleEnabled` (bool, read/write, notify: `shuffleChanged`). `Q_INVOKABLE toggleShuffle()` SHALL toggle shuffle via `PlayQueue::setShuffleEnabled()`.

#### Scenario: Toggle shuffle
- **WHEN** `toggleShuffle()` is called and `isShuffleEnabled` is `false`
- **THEN** shuffle SHALL be enabled and `shuffleChanged(true)` SHALL be emitted

### Requirement: Playback control Q_INVOKABLEs
`PlayerViewModel` SHALL expose:
- `play(const Song &song)` → `QCoro::Task<void>` (delegates to `PlaybackController::play()`)
- `pause()` → void
- `resume()` → void
- `stop()` → void
- `seek(qint64 positionMs)` → void
- `next()` → void (advances queue)
- `prev()` → void (goes to previous in queue)

#### Scenario: play delegates to PlaybackController
- **WHEN** `play(song)` is called
- **THEN** `PlaybackController::play(song)` SHALL be invoked

### Requirement: Queue management
`PlayerViewModel` SHALL expose:
- `queue` as a `SongListModel*` (read-only, notify: `queueChanged`)
- `Q_INVOKABLE addToQueue(const Song &song)`
- `Q_INVOKABLE removeFromQueue(int index)`
- `Q_INVOKABLE moveInQueue(int from, int to)`
- `Q_INVOKABLE clearQueue()`

#### Scenario: addToQueue appends song
- **WHEN** `addToQueue(song)` is called
- **THEN** the song SHALL be appended to `PlayQueue` and `queueChanged` SHALL be emitted

### Requirement: Play history recording
When the current song changes, `PlayerViewModel` SHALL call `IPlayHistoryRepository::record(song.id)` to track play history.

#### Scenario: History recorded on song change
- **WHEN** `PlaybackController` emits `currentSongChanged` with a song having `id == "abc"`
- **THEN** `IPlayHistoryRepository::record("abc")` SHALL be called

### Requirement: Error state
`PlayerViewModel` SHALL expose `hasError` (bool, notify: `errorChanged`) and `error` (ViewModelError, notify: `errorChanged`). Errors from `PlaybackController::errorOccurred` SHALL be mapped to `ViewModelError`.

#### Scenario: Playback error surfaced
- **WHEN** `PlaybackController` emits `errorOccurred("Network timeout")`
- **THEN** `hasError` SHALL be `true` and `error.message` SHALL be "Network timeout"
