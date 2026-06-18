## Why

QeriPlayer Qt has an application shell and core network module, but lacks the foundational infrastructure that every other module depends on: domain models, database persistence, logging, file system abstraction, and credential encryption. Without these, no feature layer (API clients, player, playlists, UI) can proceed. This change establishes the bedrock.

## What Changes

- Add shared domain models (`Song`, `Album`, `Artist`, `Playlist`, `Lyrics`, search/result types, and enumerations) used across all layers.
- Add a `Database` core module wrapping SQLite with schema management, migrations, and a type-safe query helper.
- Add a `Logger` core module wrapping spdlog with file rotation, console output, and category-based loggers.
- Add a `FileSystem` core module for cross-platform application paths, safe file I/O, and file watching.
- Add a `Crypto` core module for AES-256-GCM encryption and a secure credential store.
- Update `ServiceLocator` and `QeriPlayerApplication` to register and initialize all new core services.

## Capabilities

### New Capabilities

- `domain-models`: Shared value types and data structures (Song, Album, Artist, Playlist, Lyrics, SearchResult, enums) used by every layer.
- `database`: SQLite persistence layer with schema management, migrations, and query helpers.
- `logger`: spdlog-based logging with file rotation, console sink, and named loggers.
- `filesystem`: Cross-platform application paths, atomic file I/O, and file change monitoring.
- `crypto`: AES-256-GCM encryption and encrypted credential storage.

### Modified Capabilities

None — no existing specs to modify.

## Impact

- New files under `src/domain/`, `src/core/database/`, `src/core/logger/`, `src/core/filesystem/`, `src/core/crypto/`.
- Modified files: `src/app/ServiceLocator.h`, `src/app/QeriPlayerApplication.h/.cpp`.
- CMakeLists.txt updates for new source files and dependencies (spdlog, OpenSSL/QCA).
- No breaking changes — purely additive to the existing codebase.
