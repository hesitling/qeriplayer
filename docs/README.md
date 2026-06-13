# NeriPlayer Qt Documentation

This directory contains architecture design and module design documents for the NeriPlayer Qt project.

## Document Structure

```
docs/
├── README.md              # This document
├── architecture/          # Architecture design documents
│   ├── index.md           # Architecture overview
│   ├── layers.md          # Layered architecture
│   ├── coroutines.md      # C++20 coroutines & QCoro
│   ├── porting-from-android.md # Android-to-Qt architecture mapping
│   └── ...
└── modules/               # Module design documents
    ├── index.md           # Module overview
    ├── app.md             # Application module
    ├── core/              # Core infrastructure modules
    │   ├── index.md       # Core module overview
    │   ├── network.md     # Network module
    │   ├── database.md    # Database module
    │   ├── filesystem.md  # File system module
    │   ├── crypto.md      # Crypto module
    │   └── logger.md      # Logger module
    └── api/               # API modules
        ├── index.md       # API module overview
        ├── common.md      # Common types
        ├── netease.md     # NetEase Cloud Music
        ├── bilibili.md    # Bilibili
        ├── youtube.md     # YouTube Music
        └── qqmusic.md     # QQ Music
```

## Document Descriptions

### Architecture Design Documents

- [architecture/index.md](architecture/index.md) - Describes the overall architecture design of the project, including:
  - Design principles
  - Technology stack
  - Layered architecture
  - Data flow
  - Module dependencies
- [architecture/porting-from-android.md](architecture/porting-from-android.md) - Defines how to use Android NeriPlayer as a feature reference without copying Android-specific global dependencies

### Module Design Documents

Module design documents are organized in a hierarchical structure for easy reference and maintenance:

#### Core Modules

| Module | Document | Description |
|--------|----------|-------------|
| Application Module | [app.md](modules/app.md) | Application entry point and initialization |
| Network Module | [core/network.md](modules/core/network.md) | HTTP requests, WebSocket connections |
| Database Module | [core/database.md](modules/core/database.md) | SQLite operations, query building |
| File System Module | [core/filesystem.md](modules/core/filesystem.md) | File operations, path handling |
| Crypto Module | [core/crypto.md](modules/core/crypto.md) | Data encryption, secure storage |
| Logger Module | [core/logger.md](modules/core/logger.md) | Log recording, file management |

#### API Modules

| Module | Document | Description |
|--------|----------|-------------|
| Common Types | [api/common.md](modules/api/common.md) | Music types, search types |
| NetEase Cloud Music | [api/netease.md](modules/api/netease.md) | NetEase Cloud Music API client |
| Bilibili | [api/bilibili.md](modules/api/bilibili.md) | Bilibili API client |
| YouTube Music | [api/youtube.md](modules/api/youtube.md) | YouTube Music API client |
| QQ Music | [api/qqmusic.md](modules/api/qqmusic.md) | QQ Music API client |

## Design Principles

1. **Single Responsibility**: Each module is responsible for only one specific functional area
2. **Dependency Inversion**: High-level modules do not depend on low-level modules; both depend on abstract interfaces
3. **Interface Segregation**: Use fine-grained interfaces to avoid bloated interface designs
4. **Open-Closed Principle**: Open for extension, closed for modification; support feature extension through plugin mechanisms
5. **Coroutine First**: Use C++20 coroutines and QCoro to simplify asynchronous programming

## Technology Stack

| Category | Technology | Description |
|----------|-----------|-------------|
| C++ Standard | C++20 | Coroutine support |
| Coroutine Library | QCoro | Qt coroutine integration |
| UI Framework | Qt 6 | Native desktop UI |
| Build System | CMake 3.16+ | Modern build system |

## References

- [NeriPlayer Android](https://github.com/cwuom/NeriPlayer)
- [QCoro](https://github.com/qcoro/qcoro) - C++20 coroutines for Qt
- [Qt 6 Documentation](https://doc.qt.io/qt-6/)
- [CMake Documentation](https://cmake.org/cmake/help/latest/)
