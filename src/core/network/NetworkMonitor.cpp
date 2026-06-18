#include "core/network/NetworkMonitor.h"

namespace QeriPlayerQt {

NetworkMonitor::NetworkMonitor(QObject *parent)
    : QObject(parent)
{
}

bool NetworkMonitor::isOnline() const
{
    return m_online;
}

void NetworkMonitor::setOnline(bool online)
{
    if (m_online == online) {
        return;
    }

    m_online = online;
    emit onlineChanged(m_online);
}

} // namespace QeriPlayerQt
