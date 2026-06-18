/// @file BackendFactory.h
/// @brief Factory for creating audio playback backends

#ifndef QERIPLAYERQT_BACKENDFACTORY_H
#define QERIPLAYERQT_BACKENDFACTORY_H

#include "player/IPlayerBackend.h"

#include <memory>

namespace QeriPlayerQt {

/**
 * @brief Factory for creating IPlayerBackend instances
 *
 * Supports "qt" (Qt Multimedia), "mpv" (future), and "auto" (prefer mpv
 * if available, fallback to Qt).
 */
class BackendFactory {
public:
    /**
     * @brief Create a backend of the specified type
     * @param type Backend type: "qt", "mpv", "auto"
     * @return Unique pointer to the created backend
     */
    [[nodiscard]] static std::unique_ptr<IPlayerBackend> createBackend(const QString &type);

    /**
     * @brief Check if libmpv is available at runtime
     * @return true if libmpv can be loaded
     */
    [[nodiscard]] static bool isMpvAvailable();
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_BACKENDFACTORY_H
