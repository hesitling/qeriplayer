## Purpose

Defines the `SettingsViewModel` requirements for cookie-based NetEase authentication, validated session import, restored-profile hydration, and local-only session clearing.

## MODIFIED Requirements

### Requirement: NetEase authentication import
`SettingsViewModel` SHALL provide a QML-invokable cookie import method that accepts a raw NetEase cookie string and returns a `QCoro::QmlTask`. The method SHALL validate the imported cookie before `isNeteaseLoggedIn` becomes `true`.

#### Scenario: Successful validated import
- **WHEN** QML calls `settingsVm.importNeteaseCookie("MUSIC_U=xxx; __csrf=yyy")` and validation succeeds
- **THEN** `isNeteaseLoggedIn` SHALL become `true`, `neteaseUsername` SHALL be populated, and `neteaseAuthChanged` SHALL be emitted

#### Scenario: Failed validated import
- **WHEN** QML calls `settingsVm.importNeteaseCookie("MUSIC_U=bad")` and validation fails
- **THEN** `isNeteaseLoggedIn` SHALL remain `false`, local candidate session state SHALL be cleared, and `hasError` SHALL become `true`

### Requirement: NetEase logout
`Q_INVOKABLE QCoro::QmlTask logoutNetease()` SHALL clear only local NetEase session state and persisted cookies. It SHALL not call remote NetEase logout.

#### Scenario: Local-only logout
- **WHEN** QML calls `settingsVm.logoutNetease()` while authenticated
- **THEN** `isNeteaseLoggedIn` SHALL become `false`, `neteaseUsername` SHALL be cleared, and local persisted credentials SHALL be removed

## ADDED Requirements

### Requirement: Restored session hydration
When the underlying NetEase client restores a persisted session, `SettingsViewModel` SHALL be able to hydrate profile information for display.

#### Scenario: Hydrate restored session
- **WHEN** `loadSettings()` or an equivalent initialization path runs while a persisted valid NetEase session exists
- **THEN** `neteaseUsername` SHALL be populated from the current account/profile response

#### Scenario: Clear expired restored session
- **WHEN** profile hydration for a restored session fails due to an auth error
- **THEN** local NetEase session state SHALL be cleared and `isNeteaseLoggedIn` SHALL become `false`

### Requirement: Removed multi-method auth state
`SettingsViewModel` SHALL not require QR-code polling state, captcha cooldown state, or password/SMS-specific login entry points for the settings auth flow.

#### Scenario: No QR or captcha auth surface
- **WHEN** QML integrates with `SettingsViewModel` for NetEase auth
- **THEN** the required auth surface SHALL be cookie import, session state, error state, and local-only logout
