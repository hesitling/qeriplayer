/// @file PlayHistory.h
/// @brief Play history type

#ifndef NERIPLAYERQT_PLAYHISTORY_H
#define NERIPLAYERQT_PLAYHISTORY_H

#include "domain/Song.h"

#include <QMetaType>
#include <QString>

namespace NeriPlayerQt {

/**
 * @brief A record of a song play event
 */
struct PlayHistory {
    Song song;
    qint64 playedAt = 0; ///< Epoch milliseconds when played
    int playCount = 0;   ///< Total play count for this song
};

} // namespace NeriPlayerQt

Q_DECLARE_METATYPE(NeriPlayerQt::PlayHistory)

#endif // NERIPLAYERQT_PLAYHISTORY_H
