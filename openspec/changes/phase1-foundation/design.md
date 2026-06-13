## Context

NeriPlayer Qt currently has:
- An application shell (`NeriPlayerApplication`) with a `ServiceLocator` for dependency injection.
- A network module (`HttpClient`, `WebSocketClient`, `NetworkMonitor`, `NetworkManager`) under `src/core/network/`.
- A `MainWindow` placeholder with menu/toolbar/status bar.

The project targets C++20 with QCoro for async operations, Qt 6.5+ for UI, SQLite for local storage, spdlog for logging, and nlohmann/json for JSON. The architecture follows MVVM with layered separation (presentation → business → data → infrastructure).

All planned features (API clients, player, playlists, sync, UI) need shared domain models, database access, logging, file management, and credential storage. None of these exist yet.

## Goals / Non-Goals

**Goals:**
- Provide shared domain models that all layers can use without circular dependencies.
- Provide a SQLite persistence layer with schema versioning and safe migrations.
- Provide structured logging with file rotation and category-based loggers.
- Provide cross-platform application directory resolution and safe file I/O.
- Provide encrypted credential storage for API tokens and cookies.
- Register all new services in `ServiceLocator` so they are available via constructor injection.

**Non-Goals:**
- Implementing API clients (Phase 2).
- Implementing the player or playback queue (Phase 3).
- Building any UI beyond what already exists (Phase 5).
- Supporting database engines other than SQLite.
- Supporting encryption algorithms beyond AES-256-GCM.
- Building a full plugin loading system (dynamic libraries). The `IMusicPlatformPlugin` interface is defined here as a header-only contract; actual plugin implementations come in Phase 2.

## Decisions

### D1: Domain models as plain value types, not QObject

Domain models (`Song`, `Album`, etc.) will be simple structs with no QObject overhead. They are copyable, movable, and serializable. Qt-specific adaptations (QVariant conversion) happen at the ViewModel boundary.

**Rationale:** QObject disables copy/move semantics and adds memory overhead. Domain models are data carriers passed across layers — they should be lightweight. Q_DECLARE_METATYPE and QVariant::fromValue can wrap them when needed for QML/property binding.

**Alternatives considered:**
- QObject-based models: Rejected — too heavy, prevents value semantics.
- Q_GADGET models: Possible for QML interop, but adds MOC dependency to domain layer. Can be added later at ViewModel boundary if needed.

### D2: SQLite via raw C API wrapped in a thin C++ class

Use SQLite's C API directly (bundled or system) wrapped in a `DatabaseManager` class. No ORM.

**Rationale:** The data model is simple (cache tables, settings, play history). An ORM adds complexity for little benefit. A thin wrapper with RAII statement handles and type-safe bind/get methods keeps things explicit and debuggable.

**Alternatives considered:**
- Qt SQL module (QSqlDatabase): Possible but adds a Qt module dependency for a thin wrapper. The raw API is straightforward enough.
- sqlpp11 / other C++ ORM: Overkill for this project's scope.

### D3: spdlog as the logging backend

Integrate spdlog with file sink (daily rotation, 7-day retention) and stdout sink (colored). Expose a `Logger` class that creates named loggers per category.

**Rationale:** spdlog is header-only (or compiled), fast, well-maintained, and supports structured logging. It is already listed in the project's tech stack.

**Alternatives considered:**
- Qt's qCDebug/qCWarning: Lacks file rotation and structured output.
- Boost.Log: Heavier dependency for the same features.

### D4: AES-256-GCM via platform crypto APIs

Use OpenSSL on Linux, CommonCrypto on macOS, and BCrypt on Windows. Provide a thin `Encryptor` abstraction. If OpenSSL is universally available (common in Qt builds), use it directly.

**Rationale:** AES-256-GCM provides authenticated encryption. Using platform-native crypto avoids bundling extra libraries where possible.

**Alternatives considered:**
- QCA (Qt Cryptographic Architecture): Adds a dependency that may not be available on all platforms.
- NaCl/libsodium: Another dependency to manage. OpenSSL is typically already linked by Qt Network.

### D5: Service registration via existing ServiceLocator

Extend `ServiceLocator` with getters for `DatabaseManager`, `Logger`, `FileSystem`, `SecureStorage`. Initialize them in `NeriPlayerApplication::initializeCoreServices()`.

**Rationale:** The project already uses `ServiceLocator` for dependency injection. No need to introduce a DI framework — the service locator pattern is sufficient for this scope.

## Risks / Trade-offs

- **Risk: OpenSSL availability varies by platform.** → Mitigation: Check for OpenSSL at CMake configure time; fall back to platform-specific backends if unavailable. Document the dependency.
- **Risk: SQLite migrations can corrupt data if buggy.** → Mitigation: Schema version table with forward-only migrations. Test each migration step. Provide a "reset database" escape hatch in settings.
- **Risk: Encryption key storage is itself a security problem.** → Mitigation: On first run, generate a random key and store it using OS-provided secure storage (Keychain on macOS, DPAPI on Windows, Secret Service on Linux). Fall back to a file with restricted permissions.
- **Risk: spdlog compiled mode adds build complexity.** → Mitigation: Use spdlog as header-only by default. Add a CMake option for compiled mode if build times become an issue.
