## Why

The current NetEase authentication flow is spread across password, SMS captcha, and QR code methods in the QML settings UI, while the codebase already has a direct cookie path and the real end-to-end tests use cookies exclusively. That leaves the product surface larger than necessary and concentrates maintenance on the least stable part of an unofficial NetEase integration: interactive login endpoints.

We want to simplify the app to a single NetEase authentication model based on importing cookies, validating them before accepting the session, and treating sign-out as a local-only action that clears stored credentials without revoking the remote NetEase session.

## What Changes

- Replace the multi-tab NetEase login dialog with a cookie import dialog
- Remove password, SMS captcha, and QR-code authentication from the user-facing settings flow
- Add a validated cookie import flow in `SettingsViewModel`
- Make NetEase sign-out local-only: clear local session state and persisted cookies without calling remote logout
- Ensure restored cookie sessions can hydrate account/profile state for the settings page

## Non-goals

- Reworking non-NetEase authentication
- Replacing NetEase cookie persistence storage
- Adding browser automation or cookie extraction helpers
- Changing unrelated settings UI sections

## Capabilities

### Modified Capabilities
- `qml-settings-view`: replace password/SMS/QR login UI with cookie import UI and local session clearing
- `vm-settings`: replace multi-method auth helpers with cookie import, validation, profile hydration, and local-only logout semantics
- `api-netease`: support validated cookie import and local-only session clearing

## Impact

| Layer | Impact |
|-------|--------|
| QML | `LoginDialog.qml` becomes cookie-focused; settings account section wording changes |
| ViewModel | remove QR/captcha/password state; add cookie import and validation flow |
| API | add explicit validated cookie import path and local-only clear-session path |
| Tests | replace multi-method auth tests with cookie import and local clear-session coverage |

## Breaking / Behavioral Changes

- Users will no longer log in through password, SMS, or QR code inside the app
- NetEase sign-out becomes local-only and does not revoke the upstream session
- Imported cookies are not treated as authenticated until account validation succeeds
