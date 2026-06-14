/// @file Artist.h
/// @brief Artist domain model
/// @date 2024-01-15

#ifndef NERIPLAYERQT_ARTIST_H
#define NERIPLAYERQT_ARTIST_H

#include "domain/Enums.h"

#include <QMetaType>
#include <QString>
#include <QUrl>

namespace NeriPlayerQt {

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

} // namespace NeriPlayerQt

Q_DECLARE_METATYPE(NeriPlayerQt::Artist)

#endif // NERIPLAYERQT_ARTIST_H
