/// @file AppPaths.h
/// @brief Cross-platform application directory paths
/// @date 2024-01-15

#ifndef NERIPLAYERQT_APPPATHS_H
#define NERIPLAYERQT_APPPATHS_H

#include <QString>

namespace NeriPlayerQt {

/**
 * @brief Provides platform-specific application directory paths
 *
 * - Linux: ~/.local/share/NeriPlayer, ~/.config/NeriPlayer, ~/.cache/NeriPlayer
 * - macOS: ~/Library/Application Support/NeriPlayer, ~/Library/Caches/NeriPlayer
 * - Windows: %APPDATA%/NeriPlayer, %LOCALAPPDATA%/NeriPlayer/Cache
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

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_APPPATHS_H
