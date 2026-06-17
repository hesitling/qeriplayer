## Purpose

Defines the common API types shared across all platform integrations: `ApiError` for error handling, `ApiResult`/`VoidResult`/`LoginResult` for typed operation results, `QrCodeData` for login flows, `PlayHistory` for history entries, and `IMusicPlatformPlugin` as the abstract plugin interface.

## Requirements

### Requirement: ApiError type
The system SHALL provide an `ApiError` class with an integer error code, a human-readable message string, and an optional details string. `ApiError` SHALL expose convenience methods: `isNetworkError()`, `isAuthError()`, `isRateLimitError()`, `isNotFoundError()` that classify the error based on the code. `ApiError` SHALL provide a `userMessage()` method returning a localized, user-facing description.

#### Scenario: Construct an ApiError with code and message
- **WHEN** an `ApiError` is constructed with code 401 and message "Unauthorized"
- **THEN** `code()` SHALL return 401, `message()` SHALL return "Unauthorized", and `isAuthError()` SHALL return true

#### Scenario: Network error classification
- **WHEN** an `ApiError` is constructed with a network-related code (e.g., -1 for connection failure)
- **THEN** `isNetworkError()` SHALL return true and `isAuthError()` SHALL return false

#### Scenario: Rate limit error classification
- **WHEN** an `ApiError` is constructed with code 429
- **THEN** `isRateLimitError()` SHALL return true

#### Scenario: User message generation
- **WHEN** `userMessage()` is called on any `ApiError`
- **THEN** it SHALL return a non-empty, human-readable string suitable for display in the UI

### Requirement: ApiResult template
The system SHALL provide an `ApiResult<T>` template that holds either a value of type `T` or an `ApiError`. It SHALL expose `isSuccess()`, `isError()`, `data()`, and `error()` methods. An `ApiResult` in error state SHALL be implicitly convertible to `bool` as `false`.

#### Scenario: Successful result
- **WHEN** an `ApiResult<Song>` is constructed with a `Song` value
- **THEN** `isSuccess()` SHALL return true, `data()` SHALL return the song, and implicit bool conversion SHALL be true

#### Scenario: Error result
- **WHEN** an `ApiResult<Song>` is constructed with an `ApiError`
- **THEN** `isError()` SHALL return true, `error()` SHALL return the error, and implicit bool conversion SHALL be false

#### Scenario: Accessing data on error result
- **WHEN** `data()` is called on an `ApiResult` in error state
- **THEN** the behavior SHALL be defined (return default-constructed `T` or throw) — documented in the header

### Requirement: VoidResult type
The system SHALL provide an empty struct `VoidResult {}` for operations that return no meaningful data on success. `ApiResult<VoidResult>` SHALL be valid and represent success-or-error with no payload. `VoidResult` MUST be a concrete type (not `void` or `std::monostate`) so it works as a template parameter.

#### Scenario: VoidResult success
- **WHEN** an `ApiResult<VoidResult>` is constructed in success state
- **THEN** `isSuccess()` SHALL return true and `data()` SHALL return a valid `VoidResult`

### Requirement: IMusicPlatformPlugin interface
The system SHALL provide an `IMusicPlatformPlugin` abstract interface. All platform clients SHALL implement this interface. The interface SHALL declare the following pure virtual methods returning `QCoro::Task`:
- `search(keyword, type, limit, offset)` → `ApiResult<SearchResult>`
- `getSongDetail(songId)` → `ApiResult<Song>`
- `getSongUrl(songId, quality)` → `ApiResult<SongUrlResult>`
- `getLyrics(songId)` → `ApiResult<Lyrics>`
- `isAuthenticated()` → `bool` (synchronous — intentionally in-memory check; see design D6 rationale)
- `platformName()` → `QString`

The interface SHALL include only operations that make sense across all platforms. Platform-specific operations (e.g., `getSimilarSongs`, playlist CRUD, user operations) SHALL live on the concrete client class, not on the interface.

#### Scenario: NeteaseClient implements the interface
- **WHEN** `NeteaseClient` is compiled
- **THEN** it SHALL satisfy the `IMusicPlatformPlugin` interface (compile-time check via override)

#### Scenario: Search through the interface
- **WHEN** `search("test", SearchType::Song, 10, 0)` is called on any `IMusicPlatformPlugin` implementation
- **THEN** it SHALL return an `ApiResult<SearchResult>` with songs populated on success

### Requirement: LoginResult type
The system SHALL provide a `LoginResult` struct containing: `userId` (QString), `nickname` (QString), `avatarUrl` (QUrl), and `cookie` (QString). The `cookie` field SHALL be a semicolon-delimited string of key=value pairs (e.g., `"MUSIC_U=xxx; __csrf=yyy"`) suitable for injection into HTTP `Cookie` headers.

#### Scenario: Construct a LoginResult
- **WHEN** a `LoginResult` is parsed from a successful login response
- **THEN** `userId`, `nickname`, `avatarUrl`, and `cookie` SHALL be populated with `cookie` in `"key=val; key=val"` format

### Requirement: QrCodeData type
The system SHALL provide a `QrCodeData` struct containing: `key` (QString), `qrUrl` (QUrl — the QR code image or data URL), and `expiresInSeconds` (int).

#### Scenario: Construct a QrCodeData
- **WHEN** a `QrCodeData` is returned from a QR code generation call
- **THEN** `key` SHALL be non-empty and `qrUrl` SHALL be a valid URL

### Requirement: PlayHistory type
The system SHALL provide a `PlayHistory` struct containing: `song` (Song), `playedAt` (qint64 — epoch ms), and `playCount` (int).

#### Scenario: Construct a PlayHistory
- **WHEN** a `PlayHistory` is parsed from an API response
- **THEN** `song` SHALL be populated and `playedAt` SHALL be a valid epoch timestamp

### Requirement: Q_DECLARE_METATYPE registration
All API common types (`ApiError`, `VoidResult`, `LoginResult`, `QrCodeData`, `PlayHistory`) SHALL be registered with `Q_DECLARE_METATYPE` for QVariant interop. `SongUrlResult` and `AudioInfo` are already registered in domain models.

#### Scenario: Store LoginResult in QVariant
- **WHEN** a `LoginResult` is stored in a `QVariant` and retrieved
- **THEN** the retrieved value SHALL equal the original
