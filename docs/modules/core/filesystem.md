# Filesystem Module (core/filesystem/)

## Overview

The filesystem module provides platform-specific application directory paths, safe file I/O with atomic writes, and file change monitoring.

## Source Files

```text
src/core/filesystem/
├── AppPaths.h / .cpp      # Platform-specific directory paths
├── FileUtils.h / .cpp     # Safe file read/write, atomic writes
└── FileWatcher.h / .cpp   # File change monitoring
```

## AppPaths

Provides platform-specific directory paths for application data, config, cache, and temp files. Directories are created automatically if they don't exist.

```cpp
class AppPaths {
public:
    static QString dataDir();   // Persistent data (database, downloads)
    static QString configDir(); // Configuration files
    static QString cacheDir();  // Cache (album art, temp API responses)
    static QString tempDir();   // Temporary files
};
```

### Platform Paths

| Method | Linux | macOS | Windows |
|--------|-------|-------|---------|
| `dataDir()` | `~/.local/share/QeriPlayer/` | `~/Library/Application Support/QeriPlayer/` | `%APPDATA%/QeriPlayer/` |
| `configDir()` | `~/.config/QeriPlayer/` | `~/Library/Application Support/QeriPlayer/` | `%APPDATA%/QeriPlayer/` |
| `cacheDir()` | `~/.cache/QeriPlayer/` | `~/Library/Caches/QeriPlayer/` | `%LOCALAPPDATA%/QeriPlayer/Cache/` |

## FileUtils

Static utility class for safe file operations.

```cpp
class FileUtils {
public:
    static bool ensureDir(const QString &path);
    static QByteArray readFile(const QString &path);
    static bool writeFile(const QString &path, const QByteArray &data);
    static QString lastError();
};
```

- **`ensureDir()`** — creates the directory and all parents. No-op if it already exists.
- **`readFile()`** — returns file contents, or empty `QByteArray` on error. Check `lastError()` for details.
- **`writeFile()`** — writes atomically (temp file in same directory, then rename). Prevents partial writes on crash.
- **`lastError()`** — returns the last error message (thread-local).

## FileWatcher

Monitors a file or directory for changes using `QFileSystemWatcher`.

```cpp
class FileWatcher : public QObject {
    Q_OBJECT
public:
    explicit FileWatcher(QObject *parent = nullptr);
    void watch(const QString &path);
    void stop();
    bool isWatching() const;

signals:
    void fileChanged(const QString &path);
};
```

## Usage

```cpp
// Get application directories
QString dbPath = AppPaths::dataDir() + "/qeriplayer.db";
QString logDir = AppPaths::dataDir() + "/logs";

// Safe file I/O
FileUtils::ensureDir(AppPaths::cacheDir());
FileUtils::writeFile(AppPaths::cacheDir() + "/meta.json", jsonData);
QByteArray data = FileUtils::readFile(AppPaths::cacheDir() + "/meta.json");

// Watch for changes
FileWatcher watcher;
watcher.watch(AppPaths::configDir() + "/settings.json");
QObject::connect(&watcher, &FileWatcher::fileChanged, [](const QString &path) {
    // Reload settings
});
```

## Testing

See `tests/core/TestFileSystem.cpp`.
