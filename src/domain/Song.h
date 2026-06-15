/// @file Song.h
/// @brief Song domain model

#ifndef NERIPLAYERQT_SONG_H
#define NERIPLAYERQT_SONG_H

#include "domain/Enums.h"

#include <QMetaType>
#include <QString>
#include <QUrl>
#include <QVariantMap>

namespace NeriPlayerQt {

/**
 * @brief Represents a song across all platforms
 *
 * Aligned with Android NeriPlayer's SongItem model.
 * Includes customization fields (custom/original) for user overrides,
 * lyric matching fields, local file support, and platform-specific identifiers.
 */
struct Song {
    // Core identity
    QString id;
    QString name;
    QString artist;
    QString album;
    QString albumId;
    qint64 durationMs = 0; ///< Duration in milliseconds
    QUrl coverUrl;
    QUrl mediaUri;
    MusicPlatform platform = MusicPlatform::Unknown;

    // Lyric matching
    QString matchedLyric;
    QString matchedTranslatedLyric;
    MusicPlatform matchedLyricSource = MusicPlatform::Unknown;
    QString matchedSongId;
    qint64 userLyricOffsetMs = 0;

    // User customizations (overrides)
    QString customCoverUrl;
    QString customName;
    QString customArtist;

    // Original values (before user edits)
    QString originalName;
    QString originalArtist;
    QString originalCoverUrl;
    QString originalLyric;
    QString originalTranslatedLyric;

    // Local file support
    QString localFileName;
    QString localFilePath;

    // Platform-specific identifiers
    QString channelId;
    QString audioId;
    QString subAudioId;
    QString playlistContextId;
    QString streamUrl;

    // Arbitrary platform-specific data
    QVariantMap extra;
};

} // namespace NeriPlayerQt

Q_DECLARE_METATYPE(NeriPlayerQt::Song)

#endif // NERIPLAYERQT_SONG_H
