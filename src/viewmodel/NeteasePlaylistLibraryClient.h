/// @file NeteasePlaylistLibraryClient.h
/// @brief Adapter exposing NeteaseClient through IPlaylistLibraryClient

#ifndef QERIPLAYERQT_NETEASEPLAYLISTLIBRARYCLIENT_H
#define QERIPLAYERQT_NETEASEPLAYLISTLIBRARYCLIENT_H

#include "api/netease/NeteaseClient.h"
#include "viewmodel/IPlaylistLibraryClient.h"

namespace QeriPlayerQt {

class NeteasePlaylistLibraryClient : public IPlaylistLibraryClient {
public:
    explicit NeteasePlaylistLibraryClient(NeteaseClient *neteaseClient)
        : m_neteaseClient(neteaseClient)
    {
    }

    QCoro::Task<ApiResult<QVector<Playlist>>> getUserPlaylists(const QString &userId) override
    {
        co_return co_await m_neteaseClient->getUserPlaylists(userId);
    }

    QCoro::Task<ApiResult<QJsonObject>> getUserStarredAlbums(const QString &userId, int limit = 1000,
                                                             int offset = 0) override
    {
        co_return co_await m_neteaseClient->getUserStarredAlbums(userId, limit, offset);
    }

private:
    NeteaseClient *m_neteaseClient;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_NETEASEPLAYLISTLIBRARYCLIENT_H
