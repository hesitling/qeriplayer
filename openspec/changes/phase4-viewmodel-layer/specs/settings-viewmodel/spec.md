## ADDED Requirements

### Requirement: SettingsViewModel manages settings and auth
The system SHALL define `SettingsViewModel` as a `QObject` that reads/writes settings via `ISettingsRepository` and manages platform authentication.

#### Scenario: SettingsViewModel exposes settings
- **WHEN** QML accesses `settingsViewModel.theme`
- **THEN** it SHALL return the current theme setting from `ISettingsRepository`

### Requirement: Theme property
`SettingsViewModel` SHALL expose `theme` (QString, read/write, notify: `themeChanged`). Values: "light", "dark". The setter SHALL call `ISettingsRepository::set("theme", value)`.

#### Scenario: Theme set and persisted
- **WHEN** `setTheme("dark")` is called
- **THEN** `ISettingsRepository::set("theme", "dark")` SHALL be called and `themeChanged` SHALL be emitted

### Requirement: Audio quality property
`SettingsViewModel` SHALL expose `audioQuality` (AudioQuality, read/write, notify: `audioQualityChanged`). The setter SHALL call `ISettingsRepository::set("audioQuality", ...)`.

#### Scenario: Audio quality persisted
- **WHEN** `setAudioQuality(AudioQuality::High)` is called
- **THEN** `ISettingsRepository::set("audioQuality", "High")` SHALL be called

### Requirement: Download path property
`SettingsViewModel` SHALL expose `downloadPath` (QString, read/write, notify: `downloadPathChanged`). The setter SHALL call `ISettingsRepository::set("downloadPath", value)`.

#### Scenario: Download path persisted
- **WHEN** `setDownloadPath("/home/user/Music")` is called
- **THEN** `ISettingsRepository::set("downloadPath", "/home/user/Music")` SHALL be called

### Requirement: NetEase authentication status
`SettingsViewModel` SHALL expose `isNeteaseLoggedIn` (bool, read-only, notify: `neteaseAuthChanged`) and `neteaseUsername` (QString, read-only, notify: `neteaseAuthChanged`).

#### Scenario: Auth status reflects client state
- **WHEN** `NeteaseClient::isAuthenticated()` returns `true`
- **THEN** `isNeteaseLoggedIn` SHALL be `true`

### Requirement: NetEase login
`Q_INVOKABLE loginNetease(const QString &phone, const QString &password)` SHALL call `NeteaseClient::login()`. On success, `isNeteaseLoggedIn` SHALL become `true` and `neteaseAuthChanged` SHALL be emitted.

#### Scenario: Successful login
- **WHEN** `loginNetease("13800138000", "password")` succeeds
- **THEN** `isNeteaseLoggedIn` SHALL be `true` and `neteaseUsername` SHALL be populated

#### Scenario: Failed login
- **WHEN** `loginNetease` fails with an auth error
- **THEN** `hasError` SHALL be `true` and `error.type` SHALL be `Auth`

### Requirement: NetEase logout
`Q_INVOKABLE logoutNetease()` SHALL call `NeteaseClient::logout()`. On completion, `isNeteaseLoggedIn` SHALL become `false`.

#### Scenario: Logout clears auth state
- **WHEN** `logoutNetease()` succeeds
- **THEN** `isNeteaseLoggedIn` SHALL be `false` and `neteaseAuthChanged` SHALL be emitted

### Requirement: Load settings on startup
`Q_INVOKABLE loadSettings()` SHALL read all settings from `ISettingsRepository` and populate properties. It SHALL be called during app initialization.

#### Scenario: Settings loaded from repository
- **WHEN** `loadSettings()` is called and the repo has `theme=dark`, `audioQuality=High`
- **THEN** `theme` SHALL be "dark" and `audioQuality` SHALL be `High`

### Requirement: Clear play history
`Q_INVOKABLE clearPlayHistory()` SHALL call `IPlayHistoryRepository::clear()`.

#### Scenario: History cleared
- **WHEN** `clearPlayHistory()` is called
- **THEN** `IPlayHistoryRepository::clear()` SHALL be called

### Requirement: Error state
`SettingsViewModel` SHALL expose `hasError` (bool, notify: `errorChanged`) and `error` (ViewModelError, notify: `errorChanged`).

#### Scenario: Database error surfaced
- **WHEN** `ISettingsRepository::set()` throws a database error
- **THEN** `hasError` SHALL be `true` and `error.type` SHALL be `Database`
