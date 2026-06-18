/// @file ListenTogether.h
/// @brief Listen Together domain models for synchronized playback

#ifndef QERIPLAYERQT_LISTENTOGETHER_H
#define QERIPLAYERQT_LISTENTOGETHER_H

#include "domain/Song.h"

#include <QMetaType>
#include <QString>
#include <QUrl>
#include <QVariantMap>
#include <QVector>

namespace QeriPlayerQt {

/**
 * @brief Channel identifiers for Listen Together
 *
 * Must match Android QeriPlayer's ListenTogetherChannels constants exactly.
 */
namespace ListenTogetherChannels {
inline constexpr const char *NETEASE = "netease";
inline constexpr const char *BILIBILI = "bilibili";
inline constexpr const char *YOUTUBE_MUSIC = "youtubeMusic";
inline constexpr const char *LOCAL = "local";
} // namespace ListenTogetherChannels

/**
 * @brief Room status constants
 *
 * Must match Android QeriPlayer's ListenTogetherRoomStatuses constants exactly.
 */
namespace ListenTogetherRoomStatuses {
inline constexpr const char *ACTIVE = "active";
inline constexpr const char *CONTROLLER_OFFLINE = "controller_offline";
inline constexpr const char *CLOSED = "closed";
} // namespace ListenTogetherRoomStatuses

/**
 * @brief Wire-format track for Listen Together protocol
 *
 * Aligned with Android QeriPlayer's ListenTogetherTrack.
 * This is the JSON-serializable track representation exchanged
 * between clients and the coordination server.
 */
struct ListenTogetherTrack {
    QString stableKey;
    QString channelId;
    QString audioId;
    QString subAudioId;
    QString playlistContextId;
    QString mediaUri;
    QString streamUrl;
    QString name;
    QString artist;
    QString album;
    qint64 durationMs = 0;
    QString coverUrl;
};

/**
 * @brief Room settings for Listen Together
 *
 * Aligned with Android QeriPlayer's ListenTogetherRoomSettings.
 */
struct ListenTogetherRoomSettings {
    bool allowMemberControl = true;
    bool autoPauseOnMemberChange = true;
    bool shareAudioLinks = true;
};

/**
 * @brief Room member info
 *
 * Aligned with Android QeriPlayer's ListenTogetherMember.
 */
struct ListenTogetherMember {
    QString userUuid;
    QString nickname;
    QString userId;
    QString role;
    qint64 joinedAt = 0;
};

/**
 * @brief Playback synchronization state
 *
 * Aligned with Android QeriPlayer's ListenTogetherPlaybackState.
 */
struct ListenTogetherPlaybackState {
    QString state; ///< "playing" or "paused"
    qint64 basePositionMs = 0;
    qint64 baseTimestampMs = 0;
    double playbackRate = 1.0;
};

/**
 * @brief Full room state from the coordination server
 *
 * Aligned with Android QeriPlayer's ListenTogetherRoomState.
 */
struct ListenTogetherRoomState {
    QString roomId;
    qint64 version = 0;
    int schemaVersion = 1;
    QString controllerUserUuid;
    QString controllerUserId;
    qint64 controllerHeartbeatAt = 0;
    ListenTogetherRoomSettings settings;
    QVector<ListenTogetherMember> members;
    QVector<ListenTogetherTrack> queue;
    int currentIndex = 0;
    ListenTogetherTrack track;
    ListenTogetherPlaybackState playback;
    qint64 controllerOfflineSince = 0;
    QString roomStatus;
    QString closedReason;
    qint64 updatedAt = 0;
};

/**
 * @brief Cause info for events
 *
 * Aligned with Android QeriPlayer's ListenTogetherCause.
 */
struct ListenTogetherCause {
    QString userUuid;
    QString userId;
    QString nickname;
    QString eventId;
    QString type;
};

/**
 * @brief Connection state enum
 *
 * Aligned with Android QeriPlayer's ListenTogetherConnectionState.
 */
enum class ListenTogetherConnectionState : std::uint8_t { Disconnected = 0, Connecting = 1, Connected = 2 };

/**
 * @brief Client-side session state
 *
 * Aligned with Android QeriPlayer's ListenTogetherSessionState.
 */
struct ListenTogetherSessionState {
    QString baseUrl;
    QString roomId;
    QString userUuid;
    QString nickname;
    QString role;
    QString token;
    QString wsUrl;
    ListenTogetherConnectionState connectionState = ListenTogetherConnectionState::Disconnected;
    QString lastError;
    qint64 expectedPositionMs = 0;
    QString roomNotice;
};

/**
 * @brief Control event sent to the coordination server
 *
 * Aligned with Android QeriPlayer's ListenTogetherEvent.
 */
struct ListenTogetherEvent {
    QString type;
    QString eventId;
    qint64 clientTimeMs = 0;
    qint64 positionMs = 0;
    int currentIndex = 0;
    ListenTogetherTrack track;
    QVector<ListenTogetherTrack> queue;
    ListenTogetherRoomSettings roomSettings;
    bool shouldPlay = false;
    QString state;
    QString requestTrackStableKey;
};

// --- Conversion helpers ---

/**
 * @brief Compute stable key for a track
 *
 * Matches Android QeriPlayer's buildStableTrackKey() logic.
 */
inline QString buildStableTrackKey(const QString &channelId, const QString &audioId, const QString &subAudioId = {},
                                   const QString &playlistContextId = {})
{
    if (channelId == QString::fromLatin1(ListenTogetherChannels::BILIBILI)) {
        QStringList parts {channelId, audioId};
        if (!subAudioId.isEmpty())
            parts.append(subAudioId);
        return parts.join(QStringLiteral(":"));
    }
    if (channelId == QString::fromLatin1(ListenTogetherChannels::YOUTUBE_MUSIC)) {
        QStringList parts {channelId, audioId};
        if (!playlistContextId.isEmpty())
            parts.append(playlistContextId);
        return parts.join(QStringLiteral(":"));
    }
    return channelId + QStringLiteral(":") + audioId;
}

/**
 * @brief Resolve channel ID from a Song
 *
 * Matches Android QeriPlayer's SongItem.resolvedChannelId().
 */
inline QString resolvedChannelId(const Song &song)
{
    if (!song.channelId.isEmpty())
        return song.channelId;
    // Fallback heuristics — same as Android
    if (!song.localFilePath.isEmpty())
        return QString::fromLatin1(ListenTogetherChannels::LOCAL);
    if (song.mediaUri.toString().contains(QStringLiteral("youtube.com"))
        || song.mediaUri.toString().contains(QStringLiteral("youtu.be")))
        return QString::fromLatin1(ListenTogetherChannels::YOUTUBE_MUSIC);
    return QString::fromLatin1(ListenTogetherChannels::NETEASE);
}

/**
 * @brief Resolve audio ID from a Song
 *
 * Matches Android QeriPlayer's SongItem.resolvedAudioId().
 */
inline QString resolvedAudioId(const Song &song)
{
    if (!song.audioId.isEmpty())
        return song.audioId;
    return song.id;
}

/**
 * @brief Convert a Song to ListenTogetherTrack for wire transmission
 *
 * Matches Android QeriPlayer's SongItem.toListenTogetherTrackOrNull().
 * Returns an empty optional-equivalent if the song has no resolvable channel.
 */
inline ListenTogetherTrack songToTrack(const Song &song)
{
    const QString channel = resolvedChannelId(song);
    const QString audio = resolvedAudioId(song);
    const QString subAudio = song.subAudioId;
    const QString playlistContext = song.playlistContextId;
    const QString stableKey = buildStableTrackKey(channel, audio, subAudio, playlistContext);

    return ListenTogetherTrack {stableKey,
                                channel,
                                audio,
                                subAudio,
                                playlistContext,
                                song.mediaUri.toString(),
                                song.streamUrl,
                                song.customName.isEmpty() ? song.name : song.customName,
                                song.customArtist.isEmpty() ? song.artist : song.customArtist,
                                song.album,
                                song.durationMs,
                                song.customCoverUrl.isEmpty() ? song.coverUrl.toString() : song.customCoverUrl};
}

/**
 * @brief Convert a ListenTogetherTrack back to a Song
 *
 * Matches Android QeriPlayer's ListenTogetherTrack.toSongItem().
 */
inline Song trackToSong(const ListenTogetherTrack &track)
{
    Song song;
    song.name = track.name;
    song.artist = track.artist;
    song.album = track.album;
    song.durationMs = track.durationMs;
    song.coverUrl = QUrl(track.coverUrl);
    song.mediaUri = QUrl(track.mediaUri);
    song.channelId = track.channelId;
    song.audioId = track.audioId;
    song.subAudioId = track.subAudioId;
    song.playlistContextId = track.playlistContextId;
    song.streamUrl = track.streamUrl;

    // Compute a synthetic ID from audioId
    bool ok = false;
    qint64 numericId = track.audioId.toLongLong(&ok);
    song.id = ok ? QString::number(numericId) : QString::number(qHash(track.stableKey));

    return song;
}

} // namespace QeriPlayerQt

Q_DECLARE_METATYPE(QeriPlayerQt::ListenTogetherTrack)
Q_DECLARE_METATYPE(QeriPlayerQt::ListenTogetherRoomSettings)
Q_DECLARE_METATYPE(QeriPlayerQt::ListenTogetherMember)
Q_DECLARE_METATYPE(QeriPlayerQt::ListenTogetherPlaybackState)
Q_DECLARE_METATYPE(QeriPlayerQt::ListenTogetherRoomState)
Q_DECLARE_METATYPE(QeriPlayerQt::ListenTogetherCause)
Q_DECLARE_METATYPE(QeriPlayerQt::ListenTogetherConnectionState)
Q_DECLARE_METATYPE(QeriPlayerQt::ListenTogetherSessionState)
Q_DECLARE_METATYPE(QeriPlayerQt::ListenTogetherEvent)

#endif // QERIPLAYERQT_LISTENTOGETHER_H
