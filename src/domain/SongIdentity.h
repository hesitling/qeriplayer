/// @file SongIdentity.h
/// @brief Song identity for cross-platform deduplication

#ifndef NERIPLAYERQT_SONGIDENTITY_H
#define NERIPLAYERQT_SONGIDENTITY_H

#include <QMetaType>
#include <QString>

namespace NeriPlayerQt {

/**
 * @brief Stable identity for cross-platform song deduplication
 *
 * Aligned with Android NeriPlayer's SongIdentity model.
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

} // namespace NeriPlayerQt

// Include Song.h here (after the declaration) for songIdentityFromSong implementation.
// This avoids circular dependency since Song.h does not include SongIdentity.h.
#include "domain/Song.h"

namespace NeriPlayerQt {

inline SongIdentity songIdentityFromSong(const Song &song)
{
    return SongIdentity { song.id, song.album, song.mediaUri.toString() };
}

} // namespace NeriPlayerQt

Q_DECLARE_METATYPE(NeriPlayerQt::SongIdentity)

#endif // NERIPLAYERQT_SONGIDENTITY_H
