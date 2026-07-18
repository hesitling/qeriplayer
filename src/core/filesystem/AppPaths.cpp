/// @file AppPaths.cpp
/// @brief Cross-platform application directory paths

#include "core/filesystem/AppPaths.h"

#include <QDir>
#include <QStandardPaths>

namespace QeriPlayerQt {

namespace {

QString appPath(QStandardPaths::StandardLocation location)
{
    return QStandardPaths::writableLocation(location) + QStringLiteral("/QeriPlayer");
}

} // namespace

QString AppPaths::dataDir()
{
    return ensureCreated(appPath(QStandardPaths::GenericDataLocation));
}

QString AppPaths::configDir()
{
    return ensureCreated(appPath(QStandardPaths::GenericConfigLocation));
}

QString AppPaths::cacheDir()
{
    return ensureCreated(appPath(QStandardPaths::GenericCacheLocation));
}

QString AppPaths::tempDir()
{
    QString path = QDir::tempPath() + QStringLiteral("/QeriPlayer");
    return ensureCreated(path);
}

QString AppPaths::ensureCreated(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "AppPaths: failed to create directory:" << path;
            return {};
        }
    }
    return dir.absolutePath();
}

} // namespace QeriPlayerQt
