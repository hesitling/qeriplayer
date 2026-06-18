/// @file PlayQueue.cpp
/// @brief Ordered song queue with repeat modes and shuffle

#include "player/PlayQueue.h"

#include <algorithm>
#include <random>

namespace QeriPlayerQt {

PlayQueue::PlayQueue(QObject *parent)
    : QObject(parent)
{
}

// --- Song management ---

void PlayQueue::setSongs(const QVector<Song> &songs)
{
    m_songs = songs;
    m_currentIndex = 0;
    rebuildShuffleArray();
    Q_EMIT queueChanged();
    Q_EMIT currentChanged();
}

void PlayQueue::addSong(const Song &song)
{
    m_songs.append(song);
    rebuildShuffleArray();
    Q_EMIT queueChanged();
}

void PlayQueue::removeAt(int index)
{
    if (index < 0 || index >= m_songs.size()) {
        return;
    }

    m_songs.removeAt(index);

    // Adjust current index
    if (m_songs.isEmpty()) {
        m_currentIndex = 0;
    } else if (index < m_currentIndex) {
        --m_currentIndex;
    } else if (index == m_currentIndex) {
        // Current song removed — advance to next (or stay at end)
        if (m_currentIndex >= m_songs.size()) {
            m_currentIndex = m_songs.size() - 1;
        }
    }

    rebuildShuffleArray();
    Q_EMIT queueChanged();
    Q_EMIT currentChanged();
}

void PlayQueue::moveSong(int from, int to)
{
    if (from < 0 || from >= m_songs.size() || to < 0 || to >= m_songs.size() || from == to) {
        return;
    }

    Song song = m_songs.takeAt(from);
    m_songs.insert(to, song);

    // Adjust current index to track the moved song
    if (m_currentIndex == from) {
        m_currentIndex = to;
    } else if (from < m_currentIndex && to >= m_currentIndex) {
        --m_currentIndex;
    } else if (from > m_currentIndex && to <= m_currentIndex) {
        ++m_currentIndex;
    }

    rebuildShuffleArray();
    Q_EMIT queueChanged();
}

void PlayQueue::clear()
{
    m_songs.clear();
    m_currentIndex = 0;
    m_shuffledOrder.clear();
    m_shuffleCursor = 0;
    Q_EMIT queueChanged();
    Q_EMIT currentChanged();
}

// --- Navigation ---

std::optional<Song> PlayQueue::next()
{
    if (m_songs.isEmpty()) {
        return std::nullopt;
    }

    if (m_repeatMode == RepeatMode::One) {
        // Stay on the same song
        Q_EMIT currentChanged();
        return m_songs.at(m_currentIndex);
    }

    int nextIdx;
    if (m_shuffleEnabled && !m_shuffledOrder.isEmpty()) {
        // Walk the shuffled index array
        ++m_shuffleCursor;
        if (m_shuffleCursor >= m_shuffledOrder.size()) {
            if (m_repeatMode == RepeatMode::All) {
                m_shuffleCursor = 0;
            } else {
                --m_shuffleCursor;
                return std::nullopt;
            }
        }
        nextIdx = m_shuffledOrder.at(m_shuffleCursor);
    } else {
        nextIdx = advanceIndex(m_currentIndex);
        if (nextIdx == m_currentIndex && m_repeatMode != RepeatMode::All) {
            // At end, no repeat
            return std::nullopt;
        }
    }

    m_currentIndex = nextIdx;
    Q_EMIT currentChanged();
    return m_songs.at(m_currentIndex);
}

std::optional<Song> PlayQueue::prev()
{
    if (m_songs.isEmpty()) {
        return std::nullopt;
    }

    int prevIdx;
    if (m_shuffleEnabled && !m_shuffledOrder.isEmpty()) {
        --m_shuffleCursor;
        if (m_shuffleCursor < 0) {
            // No wrap-around for prev in shuffle mode
            m_shuffleCursor = 0;
            return std::nullopt;
        }
        prevIdx = m_shuffledOrder.at(m_shuffleCursor);
    } else {
        if (m_currentIndex <= 0) {
            return std::nullopt;
        }
        prevIdx = m_currentIndex - 1;
    }

    m_currentIndex = prevIdx;
    Q_EMIT currentChanged();
    return m_songs.at(m_currentIndex);
}

std::optional<Song> PlayQueue::currentSong() const
{
    if (m_songs.isEmpty() || m_currentIndex < 0 || m_currentIndex >= m_songs.size()) {
        return std::nullopt;
    }
    return m_songs.at(m_currentIndex);
}

int PlayQueue::currentIndex() const
{
    return m_currentIndex;
}

void PlayQueue::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_songs.size()) {
        return;
    }
    m_currentIndex = index;

    // Sync shuffle cursor if shuffle is active
    if (m_shuffleEnabled && !m_shuffledOrder.isEmpty()) {
        for (int i = 0; i < m_shuffledOrder.size(); ++i) {
            if (m_shuffledOrder.at(i) == index) {
                m_shuffleCursor = i;
                break;
            }
        }
    }

    Q_EMIT currentChanged();
}

// --- Repeat mode ---

RepeatMode PlayQueue::repeatMode() const
{
    return m_repeatMode;
}

void PlayQueue::setRepeatMode(RepeatMode mode)
{
    if (m_repeatMode != mode) {
        m_repeatMode = mode;
        Q_EMIT repeatChanged(mode);
    }
}

// --- Shuffle mode ---

bool PlayQueue::isShuffleEnabled() const
{
    return m_shuffleEnabled;
}

void PlayQueue::setShuffleEnabled(bool enabled)
{
    if (m_shuffleEnabled == enabled) {
        return;
    }

    m_shuffleEnabled = enabled;
    if (enabled) {
        rebuildShuffleArray();
        // Find the current song in the shuffled order and set cursor
        for (int i = 0; i < m_shuffledOrder.size(); ++i) {
            if (m_shuffledOrder.at(i) == m_currentIndex) {
                m_shuffleCursor = i;
                break;
            }
        }
    } else {
        m_shuffledOrder.clear();
        m_shuffleCursor = 0;
    }

    Q_EMIT shuffleChanged(enabled);
}

// --- Accessors ---

const QVector<Song> &PlayQueue::songs() const
{
    return m_songs;
}

int PlayQueue::size() const
{
    return m_songs.size();
}

bool PlayQueue::isEmpty() const
{
    return m_songs.isEmpty();
}

// --- Persistence ---

PersistedPlayerState PlayQueue::toPersistedState() const
{
    PersistedPlayerState state;
    state.playlist = m_songs;
    state.currentIndex = m_currentIndex;
    state.repeatMode = m_repeatMode;
    state.shuffleEnabled = m_shuffleEnabled;
    return state;
}

void PlayQueue::loadFromState(const PersistedPlayerState &state)
{
    m_songs = state.playlist;
    m_currentIndex = state.currentIndex;
    m_repeatMode = state.repeatMode;
    m_shuffleEnabled = state.shuffleEnabled;

    if (m_currentIndex < 0 || m_currentIndex >= m_songs.size()) {
        m_currentIndex = 0;
    }

    if (m_shuffleEnabled) {
        rebuildShuffleArray();
        // Find current song in shuffle order
        for (int i = 0; i < m_shuffledOrder.size(); ++i) {
            if (m_shuffledOrder.at(i) == m_currentIndex) {
                m_shuffleCursor = i;
                break;
            }
        }
    }

    Q_EMIT queueChanged();
    Q_EMIT currentChanged();
    Q_EMIT repeatChanged(m_repeatMode);
    Q_EMIT shuffleChanged(m_shuffleEnabled);
}

// --- Private ---

void PlayQueue::rebuildShuffleArray()
{
    m_shuffledOrder.resize(m_songs.size());
    for (int i = 0; i < m_songs.size(); ++i) {
        m_shuffledOrder[i] = i;
    }

    if (m_songs.size() > 1) {
        // Fisher-Yates shuffle
        std::random_device rd;
        std::mt19937 gen(rd());
        for (int i = m_shuffledOrder.size() - 1; i > 0; --i) {
            std::uniform_int_distribution<int> dist(0, i);
            int j = dist(gen);
            std::swap(m_shuffledOrder[i], m_shuffledOrder[j]);
        }
    }

    // Keep shuffle cursor aligned with current index if possible
    int newCursor = 0;
    if (m_shuffleEnabled && m_currentIndex >= 0 && m_currentIndex < m_songs.size()) {
        for (int i = 0; i < m_shuffledOrder.size(); ++i) {
            if (m_shuffledOrder[i] == m_currentIndex) {
                newCursor = i;
                break;
            }
        }
    }
    m_shuffleCursor = newCursor;
}

int PlayQueue::advanceIndex(int index) const
{
    if (index < m_songs.size() - 1) {
        return index + 1;
    }
    // At end — if repeat all, wrap to 0; otherwise stay
    if (m_repeatMode == RepeatMode::All) {
        return 0;
    }
    return index;
}

} // namespace QeriPlayerQt
