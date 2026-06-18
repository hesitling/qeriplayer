/// @file PlayerStateRepository.cpp
/// @brief SQLite-backed player state repository implementation

#include "repo/PlayerStateRepository.h"

#include "core/database/DatabaseManager.h"
#include "repo/SqlRowMapper.h"

#include <QDateTime>

namespace QeriPlayerQt {

PlayerStateRepository::PlayerStateRepository(DatabaseManager *db)
    : m_db(db)
{
}

void PlayerStateRepository::save(const PersistedPlayerState &state)
{
    QVariantMap json = SqlRowMapper::playerStateToJson(state);
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    m_db->exec("INSERT OR REPLACE INTO player_state "
               "(id, playlist_json, current_index, media_url, position_ms, "
               "should_resume, repeat_mode, shuffle_enabled, updated_at) "
               "VALUES (1, ?, ?, ?, ?, ?, ?, ?, ?)",
               {
                   json["playlist_json"],
                   json["current_index"],
                   json["media_url"],
                   json["position_ms"],
                   json["should_resume"],
                   json["repeat_mode"],
                   json["shuffle_enabled"],
                   now,
               });
}

std::optional<PersistedPlayerState> PlayerStateRepository::load()
{
    auto rows = m_db->exec("SELECT playlist_json, current_index, media_url, position_ms, "
                           "should_resume, repeat_mode, shuffle_enabled, updated_at "
                           "FROM player_state WHERE id = 1");

    if (rows.isEmpty())
        return std::nullopt;

    QVariantMap map;
    map["playlist_json"] = rows[0][0];
    map["current_index"] = rows[0][1];
    map["media_url"] = rows[0][2];
    map["position_ms"] = rows[0][3];
    map["should_resume"] = rows[0][4];
    map["repeat_mode"] = rows[0][5];
    map["shuffle_enabled"] = rows[0][6];

    return SqlRowMapper::playerStateFromJson(map);
}

void PlayerStateRepository::clear()
{
    m_db->exec("DELETE FROM player_state");
}

} // namespace QeriPlayerQt
