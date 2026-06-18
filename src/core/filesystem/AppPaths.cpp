/// @file AppPaths.cpp
/// @brief Cross-platform application directory paths

#include "core/filesystem/AppPaths.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

namespace QeriPlayerQt {

QString AppPaths::dataDir()
{
#if defined(Q_OS_WIN)
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#elif defined(Q_OS_MACOS)
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#else // Linux
    QString path = QDir::homePath() + QStringLiteral("/.local/share/QeriPlayer");
#endif
    return ensureCreated(path);
}

QString AppPaths::configDir()
{
#if defined(Q_OS_WIN)
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#elif defined(Q_OS_MACOS)
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#else // Linux
    QString path = QDir::homePath() + QStringLiteral("/.config/QeriPlayer");
#endif
    return ensureCreated(path);
}

QString AppPaths::cacheDir()
{
#if defined(Q_OS_WIN)
    QString path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
#elif defined(Q_OS_MACOS)
    QString path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
#else // Linux
    QString path = QDir::homePath() + QStringLiteral("/.cache/QeriPlayer");
#endif
    return ensureCreated(path);
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
