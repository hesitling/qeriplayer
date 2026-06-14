#ifndef NERIPLAYERQT_NETWORKMONITOR_H
#define NERIPLAYERQT_NETWORKMONITOR_H

#include <QObject>

namespace NeriPlayerQt {

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

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_NETWORKMONITOR_H
