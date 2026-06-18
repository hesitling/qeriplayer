/// @file BackendFactory.cpp
/// @brief Factory for creating audio playback backends

#include "player/BackendFactory.h"
#include "player/backends/QtMultimediaBackend.h"

#include <QDebug>
#include <QLibrary>

#include <stdexcept>

namespace QeriPlayerQt {

std::unique_ptr<IPlayerBackend> BackendFactory::createBackend(const QString &type)
{
    if (type == QStringLiteral("qt")) {
        return std::make_unique<QtMultimediaBackend>();
    }

    if (type == QStringLiteral("mpv")) {
        if (isMpvAvailable()) {
            // mpv backend not yet implemented — fall through to auto
            qWarning() << "mpv backend not yet implemented, falling back to Qt Multimedia";
            return std::make_unique<QtMultimediaBackend>();
        }
        throw std::runtime_error("mpv is not available on this system");
    }

    if (type == QStringLiteral("auto")) {
        if (isMpvAvailable()) {
            // mpv backend not yet implemented — use Qt for now
            qWarning() << "mpv detected but not yet implemented, using Qt Multimedia";
            return std::make_unique<QtMultimediaBackend>();
        }
        return std::make_unique<QtMultimediaBackend>();
    }

    qWarning() << "Unknown backend type" << type << ", using Qt Multimedia";
    return std::make_unique<QtMultimediaBackend>();
}

bool BackendFactory::isMpvAvailable()
{
    // Check if libmpv can be loaded at runtime
    QLibrary libmpv(QStringLiteral("mpv"));
    return libmpv.load();
}

} // namespace QeriPlayerQt
