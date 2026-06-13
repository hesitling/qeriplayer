# Application Module (app/)

## 1. Overview

The application module is the entry module of NeriPlayer Qt, responsible for application initialization, service registration, and lifecycle management.

## 2. Responsibilities

- Application entry point and initialization
- Service locator management
- Global exception handling
- Application lifecycle management

## 3. Directory Structure

```
src/app/
├── NeriPlayerApplication.h    # Main application class
├── NeriPlayerApplication.cpp
├── ServiceLocator.h           # Service locator
├── ServiceLocator.cpp
├── GlobalExceptionHandler.h   # Global exception handler
└── GlobalExceptionHandler.cpp
```

## 4. Main Class Design

### 4.1 NeriPlayerApplication

The main application class, inheriting from `QApplication`, responsible for application initialization and lifecycle management.

```cpp
class NeriPlayerApplication : public QApplication {
    Q_OBJECT
public:
    NeriPlayerApplication(int &argc, char **argv);
    ~NeriPlayerApplication();
    
    // Initialize application
    bool initialize();
    
    // Get service
    template<typename T>
    T* service() const;
    
    // Get config
    ConfigManager* config() const;
    
    // Get logger
    LogManager* logger() const;
    
    // Get main window
    MainWindow* mainWindow() const;
    
private:
    void initializeCoreServices();
    void initializeApiClients();
    void initializePlayer();
    void initializeUi();
    void setupConnections();
    
    std::unique_ptr<ServiceLocator> m_services;
    std::unique_ptr<ConfigManager> m_config;
    std::unique_ptr<LogManager> m_logger;
    MainWindow* m_mainWindow = nullptr;
};
```

**Initialization Flow**:

```
1. Create application instance
2. Initialize core services
   ├── Config manager
   ├── Log manager
   ├── File system manager
   └── Network manager
3. Initialize API clients
   ├── NetEase Cloud Music client
   ├── Bilibili client
   └── YouTube Music client
4. Initialize player
   ├── Player manager
   └── Audio engine
5. Initialize UI
   ├── Main window
   └── System tray
6. Connect signals and slots
7. Load user data
```

### 4.2 ServiceLocator

Service locator for managing all service instances in the application.

```cpp
class ServiceLocator {
public:
    // Get singleton
    static ServiceLocator* instance();
    
    // Register service
    template<typename T>
    void registerService(std::unique_ptr<T> service);
    
    // Get service
    template<typename T>
    T* service() const;
    
    // Check if service exists
    template<typename T>
    bool hasService() const;
    
    // Unregister service
    template<typename T>
    void unregisterService();
    
    // Clear all services
    void clear();
    
private:
    ServiceLocator() = default;
    ~ServiceLocator() = default;
    
    std::unordered_map<std::type_index, std::any> m_services;
};
```

**Usage Example**:

```cpp
// Register services
auto locator = ServiceLocator::instance();
locator->registerService<NetworkManager>(std::make_unique<NetworkManager>());
locator->registerService<DatabaseManager>(std::make_unique<DatabaseManager>("app.db"));

// Get services
auto network = locator->service<NetworkManager>();
auto database = locator->service<DatabaseManager>();

// Check if service exists
if (locator->hasService<NetworkManager>()) {
    // Service exists
}
```

### 4.3 GlobalExceptionHandler

Global exception handler class that captures unhandled exceptions and logs them.

```cpp
class GlobalExceptionHandler : public QObject {
    Q_OBJECT
public:
    static GlobalExceptionHandler* instance();
    
    // Initialize exception handling
    void initialize();
    
    // Set exception handler callback
    void setExceptionHandler(std::function<void(const AppError&)> handler);
    
    // Handle exceptions
    void handleException(const std::exception &e);
    void handleException(const AppError &error);
    
    // Get crash log
    QString crashLog() const;
    
signals:
    void exceptionOccurred(const AppError &error);
    
private:
    GlobalExceptionHandler(QObject *parent = nullptr);
    
    void installSignalHandlers();
    void installTerminateHandler();
    
    static void signalHandler(int signal);
    static void terminateHandler();
    
    std::function<void(const AppError&)> m_handler;
    static GlobalExceptionHandler* s_instance;
};
```

## 5. Initialization Order

```
main()
  └── NeriPlayerApplication::initialize()
        ├── initializeCoreServices()
        │     ├── LogManager::initialize()
        │     ├── ConfigManager::initialize()
        │     ├── FileSystemManager::initialize()
        │     ├── DatabaseManager::initialize()
        │     └── NetworkManager::initialize()
        │
        ├── initializeApiClients()
        │     ├── NeteaseClient::initialize()
        │     ├── BilibiliClient::initialize()
        │     └── YouTubeMusicClient::initialize()
        │
        ├── initializePlayer()
        │     ├── AudioEngine::initialize()
        │     └── PlayerManager::initialize()
        │
        ├── initializeUi()
        │     ├── ThemeManager::initialize()
        │     └── MainWindow::create()
        │
        ├── setupConnections()
        │
        └── loadUserData()
              ├── loadPlaylists()
              ├── loadFavorites()
              └── loadHistory()
```

## 6. Error Handling

### 6.1 Error Classification

```cpp
enum class ErrorCode {
    // Initialization errors
    InitError,
    ConfigError,
    DatabaseError,
    
    // Network errors
    NetworkError,
    TimeoutError,
    ServerError,
    
    // Authentication errors
    AuthenticationError,
    AuthorizationError,
    
    // Playback errors
    PlaybackError,
    DecodingError,
    
    // Data errors
    DataError,
    ParseError,
    
    // File errors
    FileError,
    PermissionError
};

// Error handling class
class AppError {
public:
    AppError(ErrorCode code, const QString &message, 
             const QString &details = {});
    
    ErrorCode code() const;
    QString message() const;
    QString details() const;
    
    // User-friendly error message
    QString userMessage() const;
    
    // Whether the error is retryable
    bool isRetryable() const;
    
private:
    ErrorCode m_code;
    QString m_message;
    QString m_details;
};
```

### 6.2 Exception Handling Strategy

```cpp
// In main.cpp
int main(int argc, char *argv[])
{
    try {
        NeriPlayerApplication app(argc, argv);
        
        if (!app.initialize()) {
            LOG_ERROR("App", "Failed to initialize application");
            return 1;
        }
        
        return app.exec();
    } catch (const std::exception &e) {
        LOG_FATAL("App", QString("Unhandled exception: %1").arg(e.what()));
        return 1;
    }
}
```

## 7. Configuration Management

### 7.1 Application Configuration

```cpp
// Application configuration structure
struct AppConfig {
    // Application info
    QString appName = "NeriPlayer Qt";
    QString appVersion = "0.1.0";
    QString appDataDir;
    QString appConfigDir;
    QString appCacheDir;
    QString appLogDir;
    
    // Debug options
    bool debugMode = false;
    bool verboseLogging = false;
    
    // Performance options
    int maxThreads = 4;
    int cacheSize = 1024 * 1024 * 1024; // 1GB
};
```

## 8. Thread Management

### 8.1 Thread Pool Configuration

```cpp
// Thread manager
class ThreadManager : public QObject {
    Q_OBJECT
public:
    static ThreadManager* instance();
    
    // Get thread pool
    QThreadPool* threadPool() const;
    
    // Set thread count
    void setMaxThreadCount(int count);
    
    // Execute in worker thread
    template<typename T>
    QFuture<T> run(std::function<T()> func);
    
    // Execute in main thread
    void runOnMainThread(std::function<void()> func);
    
private:
    ThreadManager(QObject *parent = nullptr);
    
    QThreadPool *m_threadPool;
};
```

## 9. Lifecycle Management

### 9.1 Application State

```cpp
enum class AppState {
    Initializing,
    Running,
    Suspending,
    Suspended,
    Resuming,
    ShuttingDown
};
```

### 9.2 Lifecycle Events

```cpp
class AppLifecycle : public QObject {
    Q_OBJECT
public:
    static AppLifecycle* instance();
    
    AppState currentState() const;
    
signals:
    void stateChanged(AppState state);
    void aboutToSuspend();
    void suspended();
    void aboutToResume();
    void resumed();
    void aboutToShutdown();
    
private:
    AppLifecycle(QObject *parent = nullptr);
    
    AppState m_state = AppState::Initializing;
};
```

## 10. Testing

### 10.1 Unit Tests

```cpp
class NeriPlayerApplicationTest : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void testInitialize();
    void testServiceLocator();
    void testConfigManager();
    void testErrorHandling();
};
```

## 11. Summary

The application module is the entry point and foundation of the entire project, responsible for:
- Application initialization and lifecycle management
- Service registration and retrieval
- Global exception handling
- Configuration management

Through the service locator pattern, loose coupling between modules is achieved, facilitating testing and maintenance.
