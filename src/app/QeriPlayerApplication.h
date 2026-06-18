/// @file QeriPlayerApplication.h
/// @brief Application entry point with service registration

#ifndef QERIPLAYERQT_QERIPLAYERAPPLICATION_H
#define QERIPLAYERQT_QERIPLAYERAPPLICATION_H

#include "app/ServiceLocator.h"

#include <QApplication>

#include <memory>

namespace QeriPlayerQt {

class MainWindow;
class NetworkManager;
class DatabaseManager;
class Logger;
class AppPaths;
class SecureStorage;

class QeriPlayerApplication : public QApplication {
    Q_OBJECT

public:
    QeriPlayerApplication(int &argc, char **argv);
    ~QeriPlayerApplication() override;

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

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_QERIPLAYERAPPLICATION_H
