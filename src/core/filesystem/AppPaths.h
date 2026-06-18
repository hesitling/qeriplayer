/// @file AppPaths.h
/// @brief Cross-platform application directory paths

#ifndef QERIPLAYERQT_APPPATHS_H
#define QERIPLAYERQT_APPPATHS_H

#include <QString>

namespace QeriPlayerQt {

/**
 * @brief Provides platform-specific application directory paths
 *
 * Linux uses hardcoded paths to match QeriPlayer naming convention:
 *   ~/.local/share/QeriPlayer, ~/.config/QeriPlayer, ~/.cache/QeriPlayer
 *
 * Windows and macOS use QStandardPaths (AppDataLocation, CacheLocation).
 */
class AppPaths {
public:
    /**
     * @brief Directory for persistent application data (database, downloads)
     */
    static QString dataDir();

    /**
     * @brief Directory for configuration files
     */
    static QString configDir();

    /**
     * @brief Directory for cache files (album art, temp API responses)
     */
    static QString cacheDir();

    /**
     * @brief Temporary directory for the application
     */
    static QString tempDir();

private:
    static QString ensureCreated(const QString &path);
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_APPPATHS_H
