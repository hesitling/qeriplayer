/// @file Album.h
/// @brief Album domain model

#ifndef QERIPLAYERQT_ALBUM_H
#define QERIPLAYERQT_ALBUM_H

#include "domain/Enums.h"

#include <QMetaType>
#include <QString>
#include <QUrl>

namespace QeriPlayerQt {

/**
 * @brief Represents an album
 *
 * Field names aligned with Android QeriPlayer's AlbumSummary model.
 */
struct Album {
    QString id;
    QString name;
    QString artist;
    QUrl coverUrl;
    int size = 0; ///< Track count
    MusicPlatform platform = MusicPlatform::Unknown;
};

} // namespace QeriPlayerQt

Q_DECLARE_METATYPE(QeriPlayerQt::Album)

#endif // QERIPLAYERQT_ALBUM_H
