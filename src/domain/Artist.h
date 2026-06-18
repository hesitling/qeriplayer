/// @file Artist.h
/// @brief Artist domain model

#ifndef QERIPLAYERQT_ARTIST_H
#define QERIPLAYERQT_ARTIST_H

#include "domain/Enums.h"

#include <QMetaType>
#include <QString>
#include <QUrl>

namespace QeriPlayerQt {

/**
 * @brief Represents an artist
 */
struct Artist {
    QString id;
    QString name;
    QUrl avatarUrl;
    QString description;
    MusicPlatform platform = MusicPlatform::Unknown;
};

} // namespace QeriPlayerQt

Q_DECLARE_METATYPE(QeriPlayerQt::Artist)

#endif // QERIPLAYERQT_ARTIST_H
