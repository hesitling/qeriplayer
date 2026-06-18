# QeriPlayer Qt Documentation

This directory contains architecture design and module design documents for the QeriPlayer Qt project.

## Document Structure

```text
docs/
├── README.md              # This document
├── TODO.md                # Implementation roadmap
├── architecture/          # Architecture design documents
│   ├── index.md           # Architecture overview
│   ├── layers.md          # Layered architecture
│   ├── coroutines.md      # C++20 coroutines & QCoro
│   └── porting-from-android.md
└── modules/               # Module design documents
    ├── index.md           # Module overview & architecture layers
    ├── app.md             # Application module (QeriPlayerApplication, ServiceLocator)
    ├── domain.md          # Domain models (Song, Playlist, Lyrics, enums, etc.)
    ├── core/              # Core infrastructure modules
    │   ├── index.md       # Core module overview
    │   ├── network.md     # HttpClient, WebSocketClient, NetworkManager
    │   ├── database.md    # DatabaseManager (sqlite3), schema, migrations
    │   ├── filesystem.md  # AppPaths, FileUtils, FileWatcher
    │   ├── crypto.md      # Encryptor/Decryptor, SecureStorage, CryptoUtils
    │   └── logger.md      # Logger (spdlog), named loggers, daily rotation
    ├── api/               # API modules
    │   ├── index.md       # API module overview
    │   ├── common.md      # ApiError, ApiResult, IMusicPlatformPlugin
    │   ├── netease.md     # NeteaseClient, NeteaseCrypto, NeteaseParser
    │   ├── bilibili.md    # (planned)
    │   ├── youtube.md     # (planned)
    │   └── qqmusic.md     # (planned)
    └── repo/              # Repository layer
        └── index.md       # ISongRepository, IPlaylistRepository, etc.
```

## Core Modules

| Module | Document | Description |
|--------|----------|-------------|
| Application | [app.md](modules/app.md) | Entry point, ServiceLocator, initialization |
| Domain | [domain.md](modules/domain.md) | Shared value types (Song, Playlist, Lyrics, enums) |
| Network | [core/network.md](modules/core/network.md) | HttpClient (QCoro), WebSocketClient, NetworkManager |
| Database | [core/database.md](modules/core/database.md) | DatabaseManager (sqlite3), schema, migrations |
| Filesystem | [core/filesystem.md](modules/core/filesystem.md) | AppPaths, FileUtils, FileWatcher |
| Crypto | [core/crypto.md](modules/core/crypto.md) | Encryptor/Decryptor, SecureStorage, CryptoUtils |
| Logger | [core/logger.md](modules/core/logger.md) | spdlog wrapper, named loggers, daily rotation |
| Repositories | [repo/index.md](modules/repo/index.md) | Song, Playlist, PlayerState, Settings, PlayHistory |

## API Modules

| Module | Document | Description |
|--------|----------|-------------|
| Common Types | [api/common.md](modules/api/common.md) | ApiError, ApiResult, IMusicPlatformPlugin |
| NetEase | [api/netease.md](modules/api/netease.md) | NeteaseClient (implemented) |
| Bilibili | [api/bilibili.md](modules/api/bilibili.md) | (planned) |
| YouTube Music | [api/youtube.md](modules/api/youtube.md) | (planned) |
| QQ Music | [api/qqmusic.md](modules/api/qqmusic.md) | (planned) |

## References

- [QeriPlayer Android](https://github.com/cwuom/QeriPlayer)
- [QCoro](https://github.com/qcoro/qcoro)
- [Qt 6 Documentation](https://doc.qt.io/qt-6/)
