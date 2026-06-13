## ADDED Requirements

### Requirement: Song model
The system SHALL provide a `Song` struct containing: `id` (QString), `title` (QString), `artist` (QString), `album` (QString), `albumId` (QString), `duration` (qint64, milliseconds), `coverUrl` (QUrl), `playbackUrl` (QUrl), `platform` (MusicPlatform enum), and `extra` (QVariantMap for platform-specific fields). All fields SHALL be default-constructible and copyable.

#### Scenario: Construct a Song with all fields
- **WHEN** a Song is constructed with id="1", title="Test", artist="Artist", album="Album", duration=180000
- **THEN** the Song SHALL hold all provided values and `platform` SHALL default to `MusicPlatform::Unknown`

#### Scenario: Copy a Song
- **WHEN** a Song is copy-assigned to another Song
- **THEN** all fields in the destination SHALL equal the source

### Requirement: Album model
The system SHALL provide an `Album` struct containing: `id` (QString), `title` (QString), `artist` (QString), `artistId` (QString), `coverUrl` (QUrl), `songCount` (int), `publishDate` (QDate), and `platform` (MusicPlatform enum).

#### Scenario: Default Album construction
- **WHEN** an Album is default-constructed
- **THEN** `songCount` SHALL be 0 and `platform` SHALL be `MusicPlatform::Unknown`

### Requirement: Artist model
The system SHALL provide an `Artist` struct containing: `id` (QString), `name` (QString), `avatarUrl` (QUrl), `description` (QString), and `platform` (MusicPlatform enum).

#### Scenario: Construct an Artist
- **WHEN** an Artist is constructed with id="a1" and name="Some Artist"
- **THEN** the Artist SHALL hold the provided values

### Requirement: Playlist model
The system SHALL provide a `Playlist` struct containing: `id` (QString), `name` (QString), `description` (QString), `coverUrl` (QUrl), `songCount` (int), `owner` (QString), `platform` (MusicPlatform enum), and `songs` (QVector<Song>, optionally populated).

#### Scenario: Playlist with embedded songs
- **WHEN** a Playlist is constructed with 3 songs in the `songs` vector
- **THEN** `songCount` SHALL be 3 and `songs` SHALL contain all 3 entries

### Requirement: Lyrics model
The system SHALL provide a `Lyrics` struct containing: `rawText` (QString) and `lines` (QVector<LyricLine>). A `LyricLine` SHALL contain: `timestamp` (qint64, milliseconds from start) and `text` (QString). Lines SHALL be sorted by timestamp in ascending order.

#### Scenario: Parse lyrics from raw text
- **WHEN** a Lyrics object is constructed with lines [{0, "Hello"}, {5000, "World"}]
- **THEN** `lines[0].timestamp` SHALL be 0 and `lines[1].timestamp` SHALL be 5000

### Requirement: SearchResult model
The system SHALL provide a `SearchResult` struct containing: `songs` (QVector<Song>), `albums` (QVector<Album>), `artists` (QVector<Artist>), `playlists` (QVector<Playlist>), `totalCount` (int), and `hasMore` (bool).

#### Scenario: Empty search result
- **WHEN** a SearchResult is default-constructed
- **THEN** all vectors SHALL be empty, `totalCount` SHALL be 0, and `hasMore` SHALL be false

### Requirement: MusicPlatform enum
The system SHALL provide an enum `MusicPlatform` with values: `Unknown`, `NetEase`, `Bilibili`, `YouTube`, `QQMusic`. The enum SHALL be usable as a QHash key.

#### Scenario: Enum comparison
- **WHEN** two MusicPlatform values are compared
- **THEN** equal values SHALL compare equal and different values SHALL compare unequal

### Requirement: SearchType enum
The system SHALL provide an enum `SearchType` with values: `Song`, `Album`, `Artist`, `Playlist`, `All`.

#### Scenario: Default search type
- **WHEN** a SearchType is default-constructed
- **THEN** it SHALL equal `SearchType::All`

### Requirement: PlaybackState enum
The system SHALL provide an enum `PlaybackState` with values: `Stopped`, `Playing`, `Paused`, `Loading`, `Error`.

#### Scenario: State transitions
- **WHEN** a PlaybackState is set to `Playing`
- **THEN** it SHALL not equal `Stopped`

### Requirement: RepeatMode enum
The system SHALL provide an enum `RepeatMode` with values: `Off`, `One`, `All`.

#### Scenario: Repeat mode values
- **WHEN** RepeatMode is iterated
- **THEN** it SHALL have exactly 3 distinct values

### Requirement: AudioQuality enum
The system SHALL provide an enum `AudioQuality` with values: `Low`, `Standard`, `High`, `Lossless`.

#### Scenario: Quality ordering
- **WHEN** AudioQuality values are compared
- **THEN** `Lossless` SHALL be greater than `High`, `High` greater than `Standard`, and `Standard` greater than `Low`
