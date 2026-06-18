/// @file PlaylistSummary.h
/// @brief Lightweight summary models for playlist and album cards

#ifndef QERIPLAYERQT_PLAYLISTSUMMARY_H
#define QERIPLAYERQT_PLAYLISTSUMMARY_H

#include "domain/Enums.h"

#include <QMetaType>
#include <QString>
#include <QUrl>

namespace QeriPlayerQt {

/**
 * @brief Lightweight playlist summary for card UI
 *
 * Aligned with Android QeriPlayer's PlaylistSummary model.
 */
struct PlaylistSummary {
    Q_GADGET
    Q_PROPERTY(QString id READ getId CONSTANT)
    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QUrl coverUrl READ getCoverUrl CONSTANT)
    Q_PROPERTY(qint64 playCount READ getPlayCount CONSTANT)
    Q_PROPERTY(int trackCount READ getTrackCount CONSTANT)

    QString getId() const
    {
        return id;
    }
    QString getName() const
    {
        return name;
    }
    QUrl getCoverUrl() const
    {
        return coverUrl;
    }
    qint64 getPlayCount() const
    {
        return playCount;
    }
    int getTrackCount() const
    {
        return trackCount;
    }

public:
    QString id;
    QString name;
    QUrl coverUrl;
    qint64 playCount = 0;
    int trackCount = 0;
};

/**
 * @brief Lightweight album summary for card UI
 *
 * Aligned with Android QeriPlayer's AlbumSummary model.
 */
struct AlbumSummary {
    Q_GADGET
    Q_PROPERTY(QString id READ getId CONSTANT)
    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QUrl coverUrl READ getCoverUrl CONSTANT)
    Q_PROPERTY(int size READ getSize CONSTANT)

    QString getId() const
    {
        return id;
    }
    QString getName() const
    {
        return name;
    }
    QUrl getCoverUrl() const
    {
        return coverUrl;
    }
    int getSize() const
    {
        return size;
    }

public:
    QString id;
    QString name;
    QUrl coverUrl;
    int size = 0;
};

/**
 * @brief Bilibili playlist summary
 *
 * Aligned with Android QeriPlayer's BiliPlaylist model.
 */
struct BiliPlaylist {
    qint64 mediaId = 0;
    qint64 fid = 0;
    qint64 mid = 0;
    QString title;
    int count = 0;
    QUrl coverUrl;
    BiliPlaylistKind kind = BiliPlaylistKind::CreatedFavorite;
    QString subtitle;
};

} // namespace QeriPlayerQt

Q_DECLARE_METATYPE(QeriPlayerQt::PlaylistSummary)
Q_DECLARE_METATYPE(QeriPlayerQt::AlbumSummary)
Q_DECLARE_METATYPE(QeriPlayerQt::BiliPlaylist)

#endif // QERIPLAYERQT_PLAYLISTSUMMARY_H
