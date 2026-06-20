## 1. Domain — QrLoginStatus enum

- [x] 1.1 Add `QrLoginStatus` enum to `src/domain/Enums.h` with values `Expired=0`, `Waiting=1`, `Scanned=2`, `Confirmed=3`
- [x] 1.2 Add `Q_DECLARE_METATYPE(QeriPlayerQt::QrLoginStatus)` to `Enums.h`

## 2. API — NeteaseClient QR code methods

- [x] 2.1 Add `generateQrKey()` declaration to `NeteaseClient.h` returning `QCoro::Task<ApiResult<QrCodeData>>`
- [x] 2.2 Add `pollQrStatus(key)` declaration to `NeteaseClient.h` returning `QCoro::Task<ApiResult<LoginResult>>`
- [x] 2.3 Implement `generateQrKey()` in `NeteaseClientAuth.cpp` — call `/login/qr/key` then `/login/qr/create`, convert `qrimg` base64 to data URL in `QrCodeData.qrUrl`
- [x] 2.4 Implement `pollQrStatus(key)` in `NeteaseClientAuth.cpp` — call `/login/qr/check`, map status codes (800=expired, 801=waiting, 802=scanned, 803=confirmed) to `ApiResult<LoginResult>` or `ApiError`

## 3. ViewModel — SettingsViewModel rework

- [x] 3.1 Rename `loginNetease()` to `loginByPassword()` in `SettingsViewModel.h` and `.cpp`
- [x] 3.2 Add `sendCaptcha(phone)` and `loginByCaptcha(phone, captcha)` declarations returning `QCoro::QmlTask`
- [x] 3.3 Implement `sendCaptchaImpl()` and `loginByCaptchaImpl()` coroutines in `SettingsViewModel.cpp`
- [x] 3.4 Add `captchaCooldown` (int) and `canSendCaptcha` (bool) properties with signals
- [x] 3.5 Add `QTimer *m_captchaCooldownTimer` member, connect timeout to decrement `captchaCooldown`
- [x] 3.6 Start cooldown timer after successful `sendCaptcha()`, stop when cooldown reaches 0
- [x] 3.7 Add `generateQrLogin()`, `pollQrLogin()`, `cancelQrLogin()` declarations
- [x] 3.8 Implement `generateQrLoginImpl()` — call `m_neteaseClient->generateQrKey()`, set `qrKey`, `qrImageUrl`, `qrLoginStatus`
- [x] 3.9 Implement `pollQrLoginImpl()` — check `m_qrPollingActive` flag, call `m_neteaseClient->pollQrStatus()`, update `qrLoginStatus` and auth state on success
- [x] 3.10 Implement `cancelQrLogin()` — set `m_qrPollingActive = false`
- [x] 3.11 Add `qrLoginStatus`, `qrImageUrl`, `qrKey`, `qrPollingActive` properties with signals
- [x] 3.12 Add `m_qrPollingActive` flag member, initialize to false

## 4. Build — CMakeLists.txt

- [x] 4.1 Add `Qt6::LabsPlatform` to `find_package(Qt6 ...)` in `CMakeLists.txt`
- [x] 4.2 Link `Qt6::LabsPlatform` to the QML/UI target

## 5. QML — SettingsView

- [x] 5.1 Create `src/qml/SettingsView.qml` with Page root, Flickable, Column layout
- [x] 5.2 Implement General section with theme ComboBox (Light/Dark), audio quality ComboBox, download path TextField + Browse button
- [x] 5.3 Implement NetEase Account section with login status label and Login/Logout button
- [x] 5.4 Implement error label in Account section bound to `settingsVm.hasError` / `settingsVm.error.message`
- [x] 5.5 Implement Storage section with "Clear Play History" button
- [x] 5.6 Implement "About QeriPlayer" button
- [x] 5.7 Add `FolderDialog` from `Qt.labs.platform` bound to download path Browse button

## 6. QML — LoginDialog

- [x] 6.1 Create `src/qml/LoginDialog.qml` with Dialog root, TabBar (Password, SMS Code, QR Code)
- [x] 6.2 Implement Password tab with phone/password TextFields and Login button calling `settingsVm.loginByPassword()`
- [x] 6.3 Implement SMS Code tab with phone/captcha TextFields, Send Code button calling `settingsVm.sendCaptcha()`, Login button calling `settingsVm.loginByCaptcha()`
- [x] 6.4 Bind Send Code button text to `settingsVm.canSendCaptcha` / `settingsVm.captchaCooldown` for countdown display
- [x] 6.5 Implement QR Code tab with Image bound to `settingsVm.qrImageUrl`, status label bound to `settingsVm.qrLoginStatus`
- [x] 6.6 Add 3-second Timer in QR tab calling `settingsVm.pollQrLogin()`, running when status is Waiting or Scanned
- [x] 6.7 Call `settingsVm.generateQrLogin()` when QR tab becomes visible
- [x] 6.8 Show "Refresh" button when status is Expired
- [x] 6.9 Close dialog when `settingsVm.isNeteaseLoggedIn` becomes true
- [x] 6.10 Call `settingsVm.cancelQrLogin()` and `settingsVm.clearError()` on dialog close

## 7. QML — AboutDialog

- [x] 7.1 Create `src/qml/AboutDialog.qml` with Dialog root showing app name, `Qt.application.version`, Qt/QCoro/C++20 info, and OK button

## 8. Integration

- [x] 8.1 Add `SettingsView.qml`, `LoginDialog.qml`, `AboutDialog.qml` entries to `src/qml/qml.qrc`
- [x] 8.2 Replace placeholder `settingsPage` Component in `main.qml` with `SettingsView {}`
- [x] 8.3 Add `Connections` block in `main.qml` for `settingsVm.errorChanged` to show toast

## 9. Tests

- [ ] 9.1 Create `tests/qml/data/tst_SettingsView.qml` with mock `settingsVm` QtObject
- [ ] 9.2 Test: SettingsView component loads without error
- [ ] 9.3 Test: Theme combo reflects VM theme property
- [ ] 9.4 Test: Login button opens LoginDialog
- [ ] 9.5 Test: Logout button calls `settingsVm.logoutNetease()`
- [ ] 9.6 Test: Clear history button calls `settingsVm.clearPlayHistory()`
- [ ] 9.7 Test: About button opens AboutDialog
