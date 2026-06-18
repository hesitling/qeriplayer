/// @file PlayHistory.h
/// @brief Play history type

#ifndef QERIPLAYERQT_PLAYHISTORY_H
#define QERIPLAYERQT_PLAYHISTORY_H

#include "domain/Song.h"

#include <QMetaType>
#include <QString>

namespace QeriPlayerQt {

/**
 * @brief A record of a song play event
 */
struct PlayHistory {
    Song song;
    qint64 playedAt = 0; ///< Epoch milliseconds when played
    int playCount = 0;   ///< Total play count for this song
};

} // namespace QeriPlayerQt

Q_DECLARE_METATYPE(QeriPlayerQt::PlayHistory)

#endif // QERIPLAYERQT_PLAYHISTORY_H
