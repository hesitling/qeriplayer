## ADDED Requirements

### Requirement: Song model
The system SHALL provide a `Song` struct containing:
- Core identity: `id` (QString), `name` (QString), `artist` (QString), `album` (QString), `albumId` (QString), `durationMs` (qint64, milliseconds), `coverUrl` (QUrl), `mediaUri` (QUrl), `platform` (MusicPlatform enum)
- Lyric matching: `matchedLyric` (QString), `matchedTranslatedLyric` (QString), `matchedLyricSource` (MusicPlatform), `matchedSongId` (QString), `userLyricOffsetMs` (qint64)
- User customizations: `customCoverUrl` (QString), `customName` (QString), `customArtist` (QString)
- Original values: `originalName` (QString), `originalArtist` (QString), `originalCoverUrl` (QString), `originalLyric` (QString), `originalTranslatedLyric` (QString)
- Local file support: `localFileName` (QString), `localFilePath` (QString)
- Platform identifiers: `channelId` (QString), `audioId` (QString), `subAudioId` (QString), `playlistContextId` (QString), `streamUrl` (QString)
- `extra` (QVariantMap for platform-specific fields)

Field names SHALL match Android NeriPlayer's `SongItem` model. All fields SHALL be default-constructible and copyable.

#### Scenario: Construct a Song with all fields
- **WHEN** a Song is constructed with id="1", name="Test", artist="Artist", album="Album", durationMs=180000
- **THEN** the Song SHALL hold all provided values and `platform` SHALL default to `MusicPlatform::Unknown`

#### Scenario: Copy a Song
- **WHEN** a Song is copy-assigned to another Song
- **THEN** all fields in the destination SHALL equal the source

### Requirement: Album model
The system SHALL provide an `Album` struct containing: `id` (QString), `name` (QString), `artist` (QString), `coverUrl` (QUrl), `size` (int, track count), and `platform` (MusicPlatform enum). Field names SHALL match Android NeriPlayer's `AlbumSummary` model.

#### Scenario: Default Album construction
- **WHEN** an Album is default-constructed
- **THEN** `size` SHALL be 0 and `platform` SHALL be `MusicPlatform::Unknown`

### Requirement: Artist model
The system SHALL provide an `Artist` struct containing: `id` (QString), `name` (QString), `avatarUrl` (QUrl), `description` (QString), and `platform` (MusicPlatform enum).

#### Scenario: Construct an Artist
- **WHEN** an Artist is constructed with id="a1" and name="Some Artist"
- **THEN** the Artist SHALL hold the provided values

### Requirement: Playlist model
The system SHALL provide a `Playlist` struct containing: `id` (QString), `name` (QString), `description` (QString), `coverUrl` (QUrl), `songCount` (int), `owner` (QString), `platform` (MusicPlatform enum), `songs` (QVector<Song>, optionally populated), `modifiedAt` (qint64, epoch milliseconds), and `customCoverUrl` (QString). Field names SHALL align with Android NeriPlayer's `LocalPlaylist` model.

#### Scenario: Playlist with embedded songs
- **WHEN** a Playlist is constructed with 3 songs in the `songs` vector
- **THEN** `songCount` SHALL be 3 and `songs` SHALL contain all 3 entries

### Requirement: Lyrics model
The system SHALL provide a `Lyrics` struct containing: `rawText` (QString) and `lines` (QVector<LyricLine>). A `LyricLine` SHALL contain: `startTimeMs` (qint64), `endTimeMs` (qint64), `text` (QString), and `words` (QVector<LyricWord>). A `LyricWord` SHALL contain: `text` (QString), `startTimeMs` (qint64), `endTimeMs` (qint64). Lines SHALL be sorted by `startTimeMs` in ascending order.

#### Scenario: Parse lyrics from raw text
- **WHEN** a Lyrics object is constructed with lines [{startTimeMs=0, endTimeMs=5000, text="Hello"}, {startTimeMs=5000, endTimeMs=10000, text="World"}]
- **THEN** `lines[0].startTimeMs` SHALL be 0 and `lines[1].startTimeMs` SHALL be 5000

#### Scenario: Word-level timing
- **WHEN** a LyricLine has words [{"Hello", 1000, 2000}, {"World", 2000, 3000}]
- **THEN** `words[0].startTimeMs` SHALL be 1000 and `words[1].endTimeMs` SHALL be 3000

### Requirement: PlaybackAudioSource enum
The system SHALL provide an enum `PlaybackAudioSource` with values: `Local`, `NetEase`, `Bilibili`, `YouTube`. Aligned with Android NeriPlayer's `PlaybackAudioSource`.

#### Scenario: Default audio source
- **WHEN** a PlaybackAudioSource is default-constructed
- **THEN** it SHALL equal `PlaybackAudioSource::Local` (enum value 0)

### Requirement: BiliPlaylistKind enum
The system SHALL provide an enum `BiliPlaylistKind` with values: `CreatedFavorite`, `CollectedFavorite`, `Collection`. Aligned with Android NeriPlayer's `BiliPlaylistKind`.

#### Scenario: BiliPlaylistKind distinct values
- **WHEN** BiliPlaylistKind values are compared
- **THEN** all three values SHALL be distinct

### Requirement: SongIdentity model
The system SHALL provide a `SongIdentity` struct containing: `id` (QString), `album` (QString), `mediaUri` (QString). It SHALL provide a `stableKey()` method returning a deterministic string for hashing and comparison. It SHALL support equality comparison operators. Aligned with Android NeriPlayer's `SongIdentity`.

#### Scenario: Stable key generation
- **WHEN** a SongIdentity with id="1", album="TestAlbum", mediaUri="http://example.com/song" has `stableKey()` called
- **THEN** the result SHALL be a non-empty deterministic string containing the id, album, and mediaUri

#### Scenario: Equality comparison
- **WHEN** two SongIdentity objects have the same id, album, and mediaUri
- **THEN** they SHALL compare equal

### Requirement: SongUrlResult model
The system SHALL provide a `SongUrlResult` struct with a `Status` enum containing: `Success`, `WaitingForAuthoritativeStream`, `RequiresLogin`, `Failure`. It SHALL contain: `status`, `url` (QString), `durationMs` (qint64), `mimeType` (QString), `noticeMessage` (QString), `expectedContentLength` (qint64), `audioInfo` (AudioInfo), `cacheKeyOverride` (QString). An `AudioInfo` struct SHALL contain: `qualityKey`, `qualityLabel`, `codecLabel`, `mimeType` (all QString), `bitrateKbps`, `sampleRateHz`, `bitDepth`, `channelCount` (all int). Aligned with Android NeriPlayer's `SongUrlResult` and `PlaybackAudioInfo`.

#### Scenario: Successful URL result
- **WHEN** a SongUrlResult is constructed with status=Success, url="http://example.com/stream", durationMs=180000
- **THEN** `status` SHALL be `Success` and `url` SHALL be "http://example.com/stream"

#### Scenario: Failure result
- **WHEN** a SongUrlResult is default-constructed
- **THEN** `status` SHALL be `Failure` and `url` SHALL be empty

### Requirement: PlaylistSummary model
The system SHALL provide a `PlaylistSummary` struct containing: `id` (QString), `name` (QString), `coverUrl` (QUrl), `playCount` (qint64), `trackCount` (int). An `AlbumSummary` struct SHALL contain: `id` (QString), `name` (QString), `coverUrl` (QUrl), `size` (int). A `BiliPlaylist` struct SHALL contain: `mediaId` (qint64), `fid` (qint64), `mid` (qint64), `title` (QString), `count` (int), `coverUrl` (QUrl), `kind` (BiliPlaylistKind), `subtitle` (QString). Aligned with Android NeriPlayer's summary models.

#### Scenario: Default PlaylistSummary construction
- **WHEN** a PlaylistSummary is default-constructed
- **THEN** `playCount` SHALL be 0 and `trackCount` SHALL be 0

#### Scenario: Default AlbumSummary construction
- **WHEN** an AlbumSummary is default-constructed
- **THEN** `size` SHALL be 0

#### Scenario: Default BiliPlaylist construction
- **WHEN** a BiliPlaylist is default-constructed
- **THEN** `kind` SHALL be `BiliPlaylistKind::CreatedFavorite` and `count` SHALL be 0

### Requirement: PersistedPlayerState model
The system SHALL provide a `PersistedPlayerState` struct containing: `playlist` (QVector<Song>), `currentIndex` (int), `mediaUrl` (QString), `positionMs` (qint64), `shouldResumePlayback` (bool), `repeatMode` (RepeatMode), `shuffleEnabled` (bool). Aligned with Android NeriPlayer's `PersistedState`.

#### Scenario: Default construction
- **WHEN** a PersistedPlayerState is default-constructed
- **THEN** `currentIndex` SHALL be 0, `positionMs` SHALL be 0, `shouldResumePlayback` SHALL be false, `repeatMode` SHALL be `RepeatMode::Off`, and `shuffleEnabled` SHALL be false

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
- **THEN** it SHALL equal `SearchType::Song` (enum value 0)

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

### Requirement: Listen Together domain models
The system SHALL provide Listen Together domain types aligned with Android NeriPlayer's `listentogether/` package:

- `ListenTogetherTrack` (wire format): `stableKey`, `channelId`, `audioId`, `subAudioId`, `playlistContextId`, `mediaUri`, `streamUrl`, `name`, `artist`, `album`, `durationMs`, `coverUrl`
- `ListenTogetherRoomState`: `roomId`, `version`, `schemaVersion`, `controllerUserUuid`, `settings`, `members`, `queue`, `currentIndex`, `track`, `playback`, `roomStatus`, etc.
- `ListenTogetherMember`: `userUuid`, `nickname`, `userId`, `role`, `joinedAt`
- `ListenTogetherPlaybackState`: `state`, `basePositionMs`, `baseTimestampMs`, `playbackRate`
- `ListenTogetherRoomSettings`: `allowMemberControl`, `autoPauseOnMemberChange`, `shareAudioLinks`
- `ListenTogetherEvent`: `type`, `eventId`, `clientTimeMs`, `positionMs`, `currentIndex`, `track`, `queue`, `roomSettings`, `shouldPlay`, `state`, `requestTrackStableKey`
- `ListenTogetherConnectionState` enum: `Disconnected`, `Connecting`, `Connected`
- `ListenTogetherSessionState`: client-side session with `baseUrl`, `roomId`, `userUuid`, `nickname`, `role`, `token`, `wsUrl`, `connectionState`, `lastError`, `expectedPositionMs`, `roomNotice`
- `ListenTogetherCause`: `userUuid`, `userId`, `nickname`, `eventId`, `type`

Constants namespace `ListenTogetherChannels`: `NETEASE`, `BILIBILI`, `YOUTUBE_MUSIC`, `LOCAL`
Constants namespace `ListenTogetherRoomStatuses`: `ACTIVE`, `CONTROLLER_OFFLINE`, `CLOSED`

Conversion helpers: `songToSong()`, `trackToSong()`, `buildStableTrackKey()`, `resolvedChannelId()`, `resolvedAudioId()`

#### Scenario: Stable key for NetEase track
- **WHEN** `buildStableTrackKey("netease", "12345")` is called
- **THEN** the result SHALL be "netease:12345"

#### Scenario: Stable key for Bilibili track with subAudioId
- **WHEN** `buildStableTrackKey("bilibili", "67890", "p1")` is called
- **THEN** the result SHALL be "bilibili:67890:p1"

#### Scenario: Song to track conversion
- **WHEN** a Song with channelId="netease", audioId="42" is converted via `songToTrack()`
- **THEN** the resulting ListenTogetherTrack SHALL have channelId="netease", audioId="42", stableKey="netease:42"

#### Scenario: Track to song conversion
- **WHEN** a ListenTogetherTrack with name="Converted", artist="Artist" is converted via `trackToSong()`
- **THEN** the resulting Song SHALL have name="Converted" and artist="Artist"

#### Scenario: Round-trip conversion
- **WHEN** a Song is converted to a ListenTogetherTrack and back to a Song
- **THEN** name, artist, album, durationMs, channelId, and audioId SHALL match the original
