/// @file IPlayHistoryRepository.h
/// @brief Interface for play history persistence operations

#ifndef QERIPLAYERQT_IPLAYHISTORYREPOSITORY_H
#define QERIPLAYERQT_IPLAYHISTORYREPOSITORY_H

#include "domain/Song.h"

#include <QString>
#include <QStringList>
#include <QVector>

namespace QeriPlayerQt {

/**
 * @brief Abstract interface for play history operations on play_history table
 */
class IPlayHistoryRepository {
public:
    virtual ~IPlayHistoryRepository() = default;

    /**
     * @brief Record that a song was played (inserts history row + updates last_played_at)
     */
    virtual void record(const QString &songId) = 0;

    /**
     * @brief Get recently played distinct songs, most recent first
     * @param limit Maximum number of songs
     * @return Full Song objects joined from songs_cache
     */
    virtual QVector<Song> recent(int limit = 50) = 0;

    /**
     * @brief Delete all play history entries (songs_cache unaffected)
     */
    virtual void clear() = 0;

    /**
     * @brief Remove history entries for specific song IDs
     */
    virtual void remove(const QStringList &songIds) = 0;

    /**
     * @brief Count how many times a song has been played
     */
    virtual int playCount(const QString &songId) = 0;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_IPLAYHISTORYREPOSITORY_H
