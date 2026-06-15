/// @file PlaylistSummary.h
/// @brief Lightweight summary models for playlist and album cards

#ifndef NERIPLAYERQT_PLAYLISTSUMMARY_H
#define NERIPLAYERQT_PLAYLISTSUMMARY_H

#include "domain/Enums.h"

#include <QMetaType>
#include <QString>
#include <QUrl>

namespace NeriPlayerQt {

/**
 * @brief Lightweight playlist summary for card UI
 *
 * Aligned with Android NeriPlayer's PlaylistSummary model.
 */
struct PlaylistSummary {
    QString id;
    QString name;
    QUrl coverUrl;
    qint64 playCount = 0;
    int trackCount = 0;
};

/**
 * @brief Lightweight album summary for card UI
 *
 * Aligned with Android NeriPlayer's AlbumSummary model.
 */
struct AlbumSummary {
    QString id;
    QString name;
    QUrl coverUrl;
    int size = 0;
};

/**
 * @brief Bilibili playlist summary
 *
 * Aligned with Android NeriPlayer's BiliPlaylist model.
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

} // namespace NeriPlayerQt

Q_DECLARE_METATYPE(NeriPlayerQt::PlaylistSummary)
Q_DECLARE_METATYPE(NeriPlayerQt::AlbumSummary)
Q_DECLARE_METATYPE(NeriPlayerQt::BiliPlaylist)

#endif // NERIPLAYERQT_PLAYLISTSUMMARY_H
