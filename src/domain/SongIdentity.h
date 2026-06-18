/// @file SongIdentity.h
/// @brief Song identity for cross-platform deduplication

#ifndef QERIPLAYERQT_SONGIDENTITY_H
#define QERIPLAYERQT_SONGIDENTITY_H

#include <QMetaType>
#include <QString>

namespace QeriPlayerQt {

/**
 * @brief Stable identity for cross-platform song deduplication
 *
 * Aligned with Android QeriPlayer's SongIdentity model.
 * Used to determine if two song entries from different sources
 * represent the same logical song.
 */
struct SongIdentity {
    QString id;
    QString album;
    QString mediaUri;

    /**
     * @brief Generate a stable key string for hashing/comparison
     */
    QString stableKey() const
    {
        return id + QStringLiteral("|") + album + QStringLiteral("|") + mediaUri;
    }

    bool operator==(const SongIdentity &other) const
    {
        return id == other.id && album == other.album && mediaUri == other.mediaUri;
    }

    bool operator!=(const SongIdentity &other) const
    {
        return !(*this == other);
    }
};

struct Song;

/**
 * @brief Extract identity from a Song
 */
inline SongIdentity songIdentityFromSong(const Song &song);

} // namespace QeriPlayerQt

// Include Song.h here (after the declaration) for songIdentityFromSong implementation.
// This avoids circular dependency since Song.h does not include SongIdentity.h.
#include "domain/Song.h"

namespace QeriPlayerQt {

inline SongIdentity songIdentityFromSong(const Song &song)
{
    return SongIdentity {song.id, song.album, song.mediaUri.toString()};
}

} // namespace QeriPlayerQt

Q_DECLARE_METATYPE(QeriPlayerQt::SongIdentity)

#endif // QERIPLAYERQT_SONGIDENTITY_H
