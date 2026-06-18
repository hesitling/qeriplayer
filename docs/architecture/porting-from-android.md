# Porting Architecture from Android QeriPlayer

## 1. Purpose

The Android QeriPlayer project is the primary feature reference for the Qt client. It should guide supported platforms, user flows, data formats, playback behavior, settings, and synchronization semantics.

The Qt client should not directly copy Android dependency shapes when they are tied to Android lifecycle, Compose state, Media3, WorkManager, DataStore, or application-wide Kotlin singletons.

## 2. Mapping Strategy

| Android QeriPlayer | Qt Client Equivalent | Porting Guidance |
|--------------------|----------------------|------------------|
| `Application` bootstrap | `QeriPlayerApplication` | Keep startup orchestration in `src/app/` |
| `AppContainer` service locator | `ServiceLocator` composition root | Register long-lived services, but inject them into consumers |
| `PlayerManager` singleton | `PlaybackController` + `IPlayerBackend` + `PlayerViewModel` | Split playback state, engine control, URL resolution, and UI state |
| Kotlin `Flow`/Compose state | Qt signals, properties, and QCoro tasks | Expose ViewModel state through `Q_PROPERTY` and signals |
| DataStore repositories | Settings repository backed by Qt/SQLite storage | Keep storage-specific code below repositories |
| OkHttp clients | `HttpClient` and platform API clients | Share network primitives, not UI or player state |
| Android `SongItem` UI models | Domain `Song`, `Playlist`, `Lyrics` models | Keep UI-specific formatting in ViewModels/adapters |

## 3. Dependency Rules

Qt modules should follow this direction:

```
UI → ViewModel → Repository / API Client → Core
```

Note: There is no dedicated service layer. ViewModels access repositories and API clients directly, following the Android QeriPlayer pattern. `PlaybackController` (in `player/`) is the exception — it encapsulates playback orchestration.

Rules:

- UI widgets should not call API clients, repositories, or player engines directly.
- ViewModels should depend on repository/API interfaces, not global singletons.
- ViewModels coordinate domain workflows and own business state (like Android's ViewModels).
- Repositories should map storage/API data into domain models.
- Core infrastructure should not depend on UI, ViewModels, or business modules.
- `ServiceLocator` should stay in the app composition root and should not become a general-purpose global access pattern.

## 4. Domain Boundary

Use domain models as the shared language between modules:

- `Song`, `Playlist`, `Album`, `Artist`, `Lyrics`, and playback result types belong in a domain/common layer.
- API modules map platform JSON into domain models.
- Repositories persist and retrieve domain models.
- ViewModels adapt domain models into UI display state.
- UI-specific models must not leak into API, repository, or player modules.

## 5. Async Boundary

Use `QCoro::Task<T>` for asynchronous APIs in new Qt code. Callback-style Qt signals remain appropriate for state notifications and long-lived subscriptions.

Avoid mixing `QFuture<T>` into public module interfaces unless a Qt API forces it internally; wrap those internals behind `QCoro::Task<T>` at module boundaries.

## 6. Implementation Order

1. Establish core infrastructure and domain types.
2. Port platform API clients behind QCoro interfaces.
3. Add repositories for settings, auth, playlists, history, and sync state.
4. Build `PlaybackController` for playback orchestration (queue, URL resolution, persistence).
5. Bind ViewModels to repos/API clients/PlaybackController with Qt properties and signals.
6. Build Widgets UI on top of ViewModels.
