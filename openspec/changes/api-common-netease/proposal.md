## Why

Phase 1 delivered the foundation layer (domain models, database, logger, filesystem, crypto, network). The application can bootstrap and make HTTP requests, but has no way to talk to any music platform. Without API clients, nothing downstream — search aggregation, playback, playlists, downloads — can function. The NetEase Cloud Music API is the primary platform (largest user base, richest feature set) and serves as the proving ground for the plugin architecture that all other platform clients will follow.

## What Changes

- Add shared API types (`ApiError`, `ApiResult<T>`, `PlaybackUrl`, `LoginResult`, `QrCodeData`, `PlayHistory`) used by all platform clients.
- Add `IMusicPlatformPlugin` abstract interface that every platform client implements.
- Add `NeteaseCrypto` — WeAPI/EAPI encryption helpers required by the NetEase API.
- Add `NeteaseParser` — JSON-to-domain-model parsing for all NetEase API responses.
- Add `NeteaseClient` implementing `IMusicPlatformPlugin` with full support for:
  - Authentication (phone, email, QR code)
  - Search (songs, playlists, albums, artists, hot searches)
  - Song detail, playback URL resolution, lyrics retrieval
  - Playlist detail, user playlists, CRUD operations
  - Recommendations (daily, personalized FM, new songs)
- Register `NeteaseClient` in `ServiceLocator` and wire it into `NeriPlayerApplication`.
- Add unit tests for NeteaseCrypto (encryption round-trips) and NeteaseParser (JSON parsing with recorded fixtures).

## Capabilities

### New Capabilities

- `api-common`: Shared API types, error handling, and the `IMusicPlatformPlugin` interface that all platform clients implement.
- `netease-client`: NetEase Cloud Music API client — authentication, search, song/playlist/album/artist operations, lyrics, playback URL resolution, and recommendations.

### Modified Capabilities

None — no existing specs to modify.

## Impact

- New files under `src/api/common/` and `src/api/netease/`.
- Modified files: `src/app/NeriPlayerApplication.cpp` (register NeteaseClient), `CMakeLists.txt` (new source files).
- New test files under `tests/api/` for crypto and parser unit tests.
- Dependency on `HttpClient` (Phase 1) for all network calls.
- No breaking changes — purely additive to the existing codebase.
