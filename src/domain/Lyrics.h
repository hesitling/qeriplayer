/// @file Lyrics.h
/// @brief Lyrics domain model with word-level timing support

#ifndef QERIPLAYERQT_LYRICS_H
#define QERIPLAYERQT_LYRICS_H

#include <QMetaType>
#include <QString>
#include <QVector>

namespace QeriPlayerQt {

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

} // namespace QeriPlayerQt

Q_DECLARE_METATYPE(QeriPlayerQt::LyricWord)
Q_DECLARE_METATYPE(QeriPlayerQt::LyricLine)
Q_DECLARE_METATYPE(QeriPlayerQt::Lyrics)

#endif // QERIPLAYERQT_LYRICS_H
