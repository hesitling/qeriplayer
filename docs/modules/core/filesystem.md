# File System Module (filesystem/)

## 1. Overview

The file system module provides file and directory operations, path handling, and file monitoring capabilities.

## 2. Directory Structure

```
src/core/filesystem/
├── FileSystemManager.h/.cpp  # File system manager
├── PathUtils.h/.cpp          # Path utilities
└── FileWatcher.h/.cpp        # File watcher
```

## 3. Main Class Design

### 3.1 FileSystemManager

File system manager.

```cpp
class FileSystemManager : public QObject {
    Q_OBJECT
public:
    explicit FileSystemManager(QObject *parent = nullptr);
    
    // ==================== Application Directories ====================
    
    QString appDataDir() const;
    QString appConfigDir() const;
    QString appCacheDir() const;
    QString appLogDir() const;
    QString appDownloadDir() const;
    
    // ==================== File Operations ====================
    
    // Check existence
    bool fileExists(const QString &path) const;
    bool dirExists(const QString &path) const;
    
    // Create directories
    bool createDir(const QString &path);
    bool createDirRecursive(const QString &path);
    
    // Delete
    bool removeFile(const QString &path);
    bool removeDir(const QString &path);
    bool removeDirRecursive(const QString &path);
    
    // Copy/Move
    bool copyFile(const QString &src, const QString &dst);
    bool copyDir(const QString &src, const QString &dst);
    bool moveFile(const QString &src, const QString &dst);
    bool moveDir(const QString &src, const QString &dst);
    
    // Rename
    bool rename(const QString &oldPath, const QString &newPath);
    
    // ==================== File Read/Write ====================
    
    // Binary read/write
    QByteArray readFile(const QString &path) const;
    bool writeFile(const QString &path, const QByteArray &data);
    bool appendFile(const QString &path, const QByteArray &data);
    
    // Text read/write
    QString readTextFile(const QString &path, 
                         const QString &codec = "UTF-8") const;
    bool writeTextFile(const QString &path, const QString &text,
                       const QString &codec = "UTF-8");
    
    // JSON operations
    QJsonObject readJsonFile(const QString &path) const;
    bool writeJsonFile(const QString &path, const QJsonObject &json);
    QJsonArray readJsonArrayFile(const QString &path) const;
    bool writeJsonArrayFile(const QString &path, const QJsonArray &json);
    
    // ==================== File Information ====================
    
    qint64 fileSize(const QString &path) const;
    QDateTime fileModified(const QString &path) const;
    QDateTime fileCreated(const QString &path) const;
    QString fileMd5(const QString &path) const;
    
    // File listing
    QStringList listFiles(const QString &dir, 
                          const QStringList &filters = {}) const;
    QStringList listDirs(const QString &dir) const;
    QStringList listAll(const QString &dir, 
                        const QStringList &filters = {}) const;
    QStringList listFilesRecursive(const QString &dir,
                                   const QStringList &filters = {}) const;
    
    // ==================== Disk Space ====================
    
    qint64 availableSpace(const QString &path) const;
    qint64 totalSpace(const QString &path) const;
    qint64 usedSpace(const QString &path) const;
    
    // ==================== File Monitoring ====================
    
    void watchFile(const QString &path);
    void watchDir(const QString &path);
    void unwatchFile(const QString &path);
    void unwatchDir(const QString &path);
    void unwatchAll();
    
    // ==================== Temporary Files ====================
    
    QString createTempFile(const QString &templateName = {});
    QString createTempDir(const QString &templateName = {});
    
signals:
    void fileChanged(const QString &path);
    void dirChanged(const QString &path);
    
private:
    std::unique_ptr<QFileSystemWatcher> m_watcher;
};
```

### 3.2 PathUtils

Path utility class.

```cpp
class PathUtils {
public:
    // Path joining
    static QString join(const QString &basePath, 
                        const QString &relativePath);
    static QString join(const QString &part1, const QString &part2,
                        const QString &part3 = {});
    
    // Get file name
    static QString fileName(const QString &path);
    static QString fileNameWithoutExtension(const QString &path);
    
    // Get file extension
    static QString fileExtension(const QString &path);
    
    // Get directory name
    static QString dirName(const QString &path);
    
    // Normalize path
    static QString normalize(const QString &path);
    
    // Convert to absolute path
    static QString absolutePath(const QString &path);
    
    // Check if path is absolute
    static bool isAbsolute(const QString &path);
    
    // Get relative path
    static QString relativePath(const QString &path, 
                                const QString &basePath);
    
    // Safe file name (remove invalid characters)
    static QString safeFileName(const QString &name);
    
    // Check if path is under directory
    static bool isPathUnder(const QString &path, const QString &dir);
    
    // Get home directory
    static QString homeDir();
    
    // Get temp directory
    static QString tempDir();
    
    // Get current working directory
    static QString currentDir();
    
    // Path separator
    static QString separator();
    
    // Path comparison
    static bool pathsEqual(const QString &path1, const QString &path2);
};
```

## 4. Usage Examples

### 4.1 File Read/Write

```cpp
auto *fs = ServiceLocator::instance()->service<FileSystemManager>();

// Write JSON file
QJsonObject config;
config["theme"] = "dark";
config["language"] = "en_US";
fs->writeJsonFile("config.json", config);

// Read JSON file
QJsonObject loaded = fs->readJsonFile("config.json");
QString theme = loaded["theme"].toString();
```

### 4.2 Path Operations

```cpp
// Join paths
QString path = PathUtils::join("/home/user", "documents", "file.txt");
// Result: /home/user/documents/file.txt

// Get file name
QString name = PathUtils::fileName("/home/user/file.txt");
// Result: file.txt

// Get extension
QString ext = PathUtils::fileExtension("file.txt");
// Result: txt

// Safe file name
QString safe = PathUtils::safeFileName("file<>:name?.txt");
// Result: filename.txt
```

### 4.3 File Monitoring

```cpp
auto *fs = ServiceLocator::instance()->service<FileSystemManager>();

// Monitor file changes
fs->watchFile("config.json");
connect(fs, &FileSystemManager::fileChanged, [](const QString &path) {
    LOG_INFO("FileSystem", QString("File changed: %1").arg(path));
    // Reload configuration
});

// Monitor directory changes
fs->watchDir("downloads/");
connect(fs, &FileSystemManager::dirChanged, [](const QString &path) {
    LOG_INFO("FileSystem", QString("Directory changed: %1").arg(path));
});
```

### 4.4 Disk Space Check

```cpp
auto *fs = ServiceLocator::instance()->service<FileSystemManager>();

QString downloadDir = fs->appDownloadDir();
qint64 available = fs->availableSpace(downloadDir);
qint64 required = 1024 * 1024 * 100; // 100MB

if (available < required) {
    LOG_WARNING("FileSystem", "Insufficient disk space");
    // Show warning
}
```

## 5. Testing

```cpp
class FileSystemManagerTest : public QObject {
    Q_OBJECT
private slots:
    void testCreateDir();
    void testReadWriteFile();
    void testReadWriteJson();
    void testCopyMove();
    void testFileWatcher();
};

class PathUtilsTest : public QObject {
    Q_OBJECT
private slots:
    void testJoin();
    void testFileName();
    void testExtension();
    void testNormalize();
    void testSafeFileName();
};
```

## 6. Summary

The file system module provides complete file operation support:
- **FileSystemManager**: File read/write, directory management, file monitoring
- **PathUtils**: Path handling, path joining, path validation
