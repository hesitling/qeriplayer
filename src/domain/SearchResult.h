/// @file SearchResult.h
/// @brief Search result domain model

#ifndef QERIPLAYERQT_SEARCHRESULT_H
#define QERIPLAYERQT_SEARCHRESULT_H

#include "domain/Album.h"
#include "domain/Artist.h"
#include "domain/Playlist.h"
#include "domain/Song.h"

#include <QMetaType>
#include <QVector>

namespace QeriPlayerQt {

/**
 * @brief Aggregated search results from one or more platforms
 */
struct SearchResult {
    QVector<Song> songs;
    QVector<Album> albums;
    QVector<Artist> artists;
    QVector<Playlist> playlists;
    int totalCount = 0;
    bool hasMore = false;
};

} // namespace QeriPlayerQt

Q_DECLARE_METATYPE(QeriPlayerQt::SearchResult)

#endif // QERIPLAYERQT_SEARCHRESULT_H
