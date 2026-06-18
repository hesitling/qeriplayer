/// @file PersistedPlayerState.h
/// @brief Player state persistence models

#ifndef QERIPLAYERQT_PERSISTEDPLAYERSTATE_H
#define QERIPLAYERQT_PERSISTEDPLAYERSTATE_H

#include "domain/Enums.h"
#include "domain/Song.h"

#include <QMetaType>
#include <QVector>

namespace QeriPlayerQt {

/**
 * @brief Serialized player state for persistence across sessions
 *
 * Aligned with Android QeriPlayer's PersistedState model.
 */
struct PersistedPlayerState {
    QVector<Song> playlist;
    int currentIndex = 0;
    QString mediaUrl;
    qint64 positionMs = 0;
    bool shouldResumePlayback = false;
    RepeatMode repeatMode = RepeatMode::Off;
    bool shuffleEnabled = false;
};

} // namespace QeriPlayerQt

Q_DECLARE_METATYPE(QeriPlayerQt::PersistedPlayerState)

#endif // QERIPLAYERQT_PERSISTEDPLAYERSTATE_H
