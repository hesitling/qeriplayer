## Why

The settings page is currently a placeholder (`"Settings (PR 5)"` label). Users need a functional UI to manage application settings (theme, audio quality, download path), authenticate with NetEase (supporting password, SMS captcha, and QR code login methods), and view app information.

## What Changes

**QML UI (new)**
- `SettingsView.qml` — Settings form with General, Account, and Storage sections
- `LoginDialog.qml` — Tabbed login dialog supporting password, SMS captcha, and QR code methods
- `AboutDialog.qml` — App version and credits dialog

**API Layer**
- Add `generateQrKey()` to `NeteaseClient` — generates QR code key and image URL
- Add `pollQrStatus(key)` to `NeteaseClient` — polls QR scan status until confirmed/expired

**Domain Layer**
- Add `QrLoginStatus` enum to `Enums.h` — status codes for QR login polling (Expired, Waiting, Scanned, Confirmed)

**ViewModel Layer**
- Rename `loginNetease()` to `loginByPassword()` for clarity
- Add `sendCaptcha(phone)` and `loginByCaptcha(phone, captcha)` for SMS login
- Add `generateQrLogin()`, `pollQrLogin()`, and `cancelQrLogin()` for QR code login
- Add `captchaCooldown` property with 60-second countdown timer
- Add `qrLoginStatus`, `qrImageUrl`, and `qrKey` properties for QR state

**Build System**
- Add `Qt6::LabsPlatform` to CMakeLists.txt for native folder dialog

**Non-goals**
- Theme switching at runtime (Material Dark only for now)
- Multi-platform login (NetEase only; Bilibili/YouTube auth is future work)
- QR code rendering library (using API-provided `qrimg` base64 PNG)

## Capabilities

### New Capabilities
- `qml-settings-view`: QML settings page with grouped sections for general settings, account management, and storage

### Modified Capabilities
- `vm-settings`: Add captcha and QR code login methods, cooldown timer, QR polling state
- `api-netease`: Add QR code key generation and status polling endpoints

## Impact

- **Files touched**: ~12 files across api, domain, viewmodel, qml, and build layers
- **Dependencies**: `Qt6::LabsPlatform` (already installed, just needs CMake linkage)
- **Breaking**: `loginNetease()` renamed to `loginByPassword()` — all call sites must update
- **Tests**: New `tst_SettingsView.qml` for QML component tests
