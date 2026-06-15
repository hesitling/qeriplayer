/// @file Lyrics.h
/// @brief Lyrics domain model with word-level timing support

#ifndef NERIPLAYERQT_LYRICS_H
#define NERIPLAYERQT_LYRICS_H

#include <QMetaType>
#include <QString>
#include <QVector>

namespace NeriPlayerQt {

/**
 * @brief Word-level timing for synchronized lyrics display
 */
struct LyricWord {
    QString text;
    qint64 startTimeMs = 0;
    qint64 endTimeMs = 0;
};

/**
 * @brief A single timed lyric line with optional word-level timing
 */
struct LyricLine {
    qint64 startTimeMs = 0; ///< Line start in milliseconds
    qint64 endTimeMs = 0;   ///< Line end in milliseconds
    QString text;
    QVector<LyricWord> words; ///< Word-level timing (optional, e.g. YRC format)
};

/**
 * @brief Full lyrics for a song
 */
struct Lyrics {
    QString rawText;
    QVector<LyricLine> lines; ///< Sorted by startTimeMs ascending
};

} // namespace NeriPlayerQt

Q_DECLARE_METATYPE(NeriPlayerQt::LyricWord)
Q_DECLARE_METATYPE(NeriPlayerQt::LyricLine)
Q_DECLARE_METATYPE(NeriPlayerQt::Lyrics)

#endif // NERIPLAYERQT_LYRICS_H
