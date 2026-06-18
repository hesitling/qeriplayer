#ifndef QERIPLAYERQT_NETWORKMONITOR_H
#define QERIPLAYERQT_NETWORKMONITOR_H

#include <QObject>

namespace QeriPlayerQt {

class NetworkMonitor : public QObject {
    Q_OBJECT

public:
    explicit NetworkMonitor(QObject *parent = nullptr);

    bool isOnline() const;
    void setOnline(bool online);

signals:
    void onlineChanged(bool online);

private:
    bool m_online = true;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_NETWORKMONITOR_H
