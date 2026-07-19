## Context

The current NetEase auth stack has two different realities:

1. The user-facing QML settings flow is built around password, SMS captcha, and QR login.
2. The existing direct-cookie path (`setCookies(...)` + `ensureWeapiSession()`) is already real and is the path used by end-to-end tests.

That mismatch creates unnecessary UI complexity and keeps the app coupled to multiple undocumented auth endpoints. We want to converge the UI, ViewModel, and API surface on the simpler operational model the codebase already trusts in practice.

## Goals / Non-Goals

**Goals**
- Make cookie import the only NetEase auth flow exposed in settings
- Validate imported cookies before exposing the session as logged in
- Keep sign-out local-only
- Hydrate username/profile state when a persisted cookie session is restored

**Non-Goals**
- Automatic browser cookie extraction
- Remote NetEase logout or token revocation
- Multi-platform auth work

## Decisions

### D1: Cookie import is the only user-facing NetEase auth path

**Choice:** Replace the three-tab login dialog with a single cookie import dialog.

The dialog accepts a pasted NetEase cookie string from the user and delegates processing to the ViewModel.

**Rationale:**
- Aligns the UI with the already-proven E2E workflow
- Removes the highest-maintenance parts of the current auth surface
- Avoids QR-specific third-party dependencies and SMS-specific rate-limit behavior

### D2: Imported cookies must be validated before auth state becomes true

**Choice:** Import is a two-step flow:

```text
paste cookie
  -> store/apply candidate cookie
  -> ensure WeAPI session
  -> fetch current account
  -> success: persist authenticated session + hydrate profile
  -> failure: clear local auth state + surface error
```

**Rationale:** The current client can become optimistically authenticated as soon as cookies are set. That is acceptable as an internal helper, but not as the main product behavior. Validation makes the settings UI honest.

### D3: Sign-out is local-only

**Choice:** The settings sign-out action clears persisted cookies and in-memory auth/profile state locally. It does not call a remote NetEase logout endpoint.

**Rationale:** Imported cookies may come from another browser or device session. Revoking them remotely would be surprising and potentially destructive outside the app.

### D4: Restored sessions hydrate profile state lazily through account fetch

**Choice:** If the client restores cookies from storage and appears authenticated, the ViewModel should fetch account/profile details during settings initialization or explicit refresh so the settings UI can display a username.

**Rationale:** Cookie persistence alone is not enough for a good UI. The current `neteaseUsername` value is populated mainly during interactive login flows and can be empty after restart.

### D5: Keep cookie parsing responsibilities in the API/client boundary

**Choice:** The ViewModel passes the raw pasted cookie string to the NetEase client/auth layer. That layer remains responsible for normalizing cookies, ensuring `__csrf` if needed, and validating the session.

**Rationale:** Cookie formatting and auth validation are transport concerns, not UI concerns.

## Risks / Trade-offs

| Risk / Trade-off | Notes |
|------------------|-------|
| Cookie import is more technical than password login | Acceptable if the app is optimized for power users and lower maintenance |
| Imported cookies may be missing `__csrf` | `ensureWeapiSession()` remains part of the validation flow |
| Persisted cookies may expire silently between launches | Profile hydration/validation catches this and can clear stale local state |
| Existing specs and tests describe removed login methods | This change explicitly replaces those requirements |

## Proposed Flow

```text
SettingsView
   │
   ▼
CookieImportDialog
   │
   ▼
SettingsViewModel::importNeteaseCookie(cookie)
   │
   ▼
Netease auth/client layer
   ├── apply candidate cookie
   ├── ensureWeapiSession()
   └── getCurrentUserAccount()
           │
           ├── success -> authenticated + profile hydrated + persisted
           └── failure -> local session cleared + error shown
```

## Open Questions

- Whether the UI button label should stay `Logout` for familiarity or change to something more explicit like `Clear Session`
- Whether the cookie dialog should include a single multiline field only or also show a short format hint/examples
