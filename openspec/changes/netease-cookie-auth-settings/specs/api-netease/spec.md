## Purpose

Defines the NetEase auth/client behavior required for validated cookie import and local-only session clearing.

## ADDED Requirements

### Requirement: Validated cookie import
The NetEase auth/client layer SHALL provide a way to accept a raw cookie string, ensure a usable WeAPI session, and validate the session via current-account/profile retrieval before the session is considered authenticated.

#### Scenario: Valid cookie import succeeds
- **WHEN** the client receives cookie string `MUSIC_U=xxx; __csrf=yyy` and the current-account/profile request succeeds
- **THEN** the client SHALL persist the normalized cookie state and report an authenticated session

#### Scenario: Missing csrf is repaired during validation
- **WHEN** the client receives a cookie string without `__csrf` but the session is otherwise valid
- **THEN** `ensureWeapiSession()` SHALL run before validation completes and the final persisted cookie state SHALL include the repaired session cookies if obtained

#### Scenario: Invalid cookie import fails closed
- **WHEN** the client receives an invalid or expired cookie string and account validation fails
- **THEN** the client SHALL clear local candidate auth state and SHALL not report an authenticated session

### Requirement: Local-only session clearing
The NetEase auth/client layer SHALL provide a local-only session clear path that removes in-memory and persisted cookies without calling remote NetEase logout.

#### Scenario: Clear local session
- **WHEN** the local clear-session path is invoked
- **THEN** in-memory cookies, CSRF token, and persisted cookie storage SHALL be cleared and `isAuthenticated()` SHALL become `false`
