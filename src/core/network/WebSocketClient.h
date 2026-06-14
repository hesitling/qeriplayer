#ifndef NERIPLAYERQT_WEBSOCKETCLIENT_H
#define NERIPLAYERQT_WEBSOCKETCLIENT_H

#include <QObject>
#include <QUrl>
#include <QWebSocket>

namespace NeriPlayerQt {

class WebSocketClient : public QObject {
    Q_OBJECT

public:
    explicit WebSocketClient(QObject *parent = nullptr);

    void connectTo(const QUrl &url);
    void sendTextMessage(const QString &message);
    void close();
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void textMessageReceived(const QString &message);
    void errorOccurred(const QString &message);

private:
    QWebSocket m_socket;
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_WEBSOCKETCLIENT_H
