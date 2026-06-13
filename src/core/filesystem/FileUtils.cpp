/// @file FileUtils.cpp
/// @brief Safe file I/O utilities implementation
/// @date 2024-01-15

#include "core/filesystem/FileUtils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>

namespace NeriPlayerQt {

QString FileUtils::s_lastError;

bool FileUtils::ensureDir(const QString &path)
{
    QDir dir(path);
    if (dir.exists()) {
        return true;
    }
    return dir.mkpath(".");
}

QByteArray FileUtils::readFile(const QString &path)
{
    s_lastError.clear();

    QFile file(path);
    if (!file.exists()) {
        s_lastError = QStringLiteral("File does not exist: %1").arg(path);
        return {};
    }

    if (!file.open(QIODevice::ReadOnly)) {
        s_lastError = QStringLiteral("Cannot open file: %1 — %2")
                          .arg(path, file.errorString());
        return {};
    }

    return file.readAll();
}

bool FileUtils::writeFile(const QString &path, const QByteArray &data)
{
    s_lastError.clear();

    // Ensure parent directory exists
    QFileInfo info(path);
    if (!info.dir().exists()) {
        QDir().mkpath(info.dir().absolutePath());
    }

    // Write to a temporary file in the same directory (same filesystem for atomic rename)
    QTemporaryFile tempFile(info.dir().filePath("neri_tmp_XXXXXX"));
    tempFile.setAutoRemove(false);

    if (!tempFile.open()) {
        s_lastError = QStringLiteral("Cannot create temp file: %1")
                          .arg(tempFile.errorString());
        return false;
    }

    if (tempFile.write(data) != data.size()) {
        s_lastError = QStringLiteral("Write to temp file failed: %1")
                          .arg(tempFile.errorString());
        tempFile.remove();
        return false;
    }

    tempFile.close();

    // Atomic rename
    QFile targetFile(path);
    if (targetFile.exists()) {
        targetFile.remove();
    }

    if (!QFile::rename(tempFile.fileName(), path)) {
        s_lastError = QStringLiteral("Rename from %1 to %2 failed")
                          .arg(tempFile.fileName(), path);
        tempFile.remove();
        return false;
    }

    return true;
}

QString FileUtils::lastError()
{
    return s_lastError;
}

} // namespace NeriPlayerQt
