#include "core/network/WebSocketClient.h"

#include <QAbstractSocket>

namespace QeriPlayerQt {

WebSocketClient::WebSocketClient(QObject *parent)
    : QObject(parent)
{
    connect(&m_socket, &QWebSocket::connected, this, &WebSocketClient::connected);
    connect(&m_socket, &QWebSocket::disconnected, this, &WebSocketClient::disconnected);
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &WebSocketClient::textMessageReceived);
    connect(&m_socket, &QWebSocket::errorOccurred, this,
            [this](QAbstractSocket::SocketError) { emit errorOccurred(m_socket.errorString()); });
}

void WebSocketClient::connectTo(const QUrl &url)
{
    m_socket.open(url);
}

void WebSocketClient::sendTextMessage(const QString &message)
{
    if (isConnected()) {
        m_socket.sendTextMessage(message);
    }
}

void WebSocketClient::close()
{
    m_socket.close();
}

bool WebSocketClient::isConnected() const
{
    return m_socket.state() == QAbstractSocket::ConnectedState;
}

} // namespace QeriPlayerQt
