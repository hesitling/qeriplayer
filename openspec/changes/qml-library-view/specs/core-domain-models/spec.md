## MODIFIED Requirements

### Requirement: PlaylistSummary model
The system SHALL provide a `PlaylistSummary` struct containing: `id` (QString), `name` (QString), `coverUrl` (QUrl), `playCount` (qint64), `trackCount` (int). An `AlbumSummary` struct SHALL contain: `id` (QString), `name` (QString), `coverUrl` (QUrl), `size` (int). A `BiliPlaylist` struct SHALL contain: `mediaId` (qint64), `fid` (qint64), `mid` (qint64), `title` (QString), `count` (int), `coverUrl` (QUrl), `kind` (BiliPlaylistKind), `subtitle` (QString). Both `PlaylistSummary` and `AlbumSummary` SHALL be QML-readable via `Q_GADGET` and `Q_PROPERTY` accessors for their fields.

#### Scenario: Default PlaylistSummary construction
- **WHEN** a PlaylistSummary is default-constructed
- **THEN** `playCount` SHALL be 0 and `trackCount` SHALL be 0

#### Scenario: Default AlbumSummary construction
- **WHEN** an AlbumSummary is default-constructed
- **THEN** `size` SHALL be 0

#### Scenario: QML property access for playlist summary
- **WHEN** QML receives a `PlaylistSummary` instance via `QVariant`
- **THEN** it SHALL be able to read `.name`, `.coverUrl`, and `.trackCount`

#### Scenario: QML property access for album summary
- **WHEN** QML receives an `AlbumSummary` instance via `QVariant`
- **THEN** it SHALL be able to read `.name`, `.coverUrl`, and `.size`
