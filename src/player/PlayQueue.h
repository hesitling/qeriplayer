/// @file PlayQueue.h
/// @brief Ordered song queue with repeat modes and shuffle

#ifndef QERIPLAYERQT_PLAYQUEUE_H
#define QERIPLAYERQT_PLAYQUEUE_H

#include "domain/Enums.h"
#include "domain/PersistedPlayerState.h"
#include "domain/Song.h"

#include <QObject>
#include <QVector>

#include <optional>

namespace QeriPlayerQt {

/**
 * @brief Ordered song queue with repeat modes, shuffle, and navigation
 *
 * Maintains an ordered list of songs with support for repeat modes
 * (off, one, all) and shuffle via a Fisher-Yates index permutation.
 * The original song order is always preserved.
 */
class PlayQueue : public QObject {
    Q_OBJECT

public:
    explicit PlayQueue(QObject *parent = nullptr);
    ~PlayQueue() override = default;

    // --- Song management ---

    /// @brief Replace all songs in the queue
    void setSongs(const QVector<Song> &songs);

    /// @brief Append a song to the end of the queue
    void addSong(const Song &song);

    /// @brief Remove the song at the given index
    void removeAt(int index);

    /// @brief Move a song from one position to another
    void moveSong(int from, int to);

    /// @brief Clear the queue
    void clear();

    // --- Navigation ---

    /// @brief Advance to the next song (respects repeat/shuffle)
    [[nodiscard]] std::optional<Song> next();

    /// @brief Move to the previous song
    [[nodiscard]] std::optional<Song> prev();

    /// @brief Current song, or nullopt if queue is empty
    [[nodiscard]] std::optional<Song> currentSong() const;

    /// @brief Current index in the song list
    [[nodiscard]] int currentIndex() const;

    /// @brief Jump to a specific index
    void setCurrentIndex(int index);

    // --- Repeat mode ---

    [[nodiscard]] RepeatMode repeatMode() const;
    void setRepeatMode(RepeatMode mode);

    // --- Shuffle mode ---

    [[nodiscard]] bool isShuffleEnabled() const;
    void setShuffleEnabled(bool enabled);

    // --- Accessors ---

    [[nodiscard]] const QVector<Song> &songs() const;
    [[nodiscard]] int size() const;
    [[nodiscard]] bool isEmpty() const;

    // --- Persistence ---

    [[nodiscard]] PersistedPlayerState toPersistedState() const;
    void loadFromState(const PersistedPlayerState &state);

Q_SIGNALS:
    void currentChanged();
    void queueChanged();
    void shuffleChanged(bool enabled);
    void repeatChanged(QeriPlayerQt::RepeatMode mode);

private:
    void rebuildShuffleArray();
    int advanceIndex(int index) const;

    QVector<Song> m_songs;
    int m_currentIndex = 0;

    RepeatMode m_repeatMode = RepeatMode::Off;
    bool m_shuffleEnabled = false;

    /// Shuffle index permutation: m_shuffledOrder[i] is the i-th song index in shuffled order
    QVector<int> m_shuffledOrder;
    int m_shuffleCursor = 0;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_PLAYQUEUE_H
