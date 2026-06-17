# Core Infrastructure Module (core/)

## Overview

The core infrastructure module provides the fundamental services required by all other modules: network communication, database access, file operations, cryptography, and logging.

## Module Composition

```
src/core/
├── network/     # HTTP client, WebSocket, network monitoring
├── database/    # SQLite wrapper with schema migrations
├── filesystem/  # App paths, safe file I/O, file watcher
├── crypto/      # AES-256-GCM encryption, secure storage, hashing
└── logger/      # spdlog-based logging with daily rotation
```

## Submodule Documents

- [Network Module](network.md) — HttpClient (QCoro), WebSocketClient, NetworkManager, NetworkMonitor
- [Database Module](database.md) — DatabaseManager (sqlite3), parameterized queries, transactions, migrations
- [Filesystem Module](filesystem.md) — AppPaths, FileUtils (atomic writes), FileWatcher
- [Crypto Module](crypto.md) — Encryptor/Decryptor (AES-256-GCM), SecureStorage, CryptoUtils (SHA-256)
- [Logger Module](logger.md) — spdlog wrapper with named loggers, daily file rotation, colored console

## Module Dependencies

```
┌──────────────────────────────────────────────┐
│              Application Module (app/)        │
└──────────────────────────────────────────────┘
                        ↓
┌──────────────────────────────────────────────┐
│               Core Module (core/)             │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐      │
│  │ Network  │ │ Database │ │FileSystem│      │
│  └──────────┘ └──────────┘ └──────────┘      │
│  ┌──────────┐ ┌──────────┐                   │
│  │  Crypto  │ │  Logger  │                   │
│  └──────────┘ └──────────┘                   │
└──────────────────────────────────────────────┘
```

## Technology Stack

| Module | Technology |
|--------|-----------|
| Network | Qt Network + QCoro |
| Database | sqlite3 C API |
| Filesystem | Qt Core (QDir, QFile, QFileSystemWatcher) |
| Crypto | OpenSSL (AES-256-GCM, SHA-256) |
| Logging | spdlog (daily_file_sink_mt, stdout_color_sink_mt) |
