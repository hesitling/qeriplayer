## Context

Phase 1 delivered all core infrastructure: domain models, SQLite database, spdlog logger, filesystem abstraction, AES-256-GCM crypto, QCoro-based HTTP client, and a service locator. The application boots, initializes services, and can make async HTTP requests — but has no music platform API integration.

This change introduces the API layer: a shared interface and common types (`api/common/`), and the first platform client for NetEase Cloud Music (`api/netease/`). The NetEase client serves as the reference implementation — its patterns will be replicated for Bilibili, YouTube Music, and QQ Music in subsequent phases.

## Goals / Non-Goals

**Goals:**

- Define `IMusicPlatformPlugin` as the abstract contract all platform clients implement.
- Define shared API types (`ApiError`, `ApiResult<T>`, `VoidResult`, `LoginResult`, etc.) used by every client.
- Implement a complete NetEase Cloud Music client covering auth, search, song/playlist/album/artist operations, lyrics, and playback URL resolution.
- Prove the plugin architecture end-to-end: client → service locator → coroutines.
- Provide unit tests for crypto (encryption round-trips) and parser (JSON → domain models).

**Non-Goals:**

- Bilibili, YouTube Music, or QQ Music clients (future changes).
- Repository/cache layer wrapping API calls with SQLite (future change — `CachedMusicRepository`).
- UI integration or ViewModel binding (Phase 4+).
- Download manager integration (Phase 6).
- Rate limiting or retry policies (can be added later in `HttpClient`).
- Self-hosted NeteaseCloudMusicApi server support (out of scope for now; hardcoded API base URL).

## Decisions

### D1: API base URL — hardcoded default, configurable

**Decision:** Default to `https://music.163.com/api` (official endpoint). Expose a setter `setBaseUrl(const QUrl &)` on `NeteaseClient` for self-hosted instances.

**Rationale:** Most users hit the official endpoint. Self-hosted NeteaseCloudMusicApi is common in the Chinese community but not the default. A setter avoids config-file complexity while still supporting it.

**Alternative considered:** Config file or `SettingsRepository` lookup at construction. Rejected — introduces a dependency on settings before settings exist.

### D2: WeAPI encryption — OpenSSL-based implementation

**Decision:** Implement WeAPI encryption directly using OpenSSL (already linked). AES-128-CBC + RSA with hardcoded keys matching the known NetEase web client crypto. WeAPI encryption is one-way (client → server) — there is no client-side decryption.

**Rationale:** OpenSSL is already a dependency. The WeAPI encryption is well-documented and stable. No need for an additional library. Responses arrive over standard HTTPS and require no client-side crypto.

**Alternative considered:** Wrapping a third-party NeteaseCloudMusicApi crypto library. Rejected — adds dependency for ~200 lines of code we can own.

**Test strategy:** Verify `weapiEncrypt` output against known test vectors (with fixed IV for determinism) rather than round-trip encrypt/decrypt.

### D3: Cookie/session persistence — SecureStorage

**Decision:** Store authentication cookies and CSRF token in `SecureStorage` (Phase 1 module). Load on client construction, save on successful login, clear on logout.

**Rationale:** `SecureStorage` already provides AES-256-GCM encrypted persistence. Cookies are sensitive credentials — they must not be stored in plaintext.

**Alternative considered:** Separate cookie jar file. Rejected — duplicates what `SecureStorage` already does, and cookies would be unencrypted.

### D4: ApiResult<T> — variant-based result type

**Decision:** `ApiResult<T>` holds either a `T` value or an `ApiError`. No exceptions for API-layer errors — all errors are returned as values.

**Rationale:** API errors are expected (network failures, auth expiry, rate limits). Using result types makes error handling explicit at call sites. Consistent with the Rust/Go-style error handling that works well with coroutines.

**Alternative considered:** Throwing `ApiError` exceptions. Rejected — exceptions for expected control flow are an anti-pattern, and the call sites would need try/catch anyway.

### D5: NeteaseParser — separate parsing layer

**Decision:** JSON response parsing lives in `NeteaseParser` (static methods), separate from `NeteaseClient` (network orchestration).

**Rationale:** Parsing logic is large (~500 lines for all endpoints). Keeping it separate improves testability — parser tests use recorded JSON fixtures without needing network mocks. Follows Single Responsibility Principle.

### D6: IMusicPlatformPlugin — minimal interface for now

**Decision:** The interface defines search, song detail, playback URL, lyrics, and auth status. Playlist CRUD and user operations are on the concrete class only.

**Rationale:** Not all platforms support the same operations (e.g., QQ Music has no playlist CRUD). The interface should only include operations that make sense across all platforms. Platform-specific operations live on the concrete client.

**Alternative considered:** A fat interface with all operations, throwing "not supported" for some platforms. Rejected — violates ISP, leads to runtime surprises.

### D7: ApiError — dual code classification (HTTP + NetEase body)

**Decision:** `ApiError` SHALL classify errors using both HTTP status codes and NetEase-specific JSON body codes. The body code takes precedence when the response is parseable. Mapping:

| Method | HTTP codes | NetEase body codes |
|--------|-----------|--------------------|
| `isNetworkError()` | Connection failure (-1) | — |
| `isAuthError()` | 401, 403 | -10 (auth expired), -460 (cheating detection) |
| `isRateLimitError()` | 429 | -429 |
| `isNotFoundError()` | 404 | — |

**Rationale:** NetEase uses custom error codes in the JSON body (e.g., `-10` for expired session, `-460` for anti-cheat). Relying only on HTTP status codes would miss these. Storing the body code as primary makes classification accurate.

### D8: Logger convention for `api/` layer

**Decision:** Use `Logger::get("api")` for logging in the `api/` module (not `qWarning()`).

**Rationale:** `qWarning()` is reserved for `core/` modules that should not depend on the Logger abstraction. The `api/` layer is a higher-level module and should use the application logger for consistency and configurability.

### D9: `isAuthenticated()` — synchronous by design

**Decision:** `IMusicPlatformPlugin::isAuthenticated()` returns `bool` synchronously.

**Rationale:** For NetEase, auth state is an in-memory cookie check (no network call). For platforms with token expiry, the concrete class can cache the state and refresh asynchronously in the background. Making the interface async would force all callers to `co_await` a simple boolean check. If a platform truly needs async auth verification, it can provide a separate `refreshAuth()` method on the concrete class.

## Risks / Trade-offs

**[Risk] NetEase API changes or breaks.** The NetEase Cloud Music API is undocumented and subject to change without notice.
→ **Mitigation:** Record JSON fixtures for all tested endpoints. Parser tests will catch breakage early. The `NeteaseParser` layer isolates parsing from networking.

**[Risk] WeAPI encryption keys rotate.** The hardcoded crypto keys could change if NetEase updates their web client.
→ **Mitigation:** Keys are isolated in `NeteaseCrypto` constants — easy to update. Monitor the open-source community for key changes.

**[Risk] Rate limiting from NetEase servers.** Excessive requests could trigger IP bans.
→ **Mitigation:** Not addressed in this change. Future work: add a rate limiter in `HttpClient` or per-client. Document the risk in code comments.

**[Trade-off] Hardcoded API base URL vs. config-driven.** Hardcoding is simpler but requires a code change for self-hosted instances.
→ **Mitigation:** The `setBaseUrl()` setter provides an escape hatch without over-engineering.

**[Trade-off] Synchronous cookie load vs. lazy load.** Cookies are loaded from `SecureStorage` at construction time.
→ **Mitigation:** `SecureStorage` is fast (in-memory after first load). Construction cost is negligible.
