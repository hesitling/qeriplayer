/// @file IMusicPlatformPlugin.h
/// @brief Abstract interface for music platform clients

#ifndef QERIPLAYERQT_IMUSICPLATFORMPLUGIN_H
#define QERIPLAYERQT_IMUSICPLATFORMPLUGIN_H

#include "api/common/ApiResult.h"
#include "domain/Enums.h"
#include "domain/Lyrics.h"
#include "domain/SearchResult.h"
#include "domain/Song.h"
#include "domain/SongUrlResult.h"

#include <QCoroTask>
#include <QString>

namespace QeriPlayerQt {

/**
 * @brief Abstract interface that all music platform clients implement
 *
 * Defines the cross-platform contract for search, song detail,
 * playback URL resolution, lyrics, and authentication status.
 * Platform-specific operations (playlist CRUD, user operations, etc.)
 * live on the concrete client class.
 */
class IMusicPlatformPlugin {
public:
    virtual ~IMusicPlatformPlugin() = default;

    /**
     * @brief Search for content across the platform
     * @param keyword Search query
     * @param type Type of content to search for
     * @param limit Maximum number of results
     * @param offset Pagination offset
     * @return Search results or error
     */
    virtual QCoro::Task<ApiResult<SearchResult>> search(const QString &keyword, SearchType type, int limit, int offset)
        = 0;

    /**
     * @brief Get detailed information about a song
     * @param songId Platform-specific song identifier
     * @return Song details or error
     */
    virtual QCoro::Task<ApiResult<Song>> getSongDetail(const QString &songId) = 0;

    /**
     * @brief Get the playback URL for a song
     * @param songId Platform-specific song identifier
     * @param quality Desired audio quality
     * @return Song URL result or error
     */
    virtual QCoro::Task<ApiResult<SongUrlResult>> getSongUrl(const QString &songId,
                                                             AudioQuality quality = AudioQuality::High) = 0;

    /**
     * @brief Get lyrics for a song
     * @param songId Platform-specific song identifier
     * @return Lyrics or error
     */
    virtual QCoro::Task<ApiResult<Lyrics>> getLyrics(const QString &songId) = 0;

    /**
     * @brief Check if the client is currently authenticated
     *
     * Synchronous by design — for most platforms this is an in-memory
     * cookie/token check with no network call.
     */
    virtual bool isAuthenticated() const = 0;

    /**
     * @brief Get the platform name
     * @return Human-readable platform name (e.g., "NetEase")
     */
    virtual QString platformName() const = 0;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_IMUSICPLATFORMPLUGIN_H
