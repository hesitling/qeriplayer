## ADDED Requirements

### Requirement: Record play event
The system SHALL provide a method to record that a song was played. The method SHALL insert a row into `play_history` with the song ID and current timestamp. It SHALL also update the `last_played_at` timestamp on the corresponding `songs_cache` row.

#### Scenario: Record a play
- **WHEN** `record(songId)` is called
- **THEN** a new row SHALL be inserted into `play_history` with the given `song_id` and `played_at` set to the current timestamp, and the song's `last_played_at` in `songs_cache` SHALL be updated

### Requirement: Recent play history
The system SHALL provide a method to retrieve recently played songs, ordered by `played_at` descending. The method SHALL join `play_history` with `songs_cache` to return full `Song` objects. Each song SHALL appear at most once (most recent play wins).

#### Scenario: Get recent history
- **WHEN** `recent(10)` is called and 15 distinct songs have been played
- **THEN** the 10 most recently played distinct songs SHALL be returned in descending order of their most recent play time

#### Scenario: History with limit larger than entries
- **WHEN** `recent(100)` is called and only 5 songs have been played
- **THEN** exactly 5 songs SHALL be returned

### Requirement: Clear play history
The system SHALL provide a method to delete all entries from `play_history`. The `songs_cache` table SHALL NOT be affected.

#### Scenario: Clear history
- **WHEN** `clear()` is called and 100 play history entries exist
- **THEN** the `play_history` table SHALL be empty and `songs_cache` SHALL be unchanged

### Requirement: Remove specific history entries
The system SHALL provide a method to remove play history entries for specific song IDs.

#### Scenario: Remove songs from history
- **WHEN** `remove({"song1", "song2"})` is called
- **THEN** all `play_history` rows with `song_id` in the given list SHALL be removed

### Requirement: Play count
The system SHALL provide a method to count how many times a specific song has been played.

#### Scenario: Count plays for a song
- **WHEN** `playCount(songId)` is called and the song has been played 5 times
- **THEN** the result SHALL be 5

#### Scenario: Count plays for unplayed song
- **WHEN** `playCount(songId)` is called for a song never played
- **THEN** the result SHALL be 0
