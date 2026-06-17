/// @file IPlayerStateRepository.h
/// @brief Interface for player state persistence operations

#ifndef NERIPLAYERQT_IPLAYERSTATEREPOSITORY_H
#define NERIPLAYERQT_IPLAYERSTATEREPOSITORY_H

#include "domain/PersistedPlayerState.h"

#include <optional>

namespace NeriPlayerQt {

/**
 * @brief Abstract interface for player state operations on player_state singleton table
 */
class IPlayerStateRepository {
public:
    virtual ~IPlayerStateRepository() = default;

    /**
     * @brief Persist the current player state (INSERT OR REPLACE)
     */
    virtual void save(const PersistedPlayerState &state) = 0;

    /**
     * @brief Load the persisted player state
     * @return State if one has been saved, empty optional otherwise
     */
    virtual std::optional<PersistedPlayerState> load() = 0;

    /**
     * @brief Delete the persisted player state
     */
    virtual void clear() = 0;
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_IPLAYERSTATEREPOSITORY_H
