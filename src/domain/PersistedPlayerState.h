/// @file PersistedPlayerState.h
/// @brief Player state persistence models

#ifndef NERIPLAYERQT_PERSISTEDPLAYERSTATE_H
#define NERIPLAYERQT_PERSISTEDPLAYERSTATE_H

#include "domain/Enums.h"
#include "domain/Song.h"

#include <QMetaType>
#include <QVector>

namespace NeriPlayerQt {

/**
 * @brief Serialized player state for persistence across sessions
 *
 * Aligned with Android NeriPlayer's PersistedState model.
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

} // namespace NeriPlayerQt

Q_DECLARE_METATYPE(NeriPlayerQt::PersistedPlayerState)

#endif // NERIPLAYERQT_PERSISTEDPLAYERSTATE_H
