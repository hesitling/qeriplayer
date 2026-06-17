## ADDED Requirements

### Requirement: QAbstractListModel for Song
The system SHALL define `SongListModel` as a `QAbstractListModel` subclass that wraps `QVector<Song>` and exposes song properties via Qt model roles.

#### Scenario: Model exposes Song roles
- **WHEN** QML accesses `model.name`, `model.artist`, `model.album`, `model.durationMs`, `model.coverUrl`, `model.platform`
- **THEN** each role SHALL return the corresponding `Song` field value

### Requirement: Role enumeration
The model SHALL define roles: `IdRole`, `NameRole`, `ArtistRole`, `AlbumRole`, `DurationMsRole`, `CoverUrlRole`, `PlatformRole`, `IsPlayingRole`. The `roleNames()` method SHALL map each role to a camelCase QByteArray name.

#### Scenario: roleNames returns correct mapping
- **WHEN** `roleNames()` is called
- **THEN** it SHALL return `{IdRole: "id", NameRole: "name", ArtistRole: "artist", AlbumRole: "album", DurationMsRole: "durationMs", CoverUrlRole: "coverUrl", PlatformRole: "platform", IsPlayingRole: "isPlaying"}`

### Requirement: Data management — setSongs
`setSongs(const QVector<Song> &songs)` SHALL replace all songs in the model. It SHALL call `beginResetModel()` before and `endResetModel()` after. It SHALL emit `countChanged()` if the count changed.

#### Scenario: setSongs replaces data
- **WHEN** `setSongs` is called with 5 songs on an empty model
- **THEN** `rowCount()` SHALL return 5, `count()` SHALL return 5, and `countChanged()` SHALL be emitted

### Requirement: Data management — appendSongs
`appendSongs(const QVector<Song> &songs)` SHALL append songs to the end of the model using `beginInsertRows`/`endInsertRows`.

#### Scenario: appendSongs adds to existing data
- **WHEN** `appendSongs` is called with 3 songs on a model with 5 songs
- **THEN** `rowCount()` SHALL return 8

### Requirement: Data management — clear
`clear()` SHALL remove all songs using `beginResetModel()`/`endResetModel()`.

#### Scenario: clear empties the model
- **WHEN** `clear()` is called on a model with 10 songs
- **THEN** `rowCount()` SHALL return 0 and `count()` SHALL return 0

### Requirement: songAt accessor
`songAt(int index)` SHALL return the `Song` at the given index. It SHALL return a default-constructed `Song` if the index is out of bounds.

#### Scenario: songAt returns correct song
- **WHEN** `songAt(2)` is called on a model with songs ["A", "B", "C", "D"]
- **THEN** the returned song SHALL have `name == "C"`

### Requirement: Playing index highlighting
`setPlayingIndex(int index)` SHALL update the internal playing index and emit `dataChanged` for the previously playing and newly playing rows on the `IsPlayingRole`.

#### Scenario: Playing index changes
- **WHEN** `setPlayingIndex(3)` is called when the previous playing index was 1
- **THEN** `dataChanged` SHALL be emitted for rows 1 and 3 with `IsPlayingRole`

### Requirement: count property
The model SHALL expose a `count` `Q_PROPERTY` (read-only, notify: `countChanged`). It SHALL return `rowCount()`.

#### Scenario: count reflects row count
- **WHEN** the model has 7 songs
- **THEN** `count` SHALL return 7
