/// @file Logger.h
/// @brief spdlog-based logging with file rotation and named loggers

#ifndef NERIPLAYERQT_LOGGER_H
#define NERIPLAYERQT_LOGGER_H

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <QString>

#include <memory>
#include <mutex>
#include <unordered_map>

namespace NeriPlayerQt {

/**
 * @brief Log severity levels
 */
enum class LogLevel { Trace, Debug, Info, Warn, Error, Fatal };

/**
 * @brief Logger configuration
 */
struct LoggerConfig {
    QString logDir; ///< Directory for log files
    LogLevel level = LogLevel::Info;
    bool enableConsole = true;
    int maxDays = 7; ///< Keep 7 days of log files
};

/**
 * @brief Named logger wrapper around spdlog
 */
class NamedLogger {
public:
    explicit NamedLogger(std::shared_ptr<spdlog::logger> logger);

    void setLevel(LogLevel level);
    LogLevel level() const;

    void trace(const char *msg);
    void debug(const char *msg);
    void info(const char *msg);
    void warn(const char *msg);
    void error(const char *msg);
    void fatal(const char *msg);

    template <typename... Args> void trace(fmt::format_string<Args...> fmt, Args &&...args)
    {
        m_logger->trace(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void debug(fmt::format_string<Args...> fmt, Args &&...args)
    {
        m_logger->debug(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void info(fmt::format_string<Args...> fmt, Args &&...args)
    {
        m_logger->info(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void warn(fmt::format_string<Args...> fmt, Args &&...args)
    {
        m_logger->warn(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void error(fmt::format_string<Args...> fmt, Args &&...args)
    {
        m_logger->error(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void fatal(fmt::format_string<Args...> fmt, Args &&...args)
    {
        m_logger->critical(fmt, std::forward<Args>(args)...);
    }

private:
    std::shared_ptr<spdlog::logger> m_logger;
};

/**
 * @brief Central logger with file rotation, console output, and named loggers
 */
class Logger {
public:
    /**
     * @brief Initialize the logger system (call once at startup)
     * @param config Logger configuration
     */
    static void initialize(const LoggerConfig &config);

    /**
     * @brief Get or create a named logger
     * @param name Logger category name
     * @return Shared pointer to the named logger
     */
    static std::shared_ptr<NamedLogger> get(const QString &name);

    /**
     * @brief Change the global log level at runtime
     */
    static void setLevel(LogLevel level);

    /**
     * @brief Flush all log sinks
     */
    static void flush();

    /**
     * @brief Convert LogLevel to spdlog level
     */
    static spdlog::level::level_enum toSpdlogLevel(LogLevel level);

    /**
     * @brief Check if logger has been initialized
     */
    static bool isInitialized();

private:
    Logger() = default;

    static std::mutex s_mutex;
    static bool s_initialized;
    static LoggerConfig s_config;
    static std::shared_ptr<spdlog::sinks::daily_file_sink_mt> s_fileSink;
    static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> s_consoleSink;
    static std::unordered_map<std::string, std::shared_ptr<NamedLogger>> s_loggers;
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_LOGGER_H
