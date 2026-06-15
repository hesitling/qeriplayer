/// @file NeriPlayerApplication.h
/// @brief Application entry point with service registration

#ifndef NERIPLAYERQT_NERIPLAYERAPPLICATION_H
#define NERIPLAYERQT_NERIPLAYERAPPLICATION_H

#include "app/ServiceLocator.h"

#include <QApplication>

#include <memory>

namespace NeriPlayerQt {

class MainWindow;
class NetworkManager;
class DatabaseManager;
class Logger;
class AppPaths;
class SecureStorage;

class NeriPlayerApplication : public QApplication {
    Q_OBJECT

public:
    NeriPlayerApplication(int &argc, char **argv);
    ~NeriPlayerApplication() override;

    bool initialize();
    void showMainWindow();

    ServiceLocator *services();
    const ServiceLocator *services() const;
    MainWindow *mainWindow() const;

private:
    void initializeCoreServices();
    void initializeUi();

    ServiceLocator m_services;
    std::unique_ptr<MainWindow> m_mainWindow;
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_NERIPLAYERAPPLICATION_H
