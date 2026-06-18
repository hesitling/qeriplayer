/// @file SongUrlResult.h
/// @brief Song URL resolution result

#ifndef QERIPLAYERQT_SONGURLRESULT_H
#define QERIPLAYERQT_SONGURLRESULT_H

#include <QMetaType>
#include <QString>

#include <cstdint>

namespace QeriPlayerQt {

/**
 * @brief Audio metadata from URL resolution
 *
 * Aligned with Android QeriPlayer's PlaybackAudioInfo.
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
 * Aligned with Android QeriPlayer's SongUrlResult sealed class.
 */
struct SongUrlResult {
    enum class Status : std::uint8_t { Success = 0, WaitingForAuthoritativeStream, RequiresLogin, Failure };

    Status status = Status::Failure;
    QString url;
    qint64 durationMs = 0;
    QString mimeType;
    QString noticeMessage;
    qint64 expectedContentLength = 0;
    AudioInfo audioInfo;
    QString cacheKeyOverride;
};

} // namespace QeriPlayerQt

Q_DECLARE_METATYPE(QeriPlayerQt::AudioInfo)
Q_DECLARE_METATYPE(QeriPlayerQt::SongUrlResult)

#endif // QERIPLAYERQT_SONGURLRESULT_H
