/// @file SongIdentity.h
/// @brief Song identity for cross-platform deduplication
/// @date 2024-01-15

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
    QString stableKey() const;

    bool operator==(const SongIdentity &other) const;
    bool operator!=(const SongIdentity &other) const;
};

struct Song;

/**
 * @brief Extract identity from a Song
 */
SongIdentity songIdentityFromSong(const Song &song);

} // namespace NeriPlayerQt

Q_DECLARE_METATYPE(NeriPlayerQt::SongIdentity)

#endif // NERIPLAYERQT_SONGIDENTITY_H
