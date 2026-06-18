/// @file QeriPlayerApplication.cpp
/// @brief Application entry point with service registration

#include "app/QeriPlayerApplication.h"

#include "api/common/IMusicPlatformPlugin.h"
#include "api/netease/NeteaseClient.h"
#include "core/crypto/SecureStorage.h"
#include "core/database/DatabaseManager.h"
#include "core/filesystem/AppPaths.h"
#include "core/logger/Logger.h"
#include "core/network/NetworkManager.h"
#include "player/BackendFactory.h"
#include "player/PlaybackController.h"
#include "repo/PlayHistoryRepository.h"
#include "repo/PlayerStateRepository.h"
#include "repo/PlaylistRepository.h"
#include "repo/SettingsRepository.h"
#include "repo/SongRepository.h"
#include "viewmodel/MainViewModel.h"
#include "viewmodel/PlayerViewModel.h"
#include "viewmodel/PlaylistViewModel.h"
#include "viewmodel/SearchViewModel.h"
#include "viewmodel/SettingsViewModel.h"

#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQmlContext>

namespace QeriPlayerQt {

QeriPlayerApplication::QeriPlayerApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
    setApplicationName(QStringLiteral("QeriPlayer Qt"));
    setApplicationVersion(QStringLiteral("0.1.0"));
    setOrganizationName(QStringLiteral("QeriPlayer"));
}

QeriPlayerApplication::~QeriPlayerApplication()
{
    // m_qmlEngine (unique_ptr) is destroyed before m_services (reverse declaration order)
}

bool QeriPlayerApplication::initialize()
{
    initializeCoreServices();
    return initializeUi();
}

ServiceLocator *QeriPlayerApplication::services()
{
    return &m_services;
}

const ServiceLocator *QeriPlayerApplication::services() const
{
    return &m_services;
}

void QeriPlayerApplication::initializeCoreServices()
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
    log->info("QeriPlayer Qt starting up");

    // 2. Database
    auto db = std::make_unique<DatabaseManager>();
    QString dbPath = AppPaths::dataDir() + QStringLiteral("/qeriplayer.db");
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

bool QeriPlayerApplication::initializeUi()
{
    auto log = Logger::get("app");

    // Gather dependencies from ServiceLocator
    auto *ctrl = m_services.service<PlaybackController>();
    auto *historyRepo = m_services.service<PlayHistoryRepository>();
    auto *songRepo = m_services.service<SongRepository>();
    auto *playlistRepo = m_services.service<PlaylistRepository>();
    auto *settingsRepo = m_services.service<SettingsRepository>();
    auto *neteaseClient = m_services.service<NeteaseClient>();

    if (!ctrl || !historyRepo || !songRepo || !playlistRepo || !settingsRepo) {
        log->error("Missing required services for UI initialization");
        return false;
    }

    // Create ViewModels
    auto *playerVm = new PlayerViewModel(ctrl, historyRepo, this);

    QVector<IMusicPlatformPlugin *> plugins;
    if (neteaseClient) {
        plugins.append(neteaseClient);
    }
    auto *searchVm = new SearchViewModel(plugins, songRepo, this);

    auto *playlistVm = new PlaylistViewModel(playlistRepo, neteaseClient, this);
    auto *settingsVm = new SettingsViewModel(settingsRepo, neteaseClient, historyRepo, this);
    auto *mainVm
        = new MainViewModel(playerVm, searchVm, playlistVm, settingsVm, songRepo, playlistRepo, neteaseClient, this);

    // Set Material Dark as default style
    qputenv("QT_QUICK_CONTROLS_STYLE", "Material");

    // Create QML engine and register context properties
    m_qmlEngine = std::make_unique<QQmlApplicationEngine>();

    // Log QML warnings
    connect(m_qmlEngine.get(), &QQmlApplicationEngine::warnings, this, [log](const QList<QQmlError> &warnings) {
        for (const auto &warning : warnings) {
            log->warn("QML: {}", warning.toString().toStdString());
        }
    });

    auto *ctx = m_qmlEngine->rootContext(); // NOLINT: unique_ptr lifetime covers usage
    ctx->setContextProperty(QStringLiteral("mainVm"), mainVm);
    ctx->setContextProperty(QStringLiteral("playerVm"), playerVm);
    ctx->setContextProperty(QStringLiteral("searchVm"), searchVm);
    ctx->setContextProperty(QStringLiteral("playlistVm"), playlistVm);
    ctx->setContextProperty(QStringLiteral("settingsVm"), settingsVm);

    log->info("Loading QML UI");
    m_qmlEngine->load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (m_qmlEngine->rootObjects().isEmpty()) {
        log->error("Failed to load QML UI — no root objects");
        return false;
    }

    log->info("QML UI loaded successfully");
    return true;
}

} // namespace QeriPlayerQt
