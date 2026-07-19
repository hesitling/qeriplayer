## MODIFIED Requirements

### Requirement: NetEase login
`SettingsViewModel` SHALL provide `loginByPassword(const QString &phone, const QString &password)` returning `QCoro::QmlTask`. It SHALL call `NeteaseClient::login()`. On success, `isNeteaseLoggedIn` SHALL become `true` and `neteaseAuthChanged` SHALL be emitted.

#### Scenario: Successful login
- **WHEN** `loginByPassword("13800138000", "password")` succeeds
- **THEN** `isNeteaseLoggedIn` SHALL be `true` and `neteaseUsername` SHALL be populated

#### Scenario: Failed login
- **WHEN** `loginByPassword` fails with an auth error
- **THEN** `hasError` SHALL be `true` and `error.type` SHALL be `Auth`

## ADDED Requirements

### Requirement: Captcha login
`SettingsViewModel` SHALL provide `sendCaptcha(const QString &phone)` returning `QCoro::QmlTask` and `loginByCaptcha(const QString &phone, const QString &captcha)` returning `QCoro::QmlTask`.

#### Scenario: Send captcha succeeds
- **WHEN** `sendCaptcha("13800138000")` succeeds
- **THEN** `captchaCooldown` SHALL be set to 60 and the cooldown timer SHALL start

#### Scenario: Send captcha failure
- **WHEN** `sendCaptcha("13800138000")` fails
- **THEN** `hasError` SHALL be `true` and `captchaCooldown` SHALL remain 0

#### Scenario: Captcha login succeeds
- **WHEN** `loginByCaptcha("13800138000", "123456")` succeeds
- **THEN** `isNeteaseLoggedIn` SHALL be `true` and `neteaseUsername` SHALL be populated

#### Scenario: Captcha login failure
- **WHEN** `loginByCaptcha("13800138000", "000000")` fails
- **THEN** `hasError` SHALL be `true` and `error.type` SHALL be `Auth`

### Requirement: Captcha cooldown timer
`SettingsViewModel` SHALL expose `captchaCooldown` (int, notify: `captchaCooldownChanged`) and `canSendCaptcha` (bool, notify: `canSendCaptchaChanged`). After a successful `sendCaptcha()`, `captchaCooldown` SHALL be 60 and count down by 1 each second. `canSendCaptcha` SHALL be `true` when `captchaCooldown` is 0.

#### Scenario: Cooldown counts down
- **WHEN** `sendCaptcha` succeeds and 5 seconds elapse
- **THEN** `captchaCooldown` SHALL be 55

#### Scenario: Cooldown expires
- **WHEN** 60 seconds elapse after `sendCaptcha` succeeds
- **THEN** `captchaCooldown` SHALL be 0 and `canSendCaptcha` SHALL be `true`

#### Scenario: canSendCaptcha reflects cooldown
- **WHEN** `captchaCooldown` is 30
- **THEN** `canSendCaptcha` SHALL be `false`

### Requirement: QR code login
`SettingsViewModel` SHALL provide `generateQrLogin()` returning `QCoro::QmlTask`, `pollQrLogin()` returning `QCoro::QmlTask`, and `cancelQrLogin()`.

#### Scenario: Generate QR code
- **WHEN** `generateQrLogin()` is called
- **THEN** `qrKey` SHALL be populated and `qrImageUrl` SHALL contain a data URL with the QR code image

#### Scenario: Poll QR status — waiting
- **WHEN** `pollQrLogin()` is called and user has not scanned
- **THEN** `qrLoginStatus` SHALL be `QrLoginStatus::Waiting`

#### Scenario: Poll QR status — scanned
- **WHEN** `pollQrLogin()` is called and user has scanned but not confirmed
- **THEN** `qrLoginStatus` SHALL be `QrLoginStatus::Scanned`

#### Scenario: Poll QR status — confirmed
- **WHEN** `pollQrLogin()` is called and user has confirmed
- **THEN** `qrLoginStatus` SHALL be `QrLoginStatus::Confirmed`, `isNeteaseLoggedIn` SHALL be `true`, and `qrPollingActive` SHALL be `false`

#### Scenario: Poll QR status — expired
- **WHEN** `pollQrLogin()` is called and QR code has expired
- **THEN** `qrLoginStatus` SHALL be `QrLoginStatus::Expired` and `qrPollingActive` SHALL be `false`

#### Scenario: Cancel QR polling
- **WHEN** `cancelQrLogin()` is called while polling is active
- **THEN** `qrPollingActive` SHALL be `false` and subsequent `pollQrLogin()` calls SHALL return immediately

### Requirement: QR login properties
`SettingsViewModel` SHALL expose: `qrLoginStatus` (QrLoginStatus, notify: `qrLoginStatusChanged`), `qrImageUrl` (QUrl, notify: `qrImageUrlChanged`), `qrKey` (QString, notify: `qrKeyChanged`), `qrPollingActive` (bool, notify: `qrPollingActiveChanged`).

#### Scenario: Properties update on generate
- **WHEN** `generateQrLogin()` succeeds
- **THEN** `qrKey` SHALL be non-empty, `qrImageUrl` SHALL be a valid data URL, and `qrLoginStatus` SHALL be `QrLoginStatus::Waiting`

#### Scenario: Properties update on poll
- **WHEN** `pollQrLogin()` returns with status Confirmed
- **THEN** `qrLoginStatus` SHALL be `QrLoginStatus::Confirmed`

### Requirement: QR polling cancellation safety
`SettingsViewModel` SHALL use a `m_qrPollingActive` flag to control polling. `pollQrLoginImpl()` SHALL check this flag at the start and exit early if false. `cancelQrLogin()` SHALL set the flag to false.

#### Scenario: Cancel before poll completes
- **WHEN** `cancelQrLogin()` is called while `pollQrLoginImpl()` is awaiting the API response
- **THEN** the coroutine SHALL exit without updating state after the `co_await` returns
