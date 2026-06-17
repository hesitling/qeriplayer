## ADDED Requirements

### Requirement: ViewModelError Q_GADGET definition
The system SHALL define a `ViewModelError` class as a `Q_GADGET` with `Q_PROPERTY` fields for `type` (ErrorType enum), `message` (QString), `details` (QString), and `canRetry` (bool).

#### Scenario: ViewModelError is registered with Qt metatype
- **WHEN** the application starts
- **THEN** `ViewModelError` SHALL be usable in Q_PROPERTY, signals, and QML contexts via `Q_DECLARE_METATYPE`

### Requirement: ErrorType enum classification
The system SHALL define `ViewModelError::ErrorType` as a `Q_ENUM` with values: `Network`, `Auth`, `RateLimit`, `NotFound`, `Api`, `Database`, `Validation`, `Unknown`.

#### Scenario: ErrorType visible in QML
- **WHEN** QML accesses `ViewModelError.ErrorType.Network`
- **THEN** it SHALL resolve to the correct enum value

### Requirement: Factory from ApiError
The system SHALL provide `ViewModelError::fromApiError(const ApiError &)` that maps `ApiError` classifications to `ErrorType`: `isNetworkError()` → `Network`, `isAuthError()` → `Auth`, `isRateLimitError()` → `RateLimit`, `isNotFoundError()` → `NotFound`, otherwise → `Api`. The `message` SHALL come from `ApiError::userMessage()`.

#### Scenario: ApiError network mapped to ViewModelError
- **WHEN** `fromApiError` is called with an `ApiError` where `isNetworkError()` returns true
- **THEN** the resulting `ViewModelError` SHALL have `type == Network`, `message` from `ApiError::userMessage()`, and `canRetry == true`

#### Scenario: ApiError auth mapped to ViewModelError
- **WHEN** `fromApiError` is called with an `ApiError` where `isAuthError()` returns true
- **THEN** the resulting `ViewModelError` SHALL have `type == Auth` and `canRetry == false`

### Requirement: Factory constructors for non-API errors
The system SHALL provide static factory methods `ViewModelError::network(message)`, `ViewModelError::database(message)`, and `ViewModelError::validation(message)`.

#### Scenario: Database error created
- **WHEN** `ViewModelError::database("SQLite constraint violation")` is called
- **THEN** the result SHALL have `type == Database`, `message == "SQLite constraint violation"`, `canRetry == false`

### Requirement: canRetry classification
`canRetry()` SHALL return `true` for `Network`, `RateLimit`, and `Api` error types. It SHALL return `false` for `Auth`, `NotFound`, `Database`, `Validation`, and `Unknown`.

#### Scenario: Network error is retryable
- **WHEN** a `ViewModelError` has `type == Network`
- **THEN** `canRetry()` SHALL return `true`

#### Scenario: Validation error is not retryable
- **WHEN** a `ViewModelError` has `type == Validation`
- **THEN** `canRetry()` SHALL return `false`
