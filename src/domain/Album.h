/// @file Album.h
/// @brief Album domain model

#ifndef NERIPLAYERQT_ALBUM_H
#define NERIPLAYERQT_ALBUM_H

#include "domain/Enums.h"

#include <QMetaType>
#include <QString>
#include <QUrl>

namespace NeriPlayerQt {

/**
 * @brief Represents an album
 *
 * Field names aligned with Android NeriPlayer's AlbumSummary model.
 */
struct Album {
    QString id;
    QString name;
    QString artist;
    QUrl coverUrl;
    int size = 0; ///< Track count
    MusicPlatform platform = MusicPlatform::Unknown;
};

} // namespace NeriPlayerQt

Q_DECLARE_METATYPE(NeriPlayerQt::Album)

#endif // NERIPLAYERQT_ALBUM_H
