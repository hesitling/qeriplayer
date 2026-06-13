# Core Infrastructure Module (core/)

## 1. Overview

The core infrastructure module provides the fundamental services required for the application to run, including network communication, data persistence, file operations, data encryption, and logging.

## 2. Module Composition

```
src/core/
├── network/          # Network module
├── database/         # Database module
├── filesystem/       # File system module
├── crypto/           # Crypto module
└── logger/           # Logger module
```

## 3. Submodule Documents

- [Network Module (network/)](network.md) - HTTP requests, WebSocket connections, network status monitoring
- [Database Module (database/)](database.md) - SQLite operations, query building, data migration
- [File System Module (filesystem/)](filesystem.md) - File operations, path handling, file monitoring
- [Crypto Module (crypto/)](crypto.md) - Data encryption, secure storage, hash computation
- [Logger Module (logger/)](logger.md) - Log recording, file management, log rotation

> Each submodule document has been split into separate files for easy reference and maintenance.

## 4. Module Dependencies

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Module (app/)                 │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                       Core Module (core/)                    │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐        │
│  │ Network  │ │ Database │ │FileSystem│ │  Crypto  │        │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘        │
│                      ┌──────────┐                           │
│                      │  Logger  │                           │
│                      └──────────┘                           │
└─────────────────────────────────────────────────────────────┘
```

## 5. Design Principles

- **Single Responsibility**: Each submodule is responsible for only one specific foundational function
- **Unified Interface**: Provides a consistent API design style
- **Testability**: Supports dependency injection and mocking
- **Cross-Platform**: Compatible with Windows, macOS, Linux

## 6. Technology Stack

| Module | Technology | Description |
|--------|-----------|-------------|
| Network | Qt Network | HTTP/WebSocket |
| Database | SQLite | Local persistence |
| File System | Qt Core | File operations |
| Crypto | OpenSSL/QCA | Data encryption |
| Logging | spdlog/Qt | Log recording |
