/// @file SearchResult.h
/// @brief Search result domain model

#ifndef NERIPLAYERQT_SEARCHRESULT_H
#define NERIPLAYERQT_SEARCHRESULT_H

#include "domain/Album.h"
#include "domain/Artist.h"
#include "domain/Playlist.h"
#include "domain/Song.h"

#include <QMetaType>
#include <QVector>

namespace NeriPlayerQt {

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

} // namespace NeriPlayerQt

Q_DECLARE_METATYPE(NeriPlayerQt::SearchResult)

#endif // NERIPLAYERQT_SEARCHRESULT_H
