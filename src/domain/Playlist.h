/// @file Playlist.h
/// @brief Playlist domain model

#ifndef QERIPLAYERQT_PLAYLIST_H
#define QERIPLAYERQT_PLAYLIST_H

#include "domain/Enums.h"
#include "domain/Song.h"

#include <QMetaType>
#include <QString>
#include <QUrl>
#include <QVector>

namespace QeriPlayerQt {

/**
 * @brief Represents a playlist with optional embedded songs
 *
 * Aligned with Android QeriPlayer's LocalPlaylist model.
 */
struct Playlist {
    QString id;
    QString name;
    QString description;
    QUrl coverUrl;
    int songCount = 0;
    QString owner;
    MusicPlatform platform = MusicPlatform::Unknown;
    QVector<Song> songs;    ///< Optionally populated
    qint64 modifiedAt = 0;  ///< Epoch milliseconds, aligned with Android
    QString customCoverUrl; ///< User override cover
};

} // namespace QeriPlayerQt

Q_DECLARE_METATYPE(QeriPlayerQt::Playlist)

#endif // QERIPLAYERQT_PLAYLIST_H
