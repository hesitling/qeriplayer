## 1. API / Auth semantics

- [x] 1.1 Add an explicit validated cookie import entry point in the NetEase auth/client layer
- [x] 1.2 Ensure cookie import runs `ensureWeapiSession()` before validation completes
- [x] 1.3 Validate imported cookies via current-account/profile fetch before treating the client as authenticated
- [x] 1.4 Add a local-only session clear path that removes persisted cookies and in-memory auth state without calling remote logout
- [x] 1.5 Ensure restored persisted sessions can fetch account/profile info for UI hydration

## 2. ViewModel — SettingsViewModel auth simplification

- [x] 2.1 Remove password, SMS captcha, and QR-code login methods from the settings auth surface
- [x] 2.2 Add a cookie import method exposed to QML (for example `importNeteaseCookie(...)`)
- [x] 2.3 Add validated import flow: clear previous error, import cookie, validate account, set username/auth state on success
- [x] 2.4 Clear local auth state and surface a user-facing error when cookie validation fails
- [x] 2.5 Change `logoutNetease()` semantics to local-only session clearing
- [x] 2.6 Add profile hydration on initialization/refresh when persisted cookies already exist
- [x] 2.7 Remove QR/captcha state properties and timers that are no longer used

## 3. QML — Settings auth UI

- [x] 3.1 Replace the tabbed `LoginDialog.qml` with a cookie import dialog
- [x] 3.2 Provide a multiline input for pasting the NetEase cookie string
- [x] 3.3 Submit the pasted cookie through the new ViewModel import method
- [x] 3.4 Close the dialog only after validated import succeeds
- [x] 3.5 Update account-section wording and actions to reflect local-only sign-out
- [x] 3.6 Remove password, SMS, QR, cooldown, and polling UI

## 4. Tests

- [x] 4.1 Replace ViewModel tests for password/SMS/QR auth with cookie import and validation coverage
- [x] 4.2 Add tests for local-only session clearing
- [x] 4.3 Add tests for restored-session profile hydration
- [x] 4.4 Update QML tests for the cookie import dialog and revised account actions

## 5. Cleanup

- [x] 5.1 Remove no-longer-used QR/captcha/domain artifacts if they are fully unreferenced
- [x] 5.2 Update or supersede outdated settings/auth OpenSpec requirements describing password, SMS, and QR login
