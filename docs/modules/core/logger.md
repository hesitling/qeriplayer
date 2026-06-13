# Logger Module (logger/)

## 1. Overview

The logger module provides log recording, log file management, log filtering, and log rotation capabilities.

## 2. Directory Structure

```
src/core/logger/
├── LogManager.h/.cpp    # Log manager
├── LogEntry.h           # Log entry
├── LogFilter.h          # Log filter
└── LogFormatter.h/.cpp  # Log formatter
```

## 3. Main Class Design

### 3.1 LogManager

Log manager.

```cpp
class LogManager : public QObject {
    Q_OBJECT
public:
    static LogManager* instance();
    
    // ==================== Initialization ====================
    
    void initialize(const QString &logDir, 
                    LogLevel minLevel = LogLevel::Info);
    
    // ==================== Log Recording ====================
    
    void log(LogLevel level, const QString &category,
             const QString &message, const QString &file = {},
             int line = 0, const QString &function = {});
    
    // Convenience methods
    void debug(const QString &category, const QString &message);
    void info(const QString &category, const QString &message);
    void warning(const QString &category, const QString &message);
    void error(const QString &category, const QString &message);
    void fatal(const QString &category, const QString &message);
    
    // ==================== Log Query ====================
    
    QList<LogEntry> getLogs(const LogFilter &filter = {}) const;
    QList<LogEntry> getLogs(LogLevel minLevel, 
                            const QString &category = {},
                            int limit = 100) const;
    
    // ==================== Log Files ====================
    
    QString currentLogFile() const;
    QStringList logFiles() const;
    qint64 totalLogSize() const;
    
    // ==================== Log Cleanup ====================
    
    void cleanOldLogs(int maxDays = 30);
    void cleanLogsBySize(qint64 maxSize);
    
    // ==================== Configuration ====================
    
    void setMinLevel(LogLevel level);
    LogLevel minLevel() const;
    
    void setConsoleOutputEnabled(bool enabled);
    bool isConsoleOutputEnabled() const;
    
    void setMaxFileSize(qint64 maxSize);
    void setMaxFiles(int maxFiles);
    
signals:
    void logAdded(const LogEntry &entry);
    
private:
    LogManager(QObject *parent = nullptr);
    void writeToFile(const LogEntry &entry);
    void writeToConsole(const LogEntry &entry);
    void rotateLogFile();
    QString formatLogEntry(const LogEntry &entry) const;
    
    QFile *m_logFile;
    QTextStream *m_logStream;
    LogLevel m_minLevel;
    bool m_consoleOutputEnabled = true;
    qint64 m_maxFileSize = 10 * 1024 * 1024; // 10MB
    int m_maxFiles = 10;
    QString m_logDir;
};
```

### 3.2 LogLevel

Log level enumeration.

```cpp
enum class LogLevel {
    Debug,    // Debug information
    Info,     // General information
    Warning,  // Warning
    Error,    // Error
    Fatal     // Fatal error
};

// Log level string conversion
QString logLevelToString(LogLevel level);
LogLevel stringToLogLevel(const QString &str);
```

### 3.3 LogEntry

Log entry.

```cpp
struct LogEntry {
    QDateTime timestamp;    // Timestamp
    LogLevel level;         // Log level
    QString category;       // Category
    QString message;        // Message
    QString file;           // Source file
    int line;               // Line number
    QString function;       // Function name
    QString threadId;       // Thread ID
    
    // Convert to string
    QString toString() const;
    
    // Convert to JSON
    QJsonObject toJson() const;
};
```

### 3.4 LogFilter

Log filter.

```cpp
struct LogFilter {
    LogLevel minLevel = LogLevel::Debug;
    LogLevel maxLevel = LogLevel::Fatal;
    QString category;
    QDateTime startTime;
    QDateTime endTime;
    QString searchText;
    int limit = -1;
    
    // Check if log entry matches
    bool matches(const LogEntry &entry) const;
};
```

### 3.5 LogFormatter

Log formatter.

```cpp
class LogFormatter {
public:
    // Format log entry
    static QString format(const LogEntry &entry, 
                          const QString &pattern = {});
    
    // Default format
    static QString defaultFormat(const LogEntry &entry);
    
    // Simple format
    static QString simpleFormat(const LogEntry &entry);
    
    // JSON format
    static QString jsonFormat(const LogEntry &entry);
    
    // Format pattern tokens
    // %d - Date/time
    // %l - Log level
    // %c - Category
    // %m - Message
    // %f - File
    // %n - Line number
    // %F - Function
    // %t - Thread ID
};
```

## 4. Usage Examples

### 4.1 Initialization

```cpp
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Initialize logging
    LogManager::instance()->initialize(
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs",
        LogLevel::Debug
    );
    
    LOG_INFO("App", "Application started");
    
    return app.exec();
}
```

### 4.2 Recording Logs

```cpp
// Using convenience macros
LOG_DEBUG("Player", "Loading song: " + song.title);
LOG_INFO("Network", "Request completed: " + url);
LOG_WARNING("Cache", "Cache size exceeds limit");
LOG_ERROR("Database", "Failed to execute query");
LOG_FATAL("App", "Unrecoverable error");

// Using full interface
LogManager::instance()->log(
    LogLevel::Info,
    "Player",
    "Playback started",
    __FILE__,
    __LINE__,
    __FUNCTION__
);
```

### 4.3 Querying Logs

```cpp
auto *logger = LogManager::instance();

// Get all error logs
LogFilter filter;
filter.minLevel = LogLevel::Error;
auto errors = logger->getLogs(filter);

// Get logs for specific category
auto playerLogs = logger->getLogs(LogLevel::Debug, "Player", 50);

// Search logs
LogFilter searchFilter;
searchFilter.searchText = "timeout";
auto timeoutLogs = logger->getLogs(searchFilter);
```

### 4.4 Log File Management

```cpp
auto *logger = LogManager::instance();

// Get current log file
QString currentFile = logger->currentLogFile();

// Get all log files
QStringList files = logger->logFiles();

// Get total log size
qint64 totalSize = logger->totalLogSize();

// Clean old logs
logger->cleanOldLogs(30);  // Keep 30 days

// Clean by size
logger->cleanLogsBySize(100 * 1024 * 1024);  // Keep 100MB
```

### 4.5 Configuration

```cpp
auto *logger = LogManager::instance();

// Set minimum log level
logger->setMinLevel(LogLevel::Warning);

// Enable/disable console output
logger->setConsoleOutputEnabled(false);

// Set file size limit
logger->setMaxFileSize(5 * 1024 * 1024);  // 5MB

// Set max file count
logger->setMaxFiles(20);
```

## 5. Log Formats

### 5.1 Default Format

```
[2024-01-15 14:30:25.123] [INFO] [Player] Playback started
[2024-01-15 14:30:25.456] [ERROR] [Network] Request failed: timeout
```

### 5.2 JSON Format

```json
{
    "timestamp": "2024-01-15T14:30:25.123",
    "level": "INFO",
    "category": "Player",
    "message": "Playback started",
    "file": "PlayerManager.cpp",
    "line": 123,
    "function": "play",
    "threadId": "0x1234"
}
```

## 6. Testing

```cpp
class LogManagerTest : public QObject {
    Q_OBJECT
private slots:
    void testInitialize();
    void testLogLevels();
    void testLogFilter();
    void testLogRotation();
    void testLogCleanup();
};
```

## 7. Summary

The logger module provides complete logging support:
- **LogManager**: Log recording, file management, log rotation
- **LogEntry**: Log data structure
- **LogFilter**: Log filtering and querying
- **LogFormatter**: Log formatting
