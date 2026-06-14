/// @file NeriPlayerApplication.cpp
/// @brief Application entry point with service registration
/// @date 2024-01-15

#include "app/NeriPlayerApplication.h"

#include "core/crypto/SecureStorage.h"
#include "core/database/DatabaseManager.h"
#include "core/filesystem/AppPaths.h"
#include "core/logger/Logger.h"
#include "core/network/NetworkManager.h"
#include "mainwindow.h"

#include <QDebug>

namespace NeriPlayerQt {

NeriPlayerApplication::NeriPlayerApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    setApplicationName(QStringLiteral("NeriPlayer Qt"));
    setApplicationVersion(QStringLiteral("0.1.0"));
    setOrganizationName(QStringLiteral("NeriPlayer"));
}

NeriPlayerApplication::~NeriPlayerApplication()
{
    // Services are cleaned up by ServiceLocator's unique_ptr.
    // Destruction order is unspecified (unordered_map).
    m_services.clear();
}

bool NeriPlayerApplication::initialize()
{
    initializeCoreServices();
    initializeUi();
    return true;
}

void NeriPlayerApplication::showMainWindow()
{
    if (m_mainWindow) {
        m_mainWindow->show();
    }
}

ServiceLocator *NeriPlayerApplication::services()
{
    return &m_services;
}

const ServiceLocator *NeriPlayerApplication::services() const
{
    return &m_services;
}

MainWindow *NeriPlayerApplication::mainWindow() const
{
    return m_mainWindow.get();
}

void NeriPlayerApplication::initializeCoreServices()
{
    // 1. Logger (first — other services may log)
    LoggerConfig logConfig;
    logConfig.logDir = AppPaths::cacheDir() + QStringLiteral("/logs");
    logConfig.level = LogLevel::Info;
    logConfig.enableConsole = true;

    try {
        Logger::initialize(logConfig);
    } catch (const std::exception &ex) {
        // Logger::get() returns a console-only fallback before initialize()
        Logger::get("app")->warn("Logger file sink init failed: {} — falling back to console", ex.what());
        logConfig.logDir.clear();
        Logger::initialize(logConfig);
    }

    auto log = Logger::get("app");
    log->info("NeriPlayer Qt starting up");

    // 2. Database
    auto db = std::make_unique<DatabaseManager>();
    QString dbPath = AppPaths::dataDir() + QStringLiteral("/neriplayer.db");
    if (!db->open(dbPath)) {
        log->error("Failed to open database: {}", dbPath.toStdString());
    } else {
        log->info("Database opened: {}", dbPath.toStdString());
        m_services.registerService<DatabaseManager>(std::move(db));
    }

    // 3. SecureStorage
    auto storage = std::make_unique<SecureStorage>(AppPaths::dataDir() + QStringLiteral("/secrets.dat"));
    m_services.registerService<SecureStorage>(std::move(storage));

    // 4. Network
    m_services.registerService<NetworkManager>(std::make_unique<NetworkManager>());

    log->info("Core services initialized");
}

void NeriPlayerApplication::initializeUi()
{
    m_mainWindow = std::make_unique<MainWindow>();
}

} // namespace NeriPlayerQt
