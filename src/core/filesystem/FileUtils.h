/// @file FileUtils.h
/// @brief Safe file I/O utilities with atomic write support

#ifndef NERIPLAYERQT_FILEUTILS_H
#define NERIPLAYERQT_FILEUTILS_H

#include <QByteArray>
#include <QString>

namespace NeriPlayerQt {

/**
 * @brief Utility class for safe file operations
 */
class FileUtils {
public:
    /**
     * @brief Create a directory and all parent directories
     * @param path Directory path to create
     * @return true on success or if already exists
     */
    static bool ensureDir(const QString &path);

    /**
     * @brief Read file contents
     * @param path Path to the file
     * @return File contents, or empty QByteArray on error
     */
    static QByteArray readFile(const QString &path);

    /**
     * @brief Write data to a file atomically (write to temp, then rename)
     * @param path Target file path
     * @param data Data to write
     * @return true on success
     */
    static bool writeFile(const QString &path, const QByteArray &data);

    /**
     * @brief Get the last error message
     */
    static QString lastError();

private:
    static thread_local QString s_lastError;
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_FILEUTILS_H
