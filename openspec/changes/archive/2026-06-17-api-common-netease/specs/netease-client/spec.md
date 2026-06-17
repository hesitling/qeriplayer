## ADDED Requirements

### Requirement: NeteaseClient construction
The system SHALL provide a `NeteaseClient` class that takes an `HttpClient*` and an optional `SecureStorage*` in its constructor. On construction, it SHALL attempt to load saved cookies from `SecureStorage`. It SHALL implement `IMusicPlatformPlugin`.

#### Scenario: Construct with HttpClient only
- **WHEN** a `NeteaseClient` is constructed with a valid `HttpClient*` and no `SecureStorage`
- **THEN** `isAuthenticated()` SHALL return false and all API calls SHALL work in anonymous mode

#### Scenario: Restore session from SecureStorage
- **WHEN** a `NeteaseClient` is constructed and `SecureStorage` contains valid NetEase cookies
- **THEN** `isAuthenticated()` SHALL return true without requiring a new login

### Requirement: API base URL configuration
`NeteaseClient` SHALL default its API base URL to `https://music.163.com/api`. It SHALL provide a `setBaseUrl(const QUrl &)` method to override the base URL for self-hosted instances.

#### Scenario: Default base URL
- **WHEN** a `NeteaseClient` is constructed without calling `setBaseUrl`
- **THEN** all requests SHALL go to `https://music.163.com/api/*`

#### Scenario: Custom base URL
- **WHEN** `setBaseUrl(QUrl("http://localhost:3000"))` is called
- **THEN** subsequent requests SHALL go to `http://localhost:3000/*`

### Requirement: Phone number login
`NeteaseClient` SHALL provide `login(phone, password)` returning `QCoro::Task<ApiResult<LoginResult>>`. On success, it SHALL persist cookies to `SecureStorage` and set `isAuthenticated()` to true.

#### Scenario: Successful phone login
- **WHEN** `login("13800138000", "correct_password")` is called and the API returns success
- **THEN** the result SHALL contain a `LoginResult` with `userId` populated, cookies SHALL be saved, and `isAuthenticated()` SHALL return true

#### Scenario: Failed phone login
- **WHEN** `login("13800138000", "wrong_password")` is called and the API returns 502
- **THEN** the result SHALL contain an `ApiError` with `isAuthError()` true, and `isAuthenticated()` SHALL remain false

### Requirement: Email login
`NeteaseClient` SHALL provide `loginByEmail(email, password)` returning `QCoro::Task<ApiResult<LoginResult>>`. Behavior SHALL match phone login (cookie persistence, auth state).

#### Scenario: Successful email login
- **WHEN** `loginByEmail("user@example.com", "password")` succeeds
- **THEN** the result SHALL contain a `LoginResult` and `isAuthenticated()` SHALL return true

### Requirement: QR code login
`NeteaseClient` SHALL provide `generateQrCode()` returning `QCoro::Task<ApiResult<QrCodeData>>` and `checkQrCodeStatus(key)` returning `QCoro::Task<ApiResult<LoginResult>>`. The QR key SHALL be usable for polling until the code expires or is scanned.

#### Scenario: Generate QR code
- **WHEN** `generateQrCode()` is called
- **THEN** the result SHALL contain a `QrCodeData` with a non-empty `key` and a valid `qrUrl`

#### Scenario: Check QR code status — pending
- **WHEN** `checkQrCodeStatus(key)` is called before the user scans
- **THEN** the result SHALL contain an `ApiError` indicating the code has not been scanned yet

#### Scenario: Check QR code status — success
- **WHEN** `checkQrCodeStatus(key)` is called after the user scans and confirms
- **THEN** the result SHALL contain a `LoginResult` and `isAuthenticated()` SHALL return true

### Requirement: Logout
`NeteaseClient` SHALL provide `logout()` returning `QCoro::Task<ApiResult<VoidResult>>`. On success, it SHALL clear stored cookies and set `isAuthenticated()` to false.

#### Scenario: Successful logout
- **WHEN** `logout()` is called while authenticated
- **THEN** cookies SHALL be cleared from `SecureStorage` and `isAuthenticated()` SHALL return false

### Requirement: Search songs
`NeteaseClient` SHALL provide `searchSongs(keyword, limit, offset)` returning `QCoro::Task<ApiResult<SearchResult>>`. The result SHALL contain a `songs` vector populated with `Song` objects parsed from the NetEase response.

#### Scenario: Search returns results
- **WHEN** `searchSongs("周杰伦", 30, 0)` is called and the API returns 30 songs
- **THEN** the result SHALL contain a `SearchResult` with `songs.size() == 30`, `totalCount` populated from the API response, and each song SHALL have `name`, `artist`, `durationMs` populated

#### Scenario: Search returns empty
- **WHEN** `searchSongs("nonexistent_query_xyz", 30, 0)` is called and the API returns 0 results
- **THEN** the result SHALL contain a `SearchResult` with `songs.isEmpty()` true and `hasMore` false

### Requirement: Search playlists
`NeteaseClient` SHALL provide `searchPlaylists(keyword, limit, offset)` returning `QCoro::Task<ApiResult<SearchResult>>`. The result SHALL contain a `playlists` vector.

#### Scenario: Search playlists returns results
- **WHEN** `searchPlaylists("华语流行", 20, 0)` is called
- **THEN** the result SHALL contain a `SearchResult` with `playlists` populated

### Requirement: Search albums and artists
`NeteaseClient` SHALL provide `searchAlbums(keyword, limit, offset)` and `searchArtists(keyword, limit, offset)` returning `QCoro::Task<ApiResult<SearchResult>>`.

#### Scenario: Search albums
- **WHEN** `searchAlbums("范特西", 10, 0)` is called
- **THEN** the result SHALL contain a `SearchResult` with `albums` populated

#### Scenario: Search artists
- **WHEN** `searchArtists("周杰伦", 10, 0)` is called
- **THEN** the result SHALL contain a `SearchResult` with `artists` populated

### Requirement: Hot searches
`NeteaseClient` SHALL provide `getHotSearches()` returning `QCoro::Task<ApiResult<QStringList>>`.

#### Scenario: Get hot searches
- **WHEN** `getHotSearches()` is called
- **THEN** the result SHALL contain a non-empty `QStringList` of trending search terms

### Requirement: Song detail
`NeteaseClient` SHALL provide `getSongDetail(songId)` returning `QCoro::Task<ApiResult<Song>>`. The returned `Song` SHALL have `platform` set to `MusicPlatform::NetEase`.

#### Scenario: Get song detail
- **WHEN** `getSongDetail("12345")` is called
- **THEN** the result SHALL contain a `Song` with `id`, `name`, `artist`, `album`, `durationMs`, `coverUrl` populated and `platform == MusicPlatform::NetEase`

#### Scenario: Invalid song ID
- **WHEN** `getSongDetail("invalid_id")` is called and the API returns an error
- **THEN** the result SHALL contain an `ApiError`

### Requirement: Song playback URL
`NeteaseClient` SHALL provide `getSongUrl(songId, quality)` returning `QCoro::Task<ApiResult<SongUrlResult>>`. The `quality` parameter SHALL default to `AudioQuality::High`. The returned `SongUrlResult` SHALL use the existing domain type (with `status`, `url`, `durationMs`, `mimeType`, `audioInfo`, etc.) — no new `PlaybackUrl` type.

#### Scenario: Get playback URL
- **WHEN** `getSongUrl("12345", AudioQuality::High)` is called
- **THEN** the result SHALL contain a `SongUrlResult` with `status == Success` and a non-empty `url`

#### Scenario: Lossless quality unavailable
- **WHEN** `getSongUrl("12345", AudioQuality::Lossless)` is called and lossless is not available
- **THEN** the API SHALL return a lower quality URL, and the `SongUrlResult` SHALL reflect the actual quality returned

#### Scenario: Song requires login
- **WHEN** `getSongUrl("12345")` is called for a VIP-only song while not authenticated
- **THEN** the result SHALL contain a `SongUrlResult` with `status == RequiresLogin`

### Requirement: Lyrics retrieval
`NeteaseClient` SHALL provide `getLyrics(songId)` returning `QCoro::Task<ApiResult<Lyrics>>`. The result SHALL contain timed lyric lines if available.

#### Scenario: Song has lyrics
- **WHEN** `getLyrics("12345")` is called and the song has lyrics
- **THEN** the result SHALL contain a `Lyrics` with `lines` populated and each line having a `timestamp` and `text`

#### Scenario: Song has no lyrics
- **WHEN** `getLyrics("12345")` is called and the song has no lyrics
- **THEN** the result SHALL contain a `Lyrics` with `lines` empty

### Requirement: Playlist detail
`NeteaseClient` SHALL provide `getPlaylistDetail(playlistId)` returning `QCoro::Task<ApiResult<Playlist>>`. The returned `Playlist` SHALL have its `songs` vector populated.

#### Scenario: Get playlist detail
- **WHEN** `getPlaylistDetail("666")` is called
- **THEN** the result SHALL contain a `Playlist` with `name`, `description`, `coverUrl`, `owner`, and `songs` populated

### Requirement: User playlists
`NeteaseClient` SHALL provide `getUserPlaylists(userId)` returning `QCoro::Task<ApiResult<QVector<Playlist>>>`.

#### Scenario: Get user playlists
- **WHEN** `getUserPlaylists("user123")` is called
- **THEN** the result SHALL contain a vector of `Playlist` objects owned by or followed by that user

### Requirement: Playlist CRUD
`NeteaseClient` SHALL provide:
- `createPlaylist(name, description)` → `QCoro::Task<ApiResult<Playlist>>`
- `deletePlaylist(playlistId)` → `QCoro::Task<ApiResult<VoidResult>>`
- `addSongToPlaylist(playlistId, songId)` → `QCoro::Task<ApiResult<VoidResult>>`
- `removeSongFromPlaylist(playlistId, songId)` → `QCoro::Task<ApiResult<VoidResult>>`

All playlist mutation methods SHALL require authentication.

#### Scenario: Create playlist
- **WHEN** `createPlaylist("My Playlist", "Description")` is called while authenticated
- **THEN** the result SHALL contain a `Playlist` with the given name and a server-assigned ID

#### Scenario: Add song to playlist
- **WHEN** `addSongToPlaylist("playlist_id", "song_id")` is called while authenticated
- **THEN** the operation SHALL succeed (VoidResult)

#### Scenario: Mutation without auth
- **WHEN** any playlist mutation is called while not authenticated
- **THEN** the result SHALL contain an `ApiError` with `isAuthError()` true

### Requirement: Recommendations
`NeteaseClient` SHALL provide:
- `getDailyRecommendations()` → `QCoro::Task<ApiResult<QVector<Song>>>`
- `getPersonalFM()` → `QCoro::Task<ApiResult<QVector<Song>>>`

These endpoints SHALL require authentication.

#### Scenario: Get daily recommendations
- **WHEN** `getDailyRecommendations()` is called while authenticated
- **THEN** the result SHALL contain a non-empty vector of `Song` objects

#### Scenario: Daily recommendations without auth
- **WHEN** `getDailyRecommendations()` is called while not authenticated
- **THEN** the result SHALL contain an `ApiError` with `isAuthError()` true

### Requirement: NeteaseCrypto — WeAPI encryption
`NeteaseCrypto` SHALL provide a `weapiEncrypt(plaintext)` static method using AES-128-CBC + RSA encryption matching the NetEase web client protocol. WeAPI encryption is one-way (client → server) — there is no client-side `weapiDecrypt`. Server responses arrive over HTTPS and require no client-side decryption.

#### Scenario: Encrypt matches known test vector
- **WHEN** `weapiEncrypt(data)` is called with a known plaintext and the AES random IV is fixed for testing
- **THEN** the AES-encrypted payload (before RSA wrapping) SHALL match the expected ciphertext for that plaintext+IV combination

#### Scenario: Encrypt produces different output each time
- **WHEN** `weapiEncrypt(data)` is called twice with the same input (normal mode, random IV)
- **THEN** the two outputs SHALL differ due to the random AES IV

### Requirement: NeteaseParser — JSON parsing
`NeteaseParser` SHALL provide static methods to parse NetEase API JSON responses into domain types. Each parser method SHALL take a `QJsonObject` (or `QJsonDocument`) and return the corresponding domain type. Parsing SHALL not throw — malformed JSON SHALL result in empty/default values with a logged warning via `Logger::get("api")`.

#### Scenario: Parse song from JSON
- **WHEN** `NeteaseParser::parseSong(json)` is called with valid song JSON
- **THEN** the returned `Song` SHALL have `id`, `name`, `artist`, `album`, `durationMs`, `coverUrl` populated

#### Scenario: Parse malformed JSON
- **WHEN** `NeteaseParser::parseSong(json)` is called with missing required fields
- **THEN** the returned `Song` SHALL have default values and a warning SHALL be logged via the `api` logger

### Requirement: ApiError code classification
`ApiError` classification methods SHALL handle both HTTP status codes and NetEase-specific body codes. The mapping SHALL be:

| Method | HTTP codes | NetEase body codes |
|--------|-----------|--------------------|
| `isNetworkError()` | Connection failure (code = -1) | — |
| `isAuthError()` | 401, 403 | -10 (auth expired), -460 (cheating detection) |
| `isRateLimitError()` | 429 | -429 (too many requests) |
| `isNotFoundError()` | 404 | — |

NetEase API responses use a JSON body with a `code` field (e.g., `{"code": -10, "msg": "..."}`). `ApiError` SHALL store this body code as its primary code when present. HTTP status codes SHALL be used only when the body is not parseable.

#### Scenario: NetEase auth expired
- **WHEN** an `ApiError` is constructed from a response with body code -10
- **THEN** `isAuthError()` SHALL return true

#### Scenario: NetEase cheating detection
- **WHEN** an `ApiError` is constructed from a response with body code -460
- **THEN** `isAuthError()` SHALL return true and `userMessage()` SHALL indicate the account may be restricted

### Requirement: NeteaseClient service registration
`NeteaseClient` SHALL be registered in `ServiceLocator` during `NeriPlayerApplication::initializeCoreServices()`, after `NetworkManager` and `SecureStorage`.

#### Scenario: Retrieve NeteaseClient from ServiceLocator
- **WHEN** `ServiceLocator::service<NeteaseClient>()` is called after application initialization
- **THEN** a non-null `NeteaseClient*` SHALL be returned

### Requirement: NeteaseClient unit tests
The system SHALL include unit tests for:
- `NeteaseCrypto` — verify `weapiEncrypt` output against known test vectors (with fixed IV)
- `NeteaseParser` — parsing songs, albums, artists, playlists, lyrics, search results, login results from recorded JSON fixtures
- Error parsing — malformed JSON handling

Test fixtures SHALL be recorded as static JSON files under `tests/fixtures/netease/`. To update fixtures: capture raw JSON responses from the NetEase API (via browser DevTools or a recording proxy), strip any sensitive data (tokens, user IDs), and commit the sanitized JSON.

#### Scenario: Crypto test-vector verification
- **WHEN** the test calls `weapiEncrypt` with a known plaintext (IV fixed for determinism)
- **THEN** the AES-encrypted portion SHALL match the expected ciphertext

#### Scenario: Parser test with fixture
- **WHEN** the test loads a recorded search response JSON and calls `NeteaseParser::parseSearchResult`
- **THEN** the returned `SearchResult` SHALL have the expected number of songs with correct fields, and `totalCount` and `hasMore` SHALL be populated from the response
