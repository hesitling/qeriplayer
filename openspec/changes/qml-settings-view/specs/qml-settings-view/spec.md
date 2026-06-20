## Purpose

Defines the QML settings page (`SettingsView.qml`), login dialog (`LoginDialog.qml`), and about dialog (`AboutDialog.qml`). These components provide the user interface for managing application settings and NetEase account authentication.

## ADDED Requirements

### Requirement: SettingsView page structure
The system SHALL provide a `SettingsView.qml` component that displays settings in grouped sections: General, NetEase Account, and Storage, plus an About button.

#### Scenario: SettingsView loads
- **WHEN** the user navigates to Settings from the sidebar
- **THEN** `SettingsView.qml` SHALL be displayed with all three sections visible

### Requirement: General section — theme
`SettingsView` SHALL display a ComboBox for theme selection with options "Light" and "Dark". The ComboBox SHALL bind bidirectionally to `settingsVm.theme`.

#### Scenario: Theme selection reflects VM state
- **WHEN** `settingsVm.theme` is "dark"
- **THEN** the theme ComboBox SHALL show "Dark" as selected

#### Scenario: Theme change propagates to VM
- **WHEN** the user selects "Light" from the theme ComboBox
- **THEN** `settingsVm.setTheme("light")` SHALL be called

### Requirement: General section — audio quality
`SettingsView` SHALL display a ComboBox for audio quality with options "Low", "Standard", "High", "Lossless". The ComboBox SHALL bind to `settingsVm.audioQuality`.

#### Scenario: Audio quality reflects VM state
- **WHEN** `settingsVm.audioQuality` is `AudioQuality::High` (enum value 2)
- **THEN** the quality ComboBox SHALL show "High" as selected

#### Scenario: Audio quality change propagates to VM
- **WHEN** the user selects "Lossless" from the quality ComboBox
- **THEN** `settingsVm.setAudioQuality(3)` SHALL be called (3 = Lossless enum value)

### Requirement: General section — download path
`SettingsView` SHALL display a TextField for the download path and a "Browse" button. The TextField SHALL bind to `settingsVm.downloadPath`. The Browse button SHALL open a native `FolderDialog`.

#### Scenario: Download path reflects VM state
- **WHEN** `settingsVm.downloadPath` is "/home/user/Music"
- **THEN** the download path TextField SHALL show "/home/user/Music"

#### Scenario: Browse opens folder dialog
- **WHEN** the user clicks "Browse"
- **THEN** a native folder selection dialog SHALL open

#### Scenario: Folder selection updates VM
- **WHEN** the user selects a folder in the dialog
- **THEN** `settingsVm.setDownloadPath(selectedFolder)` SHALL be called

### Requirement: NetEase Account section
`SettingsView` SHALL display the current login status ("Logged in as {username}" or "Not logged in") and a button that shows "Login" when logged out or "Logout" when logged in.

#### Scenario: Logged out state
- **WHEN** `settingsVm.isNeteaseLoggedIn` is false
- **THEN** the status label SHALL show "Not logged in" and the button SHALL show "Login"

#### Scenario: Logged in state
- **WHEN** `settingsVm.isNeteaseLoggedIn` is true and `settingsVm.neteaseUsername` is "TestUser"
- **THEN** the status label SHALL show "Logged in as TestUser" and the button SHALL show "Logout"

#### Scenario: Login button opens dialog
- **WHEN** the user clicks "Login"
- **THEN** `LoginDialog.qml` SHALL open as a modal dialog

#### Scenario: Logout calls VM
- **WHEN** the user clicks "Logout"
- **THEN** `settingsVm.logoutNetease()` SHALL be called

### Requirement: Account error display
`SettingsView` SHALL display an error label in the Account section when `settingsVm.hasError` is true. The label SHALL show `settingsVm.error.message`.

#### Scenario: Login error displayed
- **WHEN** `settingsVm.hasError` is true and `settingsVm.error.message` is "Invalid credentials"
- **THEN** the error label SHALL be visible with text "Invalid credentials"

### Requirement: Storage section — clear history
`SettingsView` SHALL display a "Clear Play History" button that calls `settingsVm.clearPlayHistory()`.

#### Scenario: Clear history calls VM
- **WHEN** the user clicks "Clear Play History"
- **THEN** `settingsVm.clearPlayHistory()` SHALL be called

### Requirement: About button
`SettingsView` SHALL display an "About QeriPlayer" button that opens `AboutDialog.qml`.

#### Scenario: About button opens dialog
- **WHEN** the user clicks "About QeriPlayer"
- **THEN** `AboutDialog.qml` SHALL open as a modal dialog

### Requirement: LoginDialog structure
The system SHALL provide a `LoginDialog.qml` component with a `TabBar` containing three tabs: "Password", "SMS Code", and "QR Code". Each tab SHALL display its own login form.

#### Scenario: LoginDialog opens with Password tab
- **WHEN** the LoginDialog opens
- **THEN** the Password tab SHALL be selected by default

### Requirement: Password tab
The Password tab SHALL display a phone number TextField, a password TextField (with echo mode Password), and a "Login" button. On submit, it SHALL call `settingsVm.loginByPassword(phone, password)`.

#### Scenario: Password login calls VM
- **WHEN** the user enters phone "13800138000", password "secret", and clicks "Login"
- **THEN** `settingsVm.loginByPassword("13800138000", "secret")` SHALL be called

#### Scenario: Password login success closes dialog
- **WHEN** `loginByPassword` completes and `settingsVm.isNeteaseLoggedIn` becomes true
- **THEN** the LoginDialog SHALL close

#### Scenario: Password login failure shows error
- **WHEN** `loginByPassword` completes and `settingsVm.hasError` is true
- **THEN** the Password tab SHALL display `settingsVm.error.message`

### Requirement: SMS Code tab
The SMS Code tab SHALL display a phone number TextField, a captcha TextField, a "Send Code" button, and a "Login" button. The "Send Code" button SHALL call `settingsVm.sendCaptcha(phone)`. The "Login" button SHALL call `settingsVm.loginByCaptcha(phone, captcha)`.

#### Scenario: Send Code calls VM
- **WHEN** the user enters phone "13800138000" and clicks "Send Code"
- **THEN** `settingsVm.sendCaptcha("13800138000")` SHALL be called

#### Scenario: Send Code starts cooldown
- **WHEN** `sendCaptcha` completes successfully
- **THEN** the "Send Code" button SHALL show countdown text (e.g., "Resend (60s)") and become disabled

#### Scenario: Captcha login calls VM
- **WHEN** the user enters captcha "123456" and clicks "Login"
- **THEN** `settingsVm.loginByCaptcha("13800138000", "123456")` SHALL be called

### Requirement: QR Code tab
The QR Code tab SHALL display a QR code image and a status label. On tab activation, it SHALL call `settingsVm.generateQrLogin()` to generate a new QR code. A 3-second timer SHALL call `settingsVm.pollQrLogin()` to check scan status.

#### Scenario: Tab activation generates QR code
- **WHEN** the QR Code tab becomes visible
- **THEN** `settingsVm.generateQrLogin()` SHALL be called

#### Scenario: QR code image displayed
- **WHEN** `settingsVm.qrImageUrl` is a valid data URL
- **THEN** the Image component SHALL display the QR code

#### Scenario: Polling updates status
- **WHEN** `settingsVm.qrLoginStatus` changes to `QrLoginStatus::Scanned`
- **THEN** the status label SHALL show "Scanned — confirm on phone"

#### Scenario: QR confirmed closes dialog
- **WHEN** `settingsVm.isNeteaseLoggedIn` becomes true after polling
- **THEN** the LoginDialog SHALL close

#### Scenario: QR expired shows refresh
- **WHEN** `settingsVm.qrLoginStatus` is `QrLoginStatus::Expired`
- **THEN** the status label SHALL show "Expired" and a "Refresh" button SHALL appear

#### Scenario: Refresh regenerates QR
- **WHEN** the user clicks "Refresh"
- **THEN** `settingsVm.generateQrLogin()` SHALL be called again

### Requirement: LoginDialog cancellation
When the LoginDialog closes (by Cancel button or X), it SHALL call `settingsVm.cancelQrLogin()` and `settingsVm.clearError()`.

#### Scenario: Cancel stops QR polling
- **WHEN** the user closes the LoginDialog while QR polling is active
- **THEN** `settingsVm.cancelQrLogin()` SHALL be called and polling SHALL stop

### Requirement: AboutDialog content
`AboutDialog.qml` SHALL display the app name "QeriPlayer Qt", version from `Qt.application.version`, and build dependencies (Qt version, QCoro, C++20). It SHALL have an OK button to close.

#### Scenario: AboutDialog shows version
- **WHEN** `Qt.application.version` is "0.1.0"
- **THEN** the dialog SHALL display "Version 0.1.0"

#### Scenario: AboutDialog closes on OK
- **WHEN** the user clicks OK
- **THEN** the dialog SHALL close

### Requirement: main.qml integration
`main.qml` SHALL replace the placeholder `settingsPage` Component with `SettingsView {}`.

#### Scenario: Settings navigation shows SettingsView
- **WHEN** `mainVm.currentView` changes to `MainViewModel::View::Settings` (value 5)
- **THEN** the contentStack SHALL display `SettingsView.qml`

### Requirement: qml.qrc registration
`qml.qrc` SHALL include entries for `SettingsView.qml`, `LoginDialog.qml`, and `AboutDialog.qml`.

#### Scenario: QML files loadable from resources
- **WHEN** QML engine loads `qrc:/qml/SettingsView.qml`
- **THEN** the file SHALL be found and loaded successfully
