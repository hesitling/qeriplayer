/// @file Logger.cpp
/// @brief spdlog-based logging implementation
/// @date 2024-01-15

#include "core/logger/Logger.h"

#include <spdlog/pattern_formatter.h>

#include <QDate>
#include <QDir>
#include <QFileInfo>

namespace NeriPlayerQt {

// NamedLogger implementation

NamedLogger::NamedLogger(std::shared_ptr<spdlog::logger> logger)
    : m_logger(std::move(logger))
{
}

void NamedLogger::setLevel(LogLevel level)
{
    m_logger->set_level(Logger::toSpdlogLevel(level));
}

void NamedLogger::trace(const char *msg) { m_logger->trace(msg); }
void NamedLogger::debug(const char *msg) { m_logger->debug(msg); }
void NamedLogger::info(const char *msg)  { m_logger->info(msg); }
void NamedLogger::warn(const char *msg)  { m_logger->warn(msg); }
void NamedLogger::error(const char *msg) { m_logger->error(msg); }
void NamedLogger::fatal(const char *msg) { m_logger->critical(msg); }

// Logger static members

std::mutex Logger::s_mutex;
bool Logger::s_initialized = false;
LoggerConfig Logger::s_config;
std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> Logger::s_fileSink;
std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> Logger::s_consoleSink;
std::unordered_map<std::string, std::shared_ptr<NamedLogger>> Logger::s_loggers;

void Logger::initialize(const LoggerConfig &config)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    if (s_initialized) {
        return; // no-op on double init
    }

    s_config = config;

    // Create log directory if needed
    QDir logDir(config.logDir);
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }

    // File sink with daily-style rotation (by size, with max files)
    QString logPath = logDir.filePath("neriplayer-" + QDate::currentDate().toString("yyyy-MM-dd") + ".log");
    s_fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        logPath.toStdString(),
        config.maxFileSize,
        config.maxFiles);

    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(s_fileSink);

    // Console sink (optional)
    if (config.enableConsole) {
        s_consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sinks.push_back(s_consoleSink);
    }

    // Create the default logger
    auto defaultLogger = std::make_shared<spdlog::logger>(
        "default", sinks.begin(), sinks.end());
    defaultLogger->set_level(toSpdlogLevel(config.level));
    defaultLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v");
    spdlog::set_default_logger(defaultLogger);

    // Pre-create standard loggers
    for (const auto &name : {"app", "network", "player", "api", "ui"}) {
        auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
        logger->set_level(toSpdlogLevel(config.level));
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v");
        s_loggers[name] = std::make_shared<NamedLogger>(logger);
    }

    s_initialized = true;

    // Flush to ensure file is created
    spdlog::default_logger()->flush();
}

std::shared_ptr<NamedLogger> Logger::get(const QString &name)
{
    std::lock_guard<std::mutex> lock(s_mutex);

    std::string key = name.toStdString();
    auto it = s_loggers.find(key);
    if (it != s_loggers.end()) {
        return it->second;
    }

    // Create a new logger with the same sinks
    std::vector<spdlog::sink_ptr> sinks;
    if (s_fileSink) sinks.push_back(s_fileSink);
    if (s_consoleSink) sinks.push_back(s_consoleSink);

    auto logger = std::make_shared<spdlog::logger>(key, sinks.begin(), sinks.end());
    logger->set_level(toSpdlogLevel(s_config.level));
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v");

    auto namedLogger = std::make_shared<NamedLogger>(logger);
    s_loggers[key] = namedLogger;
    return namedLogger;
}

void Logger::setLevel(LogLevel level)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    s_config.level = level;
    auto spdLevel = toSpdlogLevel(level);

    // Update all named loggers
    for (auto &[name, namedLogger] : s_loggers) {
        namedLogger->setLevel(level);
    }

    // Update spdlog's global level
    spdlog::set_level(spdLevel);

    // Update all sinks
    if (s_fileSink) s_fileSink->set_level(spdLevel);
    if (s_consoleSink) s_consoleSink->set_level(spdLevel);
}

spdlog::level::level_enum Logger::toSpdlogLevel(LogLevel level)
{
    switch (level) {
    case LogLevel::Trace: return spdlog::level::trace;
    case LogLevel::Debug: return spdlog::level::debug;
    case LogLevel::Info:  return spdlog::level::info;
    case LogLevel::Warn:  return spdlog::level::warn;
    case LogLevel::Error: return spdlog::level::err;
    case LogLevel::Fatal: return spdlog::level::critical;
    default: return spdlog::level::info;
    }
}

bool Logger::isInitialized()
{
    return s_initialized;
}

void Logger::flush()
{
    spdlog::default_logger()->flush();
}

} // namespace NeriPlayerQt
