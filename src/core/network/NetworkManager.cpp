#include "core/network/NetworkManager.h"

namespace QeriPlayerQt {

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_httpClient(this)
    , m_webSocketClient(this)
    , m_networkMonitor(this)
{
}

HttpClient *NetworkManager::httpClient()
{
    return &m_httpClient;
}

WebSocketClient *NetworkManager::webSocketClient()
{
    return &m_webSocketClient;
}

NetworkMonitor *NetworkManager::networkMonitor()
{
    return &m_networkMonitor;
}

const HttpClient *NetworkManager::httpClient() const
{
    return &m_httpClient;
}

const WebSocketClient *NetworkManager::webSocketClient() const
{
    return &m_webSocketClient;
}

const NetworkMonitor *NetworkManager::networkMonitor() const
{
    return &m_networkMonitor;
}

} // namespace QeriPlayerQt
