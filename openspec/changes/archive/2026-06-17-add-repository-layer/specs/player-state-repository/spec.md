## ADDED Requirements

### Requirement: Save player state
The system SHALL provide a method to persist the current player state. The state SHALL be stored in a singleton `player_state` table (only one row ever exists). The playlist (`QVector<Song>`) SHALL be serialized as JSON. The save operation SHALL use `INSERT OR REPLACE` to always update the single row.

#### Scenario: Save player state
- **WHEN** `save(state)` is called with a `PersistedPlayerState` containing 10 songs, index 3, position 45000ms, and repeat mode All
- **THEN** the `player_state` table SHALL have exactly one row with the serialized playlist, `current_index=3`, `position_ms=45000`, and `repeat_mode=2`

#### Scenario: Save overwrites previous state
- **WHEN** `save(state)` is called twice with different states
- **THEN** the `player_state` table SHALL still have exactly one row with the second state's values

### Requirement: Load player state
The system SHALL provide a method to load the persisted player state. The playlist JSON SHALL be deserialized back into `QVector<Song>`. If no state has been saved, an empty optional SHALL be returned.

#### Scenario: Load saved state
- **WHEN** `load()` is called after a `save()` with 5 songs
- **THEN** the returned `PersistedPlayerState` SHALL have 5 songs and all fields matching the saved state

#### Scenario: Load with no saved state
- **WHEN** `load()` is called on a fresh database
- **THEN** an empty optional SHALL be returned

### Requirement: Clear player state
The system SHALL provide a method to delete the persisted player state.

#### Scenario: Clear state
- **WHEN** `clear()` is called after a `save()`
- **THEN** the `player_state` table SHALL be empty and subsequent `load()` SHALL return an empty optional

### Requirement: Player state field mapping
The system SHALL correctly map between `PersistedPlayerState` fields and database columns:
- `playlist` ↔ `playlist_json` (JSON serialized `QVector<Song>`)
- `currentIndex` ↔ `current_index`
- `mediaUrl` ↔ `media_url`
- `positionMs` ↔ `position_ms`
- `shouldResumePlayback` ↔ `should_resume` (0/1 boolean)
- `repeatMode` ↔ `repeat_mode` (enum integer)
- `shuffleEnabled` ↔ `shuffle_enabled` (0/1 boolean)

#### Scenario: Round-trip player state through database
- **WHEN** a `PersistedPlayerState` with all fields populated is saved and then loaded
- **THEN** the loaded state SHALL be equal to the original in all fields
