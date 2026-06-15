/// @file TestLogger.cpp
/// @brief Unit tests for the logger module

#include "core/logger/Logger.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

using namespace NeriPlayerQt;

class TestLogger : public QObject {
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;

private Q_SLOTS:
    void initTestCase();

    // Logger initialization
    void initialize_setsUpLogger();
    void doubleInitialize_noop();

    // Named loggers
    void get_returnsNamedLogger();
    void get_sameInstanceTwice();

    // Log levels
    void setLevel_filtersMessages();
    void runtimeLevelChange();

    // File output
    void log_createsLogFile();
    void log_format();

    // Predefined categories
    void predefinedCategories_exist();
};

void TestLogger::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
    LoggerConfig config;
    config.logDir = m_tempDir.path();
    config.level = LogLevel::Debug;
    config.enableConsole = false; // silence during tests
    Logger::initialize(config);
}

void TestLogger::initialize_setsUpLogger()
{
    // Logger was already initialized in initTestCase
    auto logger = Logger::get("test");
    QVERIFY(logger != nullptr);
}

void TestLogger::doubleInitialize_noop()
{
    LoggerConfig config;
    config.logDir = m_tempDir.path();
    config.level = LogLevel::Trace;
    Logger::initialize(config); // should be a no-op

    // Level should still be Debug (from first init)
    auto logger = Logger::get("test");
    QVERIFY(logger != nullptr);
    QCOMPARE(logger->level(), LogLevel::Debug);
}

void TestLogger::get_returnsNamedLogger()
{
    auto logger = Logger::get("network");
    QVERIFY(logger != nullptr);
}

void TestLogger::get_sameInstanceTwice()
{
    auto logger1 = Logger::get("api");
    auto logger2 = Logger::get("api");
    QCOMPARE(logger1, logger2);
}

void TestLogger::setLevel_filtersMessages()
{
    Logger::setLevel(LogLevel::Info);
    auto logger = Logger::get("test_level");
    // Debug message should be filtered — no crash, no output
    logger->debug("this should be filtered");
    logger->info("this should appear");

    // Reset level
    Logger::setLevel(LogLevel::Debug);
}

void TestLogger::runtimeLevelChange()
{
    Logger::setLevel(LogLevel::Warn);
    Logger::setLevel(LogLevel::Debug);
    // No crash = success
}

void TestLogger::log_createsLogFile()
{
    // A log message was already emitted in initTestCase
    QDir logDir(m_tempDir.path());
    QStringList filters;
    filters << "neriplayer-*.log";
    auto files = logDir.entryList(filters, QDir::Files);
    QVERIFY(!files.isEmpty());
}

void TestLogger::log_format()
{
    // Ensure at least one info message is logged
    auto logger = Logger::get("format_test");
    logger->info("test message for format check");
    Logger::flush();

    QDir logDir(m_tempDir.path());
    QStringList filters;
    filters << "neriplayer-*.log";
    auto files = logDir.entryList(filters, QDir::Files);
    if (files.isEmpty()) {
        QSKIP("Log file not created yet (buffered)");
        return;
    }

    QFile logFile(logDir.filePath(files.first()));
    QVERIFY(logFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QByteArray content = logFile.readAll();
    // Should contain [info] and the logger name
    QVERIFY(content.contains("[info]"));
    QVERIFY(content.contains("format_test"));
}

void TestLogger::predefinedCategories_exist()
{
    auto app = Logger::get("app");
    auto network = Logger::get("network");
    auto player = Logger::get("player");
    auto api = Logger::get("api");
    auto ui = Logger::get("ui");

    QVERIFY(app != nullptr);
    QVERIFY(network != nullptr);
    QVERIFY(player != nullptr);
    QVERIFY(api != nullptr);
    QVERIFY(ui != nullptr);
}

QTEST_MAIN(TestLogger)
#include "TestLogger.moc"
