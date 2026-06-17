## ADDED Requirements

### Requirement: Queue song management
The `PlayQueue` SHALL maintain an ordered list of `Song` objects. It SHALL provide `setSongs(QVector<Song>)`, `addSong(Song)`, `removeAt(int)`, `moveSong(int from, int to)`, and `clear()`.

#### Scenario: Set songs
- **WHEN** `setSongs()` is called with a list of 10 songs
- **THEN** the queue contains exactly those 10 songs in order, and `currentSong()` returns the first song

#### Scenario: Add song
- **WHEN** `addSong()` is called with a new song
- **THEN** the song is appended to the end of the queue

#### Scenario: Remove song at index
- **WHEN** `removeAt(3)` is called on a queue with 5 songs
- **THEN** the song at index 3 is removed and the queue has 4 songs

#### Scenario: Remove current song
- **WHEN** `removeAt()` is called with the current song's index
- **THEN** the current index advances to the next song (or stays at end if last)

### Requirement: Queue navigation
The `PlayQueue` SHALL provide `next()` and `prev()` returning `std::optional<Song>`. `next()` advances to the next song considering repeat and shuffle modes. `prev()` moves to the previous song.

#### Scenario: Next song in normal mode
- **WHEN** `next()` is called with repeat off and more songs remain
- **THEN** the current index increments by 1 and the next song is returned

#### Scenario: Next at end with repeat off
- **WHEN** `next()` is called at the last song with repeat off
- **THEN** `std::nullopt` is returned and the current index does not change

#### Scenario: Next at end with repeat all
- **WHEN** `next()` is called at the last song with repeat all
- **THEN** the current index wraps to 0 and the first song is returned

#### Scenario: Next with repeat one
- **WHEN** `next()` is called with repeat one
- **THEN** the current song is returned again without advancing

#### Scenario: Previous song
- **WHEN** `prev()` is called with current index > 0
- **THEN** the current index decrements by 1 and the previous song is returned

#### Scenario: Previous at start
- **WHEN** `prev()` is called with current index == 0
- **THEN** `std::nullopt` is returned (no wrap-around)

### Requirement: Current song tracking
The `PlayQueue` SHALL provide `currentSong()` returning `std::optional<Song>` and `currentIndex()`. It SHALL provide `setCurrentIndex(int)` to jump to a specific position.

#### Scenario: Query current song
- **WHEN** the queue has songs and `currentIndex()` is 2
- **THEN** `currentSong()` returns the song at index 2

#### Scenario: Jump to index
- **WHEN** `setCurrentIndex(5)` is called on a queue with 10 songs
- **THEN** `currentIndex()` returns 5 and `currentSong()` returns the song at index 5

### Requirement: Repeat mode
The `PlayQueue` SHALL provide `setRepeatMode(RepeatMode)` and `repeatMode()`. It SHALL use the existing `RepeatMode` enum (`Off`, `One`, `All`).

#### Scenario: Set repeat mode
- **WHEN** `setRepeatMode(RepeatMode::All)` is called
- **THEN** subsequent `next()` calls wrap around at the end of the queue

### Requirement: Shuffle mode
The `PlayQueue` SHALL provide `setShuffleEnabled(bool)` and `isShuffleEnabled()`. When shuffle is enabled, `next()` SHALL traverse songs in a randomized order using a Fisher-Yates shuffled index array. The original song order MUST be preserved.

#### Scenario: Enable shuffle
- **WHEN** `setShuffleEnabled(true)` is called on a queue with 10 songs
- **THEN** subsequent `next()` calls traverse songs in a randomized order

#### Scenario: Disable shuffle
- **WHEN** `setShuffleEnabled(false)` is called after shuffling
- **THEN** `next()` resumes normal sequential order from the current song

#### Scenario: Shuffle preserves original order
- **WHEN** shuffle is enabled then disabled
- **THEN** the underlying song list order is unchanged

### Requirement: Queue persistence
The `PlayQueue` SHALL provide `toPersistedState()` returning `PersistedPlayerState` and `loadFromState(PersistedPlayerState)` to restore queue state. `PersistedPlayerState` includes `playlist`, `currentIndex`, `repeatMode`, and `shuffleEnabled`.

#### Scenario: Save queue state
- **WHEN** `toPersistedState()` is called
- **THEN** the returned `PersistedPlayerState` contains the current song list, index, repeat mode, and shuffle state

#### Scenario: Restore queue state
- **WHEN** `loadFromState()` is called with a valid `PersistedPlayerState`
- **THEN** the queue restores the song list, index, repeat mode, and shuffle state

### Requirement: Queue signals
The `PlayQueue` SHALL emit `currentChanged()` when the current song changes, `queueChanged()` when the song list is modified, and `shuffleChanged()` / `repeatChanged()` on mode changes.

#### Scenario: Signal on next
- **WHEN** `next()` advances to a new song
- **THEN** `currentChanged()` is emitted

#### Scenario: Signal on add song
- **WHEN** `addSong()` appends a song
- **THEN** `queueChanged()` is emitted
