#ifndef NERIPLAYERQT_NETWORKMANAGER_H
#define NERIPLAYERQT_NETWORKMANAGER_H

#include "core/network/HttpClient.h"
#include "core/network/NetworkMonitor.h"
#include "core/network/WebSocketClient.h"

#include <QObject>

namespace NeriPlayerQt {

class NetworkManager : public QObject {
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);

    HttpClient *httpClient();
    WebSocketClient *webSocketClient();
    NetworkMonitor *networkMonitor();

    const HttpClient *httpClient() const;
    const WebSocketClient *webSocketClient() const;
    const NetworkMonitor *networkMonitor() const;

private:
    HttpClient m_httpClient;
    WebSocketClient m_webSocketClient;
    NetworkMonitor m_networkMonitor;
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_NETWORKMANAGER_H
