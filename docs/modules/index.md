# QeriPlayer Qt Module Design

## Overview

QeriPlayer Qt is a multi-platform music player built with Qt 6, C++20, and QCoro. Android QeriPlayer serves as the feature reference; the Qt client uses Qt-native module boundaries, domain models, and coroutine-based async APIs.

## Module Structure

```text
src/
├── app/                    # Application entry, ServiceLocator
├── core/                   # Infrastructure
│   ├── network/            # HttpClient (QCoro), WebSocketClient, NetworkManager
│   ├── database/           # DatabaseManager (sqlite3), migrations
│   ├── filesystem/         # AppPaths, FileUtils, FileWatcher
│   ├── crypto/             # Encryptor/Decryptor, SecureStorage, CryptoUtils
│   └── logger/             # Logger (spdlog), named loggers, daily rotation
├── domain/                 # Shared value types (Song, Playlist, Lyrics, etc.)
├── api/                    # Platform API clients
│   ├── common/             # ApiError, ApiResult, IMusicPlatformPlugin
│   └── netease/            # NeteaseClient, NeteaseCrypto, NeteaseParser
├── repo/                   # Repository layer (SQLite-backed persistence)
│   ├── ISongRepository     # Song CRUD
│   ├── IPlaylistRepository # Playlist CRUD + song membership
│   ├── IPlayerStateRepository # Singleton player state
│   ├── ISettingsRepository # Key-value settings
│   └── IPlayHistoryRepository # Play event tracking
├── main.cpp
└── mainwindow.h / .cpp
```

## Module Documents

### Core Infrastructure
- [Core Module Overview](core/index.md)
- [Network](core/network.md) — HttpClient, WebSocketClient, NetworkManager, NetworkMonitor
- [Database](core/database.md) — DatabaseManager (sqlite3), schema, migrations
- [Filesystem](core/filesystem.md) — AppPaths, FileUtils, FileWatcher
- [Crypto](core/crypto.md) — Encryptor, Decryptor, SecureStorage, CryptoUtils
- [Logger](core/logger.md) — spdlog-based logging with named loggers

### Domain & Data
- [Domain Models](domain.md) — Song, Album, Artist, Playlist, Lyrics, enums, etc.
- [Repository Layer](repo/index.md) — ISongRepository, IPlaylistRepository, etc.

### API
- [API Module Overview](api/index.md)
- [Common Types](api/common.md) — ApiError, ApiResult, IMusicPlatformPlugin
- [NetEase](api/netease.md) — NeteaseClient, NeteaseCrypto, NeteaseParser

### Application
- [App Module](app.md) — QeriPlayerApplication, ServiceLocator

## Architecture Layers

```text
┌──────────────────────────────────────┐
│              UI Layer                 │  (planned)
├──────────────────────────────────────┤
│           ViewModel Layer            │  (planned)
├──────────────────────────────────────┤
│      PlaybackController (player/)    │  Playback orchestration
├──────────────────────────────────────┤
│          Repository Layer            │  SongRepository, PlaylistRepository, ...
├──────────────────────────────────────┤
│             API Layer                │  NeteaseClient, ...
├──────────────────────────────────────┤
│           Domain Layer               │  Song, Playlist, Lyrics, ...
├──────────────────────────────────────┤
│            Core Layer                │  Network, Database, FileSystem, Crypto, Logger
└──────────────────────────────────────┘
```

Note: There is no dedicated service layer. ViewModels access repositories and API clients directly, following the Android QeriPlayer pattern. `PlaybackController` is the exception — it encapsulates complex playback orchestration.

## Technology Stack

| Category | Technology |
|----------|-----------|
| Language | C++20 |
| UI Framework | Qt 6.5+ (Widgets) |
| Async | QCoro 0.10+ (C++20 coroutines) |
| Build System | CMake 3.16+ |
| Database | SQLite 3.35+ (raw sqlite3 C API) |
| JSON | nlohmann/json 3.11+ |
| Logging | spdlog 1.12+ |
| Crypto | OpenSSL (AES-256-GCM, SHA-256) |

## Design Principles

- **Domain models in `domain/`** — shared across all layers, no QObject dependency.
- **Interfaces in `IXxxRepository`** — dependency inversion for testability.
- **QCoro for async** — all async functions return `QCoro::Task<T>`, use `co_await`/`co_return`.
- **No globals** — `ServiceLocator` is owned by `QeriPlayerApplication`, passed via constructor injection.
- **Platform-agnostic plugin interface** — `IMusicPlatformPlugin` defines the cross-platform contract.
