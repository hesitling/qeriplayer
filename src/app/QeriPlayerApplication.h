/// @file QeriPlayerApplication.h
/// @brief Application entry point with service registration

#ifndef QERIPLAYERQT_QERIPLAYERAPPLICATION_H
#define QERIPLAYERQT_QERIPLAYERAPPLICATION_H

#include "app/ServiceLocator.h"

#include <QApplication>

#include <memory>

class QQmlApplicationEngine;

namespace QeriPlayerQt {

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

    ServiceLocator *services();
    const ServiceLocator *services() const;

private:
    void initializeCoreServices();
    bool initializeUi();

    ServiceLocator m_services;
    std::unique_ptr<QQmlApplicationEngine> m_qmlEngine;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_QERIPLAYERAPPLICATION_H
