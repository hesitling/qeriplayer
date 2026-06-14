/// @file SongUrlResult.h
/// @brief Song URL resolution result
/// @date 2024-01-15

#ifndef NERIPLAYERQT_SONGURLRESULT_H
#define NERIPLAYERQT_SONGURLRESULT_H

#include <QMetaType>
#include <QString>

namespace NeriPlayerQt {

/**
 * @brief Audio metadata from URL resolution
 *
 * Aligned with Android NeriPlayer's PlaybackAudioInfo.
 */
struct AudioInfo {
    QString qualityKey;
    QString qualityLabel;
    QString codecLabel;
    QString mimeType;
    int bitrateKbps = 0;
    int sampleRateHz = 0;
    int bitDepth = 0;
    int channelCount = 0;
};

/**
 * @brief Result of resolving a song's playback URL
 *
 * Aligned with Android NeriPlayer's SongUrlResult sealed class.
 */
struct SongUrlResult {
    enum class Status : std::uint8_t {
        Success = 0,
        WaitingForAuthoritativeStream,
        RequiresLogin,
        Failure
    };

    Status status = Status::Failure;
    QString url;
    qint64 durationMs = 0;
    QString mimeType;
    QString noticeMessage;
    qint64 expectedContentLength = 0;
    AudioInfo audioInfo;
    QString cacheKeyOverride;
};

} // namespace NeriPlayerQt

Q_DECLARE_METATYPE(NeriPlayerQt::AudioInfo)
Q_DECLARE_METATYPE(NeriPlayerQt::SongUrlResult)

#endif // NERIPLAYERQT_SONGURLRESULT_H
