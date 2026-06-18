# Application Module (app/)

## Overview

The application module is the entry point of QeriPlayer Qt. It initializes core services, registers them in the `ServiceLocator`, and manages the application lifecycle.

## Source Files

```
src/app/
├── QeriPlayerApplication.h / .cpp
└── ServiceLocator.h
```

## QeriPlayerApplication

Inherits from `QApplication`. Owns the `ServiceLocator` and `MainWindow`.

```cpp
class QeriPlayerApplication : public QApplication {
    Q_OBJECT
public:
    QeriPlayerApplication(int &argc, char **argv);
    ~QeriPlayerApplication() override;

    bool initialize();
    void showMainWindow();

    ServiceLocator *services();
    MainWindow *mainWindow() const;

private:
    void initializeCoreServices();
    void initializeUi();

    ServiceLocator m_services;
    std::unique_ptr<MainWindow> m_mainWindow;
};
```

### Initialization Flow

```
QeriPlayerApplication::initialize()
├── initializeCoreServices()
│   ├── Logger::initialize()
│   ├── DatabaseManager → open, register migrations, create tables
│   ├── SecureStorage
│   ├── HttpClient, NetworkManager
│   └── Register all in ServiceLocator
└── initializeUi()
    └── MainWindow::create()
```

## ServiceLocator

Type-erased service registry. Stores `unique_ptr<void>` with custom deleters, keyed by `std::type_index`.

```cpp
class ServiceLocator {
public:
    ServiceLocator() = default;

    template <typename T>
    void registerService(std::unique_ptr<T> service);

    template <typename T>
    T *service() const;  // Returns nullptr if not registered

    template <typename T>
    bool hasService() const;

    void clear();
};
```

### Usage

```cpp
// Register
auto locator = app.services();
locator->registerService<NetworkManager>(std::make_unique<NetworkManager>());
locator->registerService<DatabaseManager>(std::make_unique<DatabaseManager>());

// Retrieve
auto *network = locator->service<NetworkManager>();
if (network) {
    auto *http = network->httpClient();
}
```

### Design Decisions

- **Not a singleton** — `ServiceLocator` is a regular member of `QeriPlayerApplication`. Access via `app.services()`.
- **Type-based keys** — each type can have at most one registered service.
- **Returns nullptr for missing services** — callers must check.
- **No circular dependency detection** — services should be registered in dependency order.

## main.cpp

```cpp
int main(int argc, char *argv[]) {
    QeriPlayerApplication app(argc, argv);
    if (!app.initialize()) return 1;
    app.showMainWindow();
    return app.exec();
}
```
