# Logger Module (core/logger/)

## Overview

The logger module wraps spdlog to provide named loggers with daily file rotation and colored console output. Loggers are organized by category (e.g., "network", "api", "player") and share common sinks.

## Source Files

```text
src/core/logger/
├── Logger.h
└── Logger.cpp
```

## Logger

Static class that initializes spdlog and provides named logger instances.

```cpp
class Logger {
public:
    static void initialize(const LoggerConfig &config);
    static std::shared_ptr<NamedLogger> get(const QString &name);
    static void setLevel(LogLevel level);
    static void flush();
    static bool isInitialized();
};
```

### LoggerConfig

```cpp
struct LoggerConfig {
    QString logDir;                    // Directory for log files
    LogLevel level = LogLevel::Info;   // Minimum log level
    bool enableConsole = true;         // Enable console output
    int maxDays = 7;                   // Keep 7 days of log files
};
```

### LogLevel

```cpp
enum class LogLevel { Trace, Debug, Info, Warn, Error, Fatal };
```

## NamedLogger

Wrapper around an spdlog logger instance. One per category name, all sharing the same file and console sinks.

```cpp
class NamedLogger {
public:
    void setLevel(LogLevel level);
    LogLevel level() const;

    void trace(const char *msg);
    void debug(const char *msg);
    void info(const char *msg);
    void warn(const char *msg);
    void error(const char *msg);
    void fatal(const char *msg);

    // fmt-style formatted logging
    template <typename... Args>
    void info(fmt::format_string<Args...> fmt, Args &&...args);
    // ... same for trace, debug, warn, error, fatal
};
```

## Design Decisions

- **spdlog-based** — uses `daily_file_sink_mt` for file rotation (not `rotating_file_sink_mt`).
- **Daily rotation** — log files are named `qeriplayer-YYYY-MM-DD.log`. Files older than `maxDays` are deleted automatically.
- **Shared sinks** — all named loggers share the same file and console sinks (initialized once).
- **Double initialization is a no-op** — calling `initialize()` twice preserves the original config.
- **Runtime level change** — `setLevel()` takes effect immediately.
- **Colored console** — `stdout_color_sink_mt` provides ANSI-colored output when the terminal supports it; plain text when piped.

## Log Format

```text
[2024-01-15 10:30:45.123] [info] [network] Connection established
[2024-01-15 10:30:45.456] [error] [api] Request failed: timeout
```

## Usage

```cpp
// Initialize once at startup
Logger::initialize({ .logDir = AppPaths::dataDir() + "/logs", .level = LogLevel::Debug });

// Get named loggers
auto netLog = Logger::get("network");
netLog->info("Connected to {}", serverUrl);

auto apiLog = Logger::get("api");
apiLog->error("Request failed: {}", errorMsg);

// Change level at runtime
Logger::setLevel(LogLevel::Warn);
```

## Testing

See `tests/core/TestLogger.cpp`.
