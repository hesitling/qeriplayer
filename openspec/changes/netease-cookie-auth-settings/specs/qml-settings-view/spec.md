## Purpose

Defines the QML settings page (`SettingsView.qml`), cookie import dialog (`LoginDialog.qml` or equivalent replacement), and account-management interactions for NetEase authentication.

## MODIFIED Requirements

### Requirement: NetEase Account section
`SettingsView` SHALL display the current NetEase session state and an account action button. When logged out, the action SHALL open the cookie import dialog. When logged in, the action SHALL clear the local session without performing remote NetEase logout.

#### Scenario: Logged out state
- **WHEN** `settingsVm.isNeteaseLoggedIn` is false
- **THEN** the status label SHALL show "Not logged in" and the action button SHALL open the cookie import dialog

#### Scenario: Logged in state
- **WHEN** `settingsVm.isNeteaseLoggedIn` is true and `settingsVm.neteaseUsername` is "TestUser"
- **THEN** the status label SHALL show "Logged in as TestUser"

#### Scenario: Local-only sign-out action
- **WHEN** the user activates the signed-in account action
- **THEN** `settingsVm.logoutNetease()` SHALL be called and only local session state SHALL be cleared

### Requirement: Account error display
`SettingsView` SHALL display an error label in the Account section when `settingsVm.hasError` is true. The label SHALL show `settingsVm.error.message`.

#### Scenario: Cookie validation error displayed
- **WHEN** `settingsVm.hasError` is true and `settingsVm.error.message` is "Cookie is invalid or expired"
- **THEN** the error label SHALL be visible with that message

### Requirement: Cookie import dialog structure
The system SHALL provide a modal cookie import dialog for NetEase authentication. The dialog SHALL contain a multiline text input for the cookie string and a submit action.

#### Scenario: Cookie dialog opens
- **WHEN** the user clicks the logged-out account action in Settings
- **THEN** the cookie import dialog SHALL open as a modal dialog

### Requirement: Cookie import submission
Submitting the dialog SHALL call the ViewModel cookie import method with the pasted raw cookie string.

#### Scenario: Cookie import calls VM
- **WHEN** the user pastes `MUSIC_U=xxx; __csrf=yyy` and submits
- **THEN** `settingsVm.importNeteaseCookie("MUSIC_U=xxx; __csrf=yyy")` SHALL be called

#### Scenario: Successful cookie import closes dialog
- **WHEN** cookie import completes and `settingsVm.isNeteaseLoggedIn` becomes true
- **THEN** the cookie import dialog SHALL close

#### Scenario: Failed cookie import shows error
- **WHEN** cookie import completes and `settingsVm.hasError` is true
- **THEN** the dialog SHALL remain open and display `settingsVm.error.message`

### Requirement: Cookie dialog cancellation
When the cookie import dialog closes without a successful login, it SHALL clear transient error state.

#### Scenario: Cancel clears error
- **WHEN** the user closes the cookie import dialog after a failed import
- **THEN** `settingsVm.clearError()` SHALL be called

## ADDED Requirements

### Requirement: Session hydration display
When a persisted NetEase session is restored and profile data is available, `SettingsView` SHALL display the hydrated username without requiring a fresh manual import.

#### Scenario: Restored session shows username
- **WHEN** the app loads Settings with `settingsVm.isNeteaseLoggedIn == true` and hydrated profile name `TestUser`
- **THEN** the account status SHALL show "Logged in as TestUser"
