# NeriPlayer Qt Module Design Documents

## 1. Overview

This document describes the functional module design of the NeriPlayer Qt project in detail. Android NeriPlayer provides feature and behavior references, while the Qt client uses Qt-native module boundaries, domain models, and QCoro-based asynchronous APIs.

## 2. Module Overview

```
neriplayer-qt/
├── src/
│   ├── app/                    # Application entry module
│   ├── core/                   # Core infrastructure
│   │   ├── network/            # Network module
│   │   ├── database/           # Database module
│   │   ├── filesystem/         # File system module
│   │   ├── crypto/             # Crypto module
│   │   └── logger/             # Logger module
│   ├── domain/                 # Shared domain models and value types
│   ├── api/                    # API client modules
│   │   ├── netease/            # NetEase Cloud Music
│   │   ├── bilibili/           # Bilibili
│   │   ├── youtube/            # YouTube Music
│   │   └── qqmusic/            # QQ Music
│   ├── player/                 # Player module
│   ├── playlist/               # Playlist module
│   ├── download/               # Download module
│   ├── search/                 # Search module
│   ├── sync/                   # Sync module
│   ├── settings/               # Settings module
│   ├── ui/                     # UI module
│   │   ├── widgets/            # Custom widgets
│   │   ├── dialogs/            # Dialogs
│   │   ├── views/              # View pages
│   │   └── themes/             # Themes
│   ├── viewmodel/              # ViewModel module
│   └── utils/                  # Utility module
└── docs/                       # Documentation
```

## 3. Module Document Index

### 3.1 Core Modules
- [Application Module (app/)](app.md) - Application entry point and initialization
- [Core Infrastructure Module (core/)](core/index.md) - Network, database, file system, crypto, logger
  - [Network Module](core/network.md)
  - [Database Module](core/database.md)
  - [File System Module](core/filesystem.md)
  - [Crypto Module](core/crypto.md)
  - [Logger Module](core/logger.md)

### 3.2 Business Modules
- [API Module (api/)](api/index.md) - Multi-platform API client wrappers
  - [Common Types](api/common.md)
  - [NetEase Cloud Music](api/netease.md)
  - [Bilibili](api/bilibili.md)
  - [YouTube Music](api/youtube.md)
  - [QQ Music](api/qqmusic.md)
- Player Module (player/) - Audio playback, playback control, audio effects (to be written)
- Data Modules - Playlist, download, search, sync, settings (to be written)

### 3.3 Presentation Layer Modules
- UI Module (ui/) - Interface components, view pages, theme system (to be written)

## 4. Module Dependencies

```
┌─────────────────────────────────────────────────────────────┐
│                         UI Module                            │
│  MainWindow, Widgets, Dialogs, Themes                       │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                      ViewModel Module                        │
│  MainViewModel, PlayerViewModel, SearchViewModel...         │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                       Service Module                          │
│  PlayerService, SearchService, PlaylistService...            │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                      Repository Module                       │
│  MusicRepository, PlaylistRepository, SettingsRepository... │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                       API Module                             │
│  NeteaseClient, BilibiliClient, YouTubeMusicClient...       │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                       Core Module                            │
│  Network, Database, FileSystem, Crypto, Logger              │
└─────────────────────────────────────────────────────────────┘
```

## 5. Design Principles

### 5.1 Android as Feature Reference
Android NeriPlayer is used to identify features, data flows, and platform behavior. The Qt client should not copy Android-specific globals, lifecycle coupling, or UI-bound data models directly; see [Porting from Android NeriPlayer](../architecture/porting-from-android.md).

### 5.2 Domain Model Boundary
Shared models such as `Song`, `Playlist`, `Lyrics`, and playback result types belong in the domain layer. API clients, repositories, services, ViewModels, and UI adapters should exchange domain models instead of UI-specific types.

### 5.3 Single Responsibility
Each module is responsible for only one specific functional area, avoiding confusion of responsibilities.

### 5.4 Dependency Inversion
High-level modules do not depend on low-level modules; both depend on abstract interfaces.

### 5.5 Interface Segregation
Use fine-grained interfaces to avoid bloated interface designs.

### 5.6 Open-Closed Principle
Open for extension, closed for modification; support feature extension through plugin mechanisms.

## 6. Extension Point Design

### 6.1 Music Platform Plugin Interface

```cpp
// Music platform plugin interface
class IMusicPlatformPlugin {
public:
    virtual ~IMusicPlatformPlugin() = default;
    
    virtual QString name() const = 0;
    virtual MusicPlatform platform() const = 0;
    
    // Authentication
    virtual bool isAuthenticated() const = 0;
    virtual QCoro::Task<LoginResult> authenticate() = 0;
    virtual void logout() = 0;
    
    // Search
    virtual QCoro::Task<SearchResult> search(const QString &query,
                                             SearchType type) = 0;
    
    // Song details
    virtual QCoro::Task<SongDetail> getSongDetail(const QString &id) = 0;
    virtual QCoro::Task<QUrl> getPlaybackUrl(const QString &id) = 0;
    virtual QCoro::Task<Lyrics> getLyrics(const QString &id) = 0;
    
    // Playlists
    virtual QCoro::Task<PlaylistDetail> getPlaylistDetail(const QString &id) = 0;
    virtual QCoro::Task<QList<Playlist>> getUserPlaylists() = 0;
};
```

### 6.2 Plugin Registry

```cpp
class PluginRegistry : public QObject {
    Q_OBJECT
public:
    explicit PluginRegistry(QObject *parent = nullptr);
    
    void registerPlugin(std::unique_ptr<IMusicPlatformPlugin> plugin);
    IMusicPlatformPlugin* plugin(MusicPlatform platform) const;
    QList<IMusicPlatformPlugin*> plugins() const;
    
private:
    QHash<MusicPlatform, std::unique_ptr<IMusicPlatformPlugin>> m_plugins;
};
```

`PluginRegistry` should be created by the application bootstrap and injected into services that need platform discovery.

## 7. Technology Stack

| Category | Technology | Description |
|----------|-----------|-------------|
| **UI Framework** | Qt 6 Widgets / QML | Native desktop UI |
| **Build System** | CMake 3.16+ | Modern C++ build |
| **C++ Standard** | C++20 | Coroutine support |
| **Multimedia** | Qt Multimedia / VLC | Audio playback |
| **Network** | Qt Network / libcurl | HTTP requests |
| **Database** | SQLite | Local data storage |
| **Serialization** | nlohmann/json | JSON processing |
| **Configuration** | QSettings / TOML | Application configuration |
| **Logging** | spdlog / Qt Logging | Logging system |

## 8. Coroutine Support

This project uses C++20 coroutines and the QCoro library for asynchronous programming. See:
- [C++20 Coroutines & QCoro](../architecture/coroutines.md)

## 9. Summary

The module design of NeriPlayer Qt uses Android NeriPlayer as a feature reference while keeping Qt-specific module boundaries, domain models, and dependency injection. Each module has clear responsibilities and well-defined interfaces, facilitating development and maintenance. Through plugin registration and constructor injection, good extensibility is achieved without spreading global access patterns.
