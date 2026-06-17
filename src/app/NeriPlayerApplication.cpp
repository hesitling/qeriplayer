/// @file NeriPlayerApplication.cpp
/// @brief Application entry point with service registration

#include "app/NeriPlayerApplication.h"

#include "api/netease/NeteaseClient.h"
#include "core/crypto/SecureStorage.h"
#include "core/database/DatabaseManager.h"
#include "core/filesystem/AppPaths.h"
#include "core/logger/Logger.h"
#include "core/network/NetworkManager.h"
#include "mainwindow.h"
#include "player/BackendFactory.h"
#include "player/PlaybackController.h"
#include "repo/PlayHistoryRepository.h"
#include "repo/PlayerStateRepository.h"
#include "repo/PlaylistRepository.h"
#include "repo/SettingsRepository.h"
#include "repo/SongRepository.h"

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

    // 5. Repositories
    if (m_services.hasService<DatabaseManager>()) {
        auto *db = m_services.service<DatabaseManager>();

        auto songRepo = std::make_unique<SongRepository>(db);
        m_services.registerService<SongRepository>(std::move(songRepo));

        auto playlistRepo = std::make_unique<PlaylistRepository>(db);
        m_services.registerService<PlaylistRepository>(std::move(playlistRepo));

        auto playHistoryRepo = std::make_unique<PlayHistoryRepository>(db);
        m_services.registerService<PlayHistoryRepository>(std::move(playHistoryRepo));

        auto playerStateRepo = std::make_unique<PlayerStateRepository>(db);
        m_services.registerService<PlayerStateRepository>(std::move(playerStateRepo));

        auto settingsRepo = std::make_unique<SettingsRepository>(db);
        m_services.registerService<SettingsRepository>(std::move(settingsRepo));

        log->info("Repositories registered");
    }

    // 6. NeteaseClient
    auto *netMgr = m_services.service<NetworkManager>();
    auto *secStorage = m_services.service<SecureStorage>();
    m_services.registerService<NeteaseClient>(std::make_unique<NeteaseClient>(netMgr->httpClient(), secStorage));
    log->info("NeteaseClient registered");

    // 7. Playback Engine
    if (m_services.hasService<SettingsRepository>() && m_services.hasService<PlayerStateRepository>()) {
        auto *settingsRepo = m_services.service<SettingsRepository>();
        auto *playerStateRepo = m_services.service<PlayerStateRepository>();

        // Read backend preference from settings, with safe fallback
        auto backendType = settingsRepo->get(QStringLiteral("player/backend")).value_or(QStringLiteral("auto"));
        std::unique_ptr<IPlayerBackend> backend;
        try {
            backend = BackendFactory::createBackend(backendType);
        } catch (const std::exception &ex) {
            log->warn("Backend '{}' unavailable ({}), falling back to Qt Multimedia", backendType.toStdString(),
                      ex.what());
            backend = BackendFactory::createBackend(QStringLiteral("qt"));
        }
        log->info("Audio backend: {}", backend->backendName().toStdString());

        auto *neteaseClient = m_services.service<NeteaseClient>();
        auto playbackCtrl
            = std::make_unique<PlaybackController>(std::move(backend), neteaseClient, playerStateRepo, settingsRepo);

        m_services.registerService<PlaybackController>(std::move(playbackCtrl));

        // Restore persisted playback state (non-blocking)
        auto *ctrl = m_services.service<PlaybackController>();
        ctrl->restoreState();
        log->info("PlaybackController registered");
    }

    log->info("Core services initialized");
}

void NeriPlayerApplication::initializeUi()
{
    m_mainWindow = std::make_unique<MainWindow>();
}

} // namespace NeriPlayerQt
