## Context

Phase 5 builds the QML UI for QeriPlayer Qt. PRs 0-4 are complete (QmlTask integration, main shell, player bar, search, playlists). The settings page is a placeholder. The `SettingsViewModel` already exposes `theme`, `audioQuality`, `downloadPath`, `isNeteaseLoggedIn`, `neteaseUsername`, and error handling. `NeteaseClient` has password login, captcha login (`sendCaptcha`, `loginByCaptcha`), and logout — but no QR code methods despite `QrCodeData` existing in `api/common/`.

## Goals / Non-Goals

**Goals:**
- Build a functional settings UI with grouped sections (General, Account, Storage, About)
- Support three NetEase login methods: password, SMS captcha, QR code
- Use native folder picker for download path selection
- Display QR code as base64 PNG from API response

**Non-Goals:**
- Runtime theme switching (Material Dark only)
- Multi-platform auth (NetEase only)
- QR code rendering library (use API-provided `qrimg`)

## Decisions

### Decision 1: Three login tabs in a single dialog

**Choice**: Single `LoginDialog.qml` with `TabBar` for Password / SMS / QR Code.

**Alternatives considered**:
- Separate dialogs per method → more files, more navigation complexity
- Inline login in SettingsView → clutters the page, harder to dismiss

**Rationale**: TabBar keeps all methods discoverable in one place. Dialog is modal and dismissible.

### Decision 2: QR code image as base64 data URL

**Choice**: Store `qrimg` (base64 PNG) from API response as a data URL in `QrCodeData.qrUrl`. Display with `Image { source: qrUrl }`.

**Alternatives considered**:
- Add `qrImageBase64` field to `QrCodeData` → breaks existing spec
- Use `qrurl` and render QR externally → requires QR rendering dependency

**Rationale**: `qrUrl` is documented as "QR code image or data URL" in `core-api-common` spec. A data URL (`data:image/png;base64,...`) is a valid URL and renders natively in QML `Image`.

### Decision 3: Captcha cooldown timer in ViewModel

**Choice**: `SettingsViewModel` owns a `QTimer` (1s interval) that counts down from 60 after `sendCaptcha()` succeeds. Expose `captchaCooldown` (int) and `canSendCaptcha` (bool) properties.

**Alternatives considered**:
- Timer in QML → logic leaks into UI layer
- No cooldown → user spams API, gets rate-limited

**Rationale**: ViewModel owns the state. QML binds to properties. Follows existing pattern (SearchViewModel owns debounce timer).

### Decision 4: QR polling controlled by ViewModel with cancellation flag

**Choice**: `SettingsViewModel` exposes `pollQrLogin()` as a `QmlTask`. QML `Timer` calls it every 3 seconds. A `m_qrPollingActive` flag allows `cancelQrLogin()` to stop the coroutine early. `qrLoginStatus` property drives UI state.

**Alternatives considered**:
- VM owns the timer → VM needs to know when dialog opens/closes, coupling
- QML owns everything → VM can't enforce cancellation semantics

**Rationale**: QML timer is natural for periodic polling. VM flag ensures coroutine exits cleanly when dialog closes. `QPointer` prevents use-after-free if VM is destroyed mid-poll.

### Decision 5: Rename `loginNetease` to `loginByPassword`

**Choice**: Rename the existing method for clarity alongside new `loginByCaptcha` and QR methods.

**Rationale**: `loginNetease` is ambiguous now that there are three login methods. `loginByPassword` matches the pattern of `loginByCaptcha`.

### Decision 6: `Qt.labs.platform` FolderDialog for browse

**Choice**: Use `FolderDialog` from `Qt.labs.platform` for native folder picker.

**Alternatives considered**:
- `FileDialog` from `QtQuick.Dialogs` → file-oriented, not folder
- Manual text input only → poor UX

**Rationale**: Native look, `Qt6::LabsPlatform` is already installed. Fallback: user can still type path manually.

## Risks / Trade-offs

**[Risk]** QR code expires during polling → VM sets `qrLoginStatus = Expired`, QML shows "Refresh" button. User clicks to regenerate.

**[Risk]** Network failure during poll → VM sets `hasError = true`, polling continues. QML shows error toast. User can cancel or retry.

**[Risk]** `Qt.labs.platform` unavailable at runtime → `FolderDialog` fails silently. Mitigation: `TextField` is always editable as fallback.

**[Trade-off]** Polling interval (3s) vs API load → 3s is standard for NetEase QR login. Matches Android implementation.

**[Trade-off]** `m_qrPollingActive` flag vs coroutine cancellation → Flag is simpler than `QCoro::Task::cancel()`. Coroutine checks flag at each `co_await` point, exits within one poll cycle.
