/// @file IPlaylistLibraryClient.h
/// @brief Interface for remote library overview data used by PlaylistViewModel

#ifndef QERIPLAYERQT_IPLAYLISTLIBRARYCLIENT_H
#define QERIPLAYERQT_IPLAYLISTLIBRARYCLIENT_H

#include "api/common/ApiResult.h"
#include "domain/Playlist.h"

#include <QCoroTask>
#include <QJsonObject>
#include <QString>
#include <QVector>

namespace QeriPlayerQt {

class IPlaylistLibraryClient {
public:
    virtual ~IPlaylistLibraryClient() = default;

    virtual QCoro::Task<ApiResult<QVector<Playlist>>> getUserPlaylists(const QString &userId) = 0;
    virtual QCoro::Task<ApiResult<QJsonObject>> getUserStarredAlbums(const QString &userId, int limit = 1000,
                                                                     int offset = 0)
        = 0;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_IPLAYLISTLIBRARYCLIENT_H
