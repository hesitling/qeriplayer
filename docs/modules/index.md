# NeriPlayer Qt Module Design Documents

## 1. Overview

This document describes the functional module design of the NeriPlayer Qt project in detail, referencing the module structure of the Android version of NeriPlayer and adapting it for Qt desktop development.

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
│                    Service/Manager Module                     │
│  PlayerManager, SearchManager, PlaylistManager...           │
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

### 5.1 Single Responsibility
Each module is responsible for only one specific functional area, avoiding confusion of responsibilities.

### 5.2 Dependency Inversion
High-level modules do not depend on low-level modules; both depend on abstract interfaces.

### 5.3 Interface Segregation
Use fine-grained interfaces to avoid bloated interface designs.

### 5.4 Open-Closed Principle
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
    virtual QFuture<LoginResult> authenticate() = 0;
    virtual void logout() = 0;
    
    // Search
    virtual QFuture<SearchResult> search(const QString &query,
                                         SearchType type) = 0;
    
    // Song details
    virtual QFuture<SongDetail> getSongDetail(const QString &id) = 0;
    virtual QFuture<QUrl> getPlaybackUrl(const QString &id) = 0;
    virtual QFuture<Lyrics> getLyrics(const QString &id) = 0;
    
    // Playlists
    virtual QFuture<PlaylistDetail> getPlaylistDetail(const QString &id) = 0;
    virtual QFuture<QList<Playlist>> getUserPlaylists() = 0;
};
```

### 6.2 Plugin Manager

```cpp
class PluginManager : public QObject {
    Q_OBJECT
public:
    static PluginManager* instance();
    
    void registerPlugin(std::unique_ptr<IMusicPlatformPlugin> plugin);
    IMusicPlatformPlugin* plugin(MusicPlatform platform) const;
    QList<IMusicPlatformPlugin*> plugins() const;
    
private:
    QHash<MusicPlatform, std::unique_ptr<IMusicPlatformPlugin>> m_plugins;
};
```

## 7. Technology Stack

| Category | Technology | Description |
|----------|-----------|-------------|
| **UI Framework** | Qt 6 Widgets / QML | Native desktop UI |
| **Build System** | CMake 3.16+ | Modern C++ build |
| **C++ Standard** | C++17 | Modern C++ features |
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

The module design of NeriPlayer Qt references the architecture of the Android version of NeriPlayer and adapts it for Qt desktop development. Each module has clear responsibilities and well-defined interfaces, facilitating development and maintenance. Through the plugin architecture and dependency injection, good extensibility is achieved.
