/// @file PlayerStateRepository.h
/// @brief SQLite-backed player state repository

#ifndef NERIPLAYERQT_PLAYERSTATEREPOSITORY_H
#define NERIPLAYERQT_PLAYERSTATEREPOSITORY_H

#include "repo/IPlayerStateRepository.h"

namespace NeriPlayerQt {

class DatabaseManager;

/**
 * @brief SQLite implementation of IPlayerStateRepository
 *
 * Uses the player_state singleton table (CHECK(id=1)) to persist
 * the current playback state across sessions.
 */
class PlayerStateRepository : public IPlayerStateRepository {
public:
    explicit PlayerStateRepository(DatabaseManager *db);

    void save(const PersistedPlayerState &state) override;
    std::optional<PersistedPlayerState> load() override;
    void clear() override;

private:
    DatabaseManager *m_db;
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_PLAYERSTATEREPOSITORY_H
