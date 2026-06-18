/// @file TestPlayQueue.cpp
/// @brief Unit tests for PlayQueue

#include "player/PlayQueue.h"

#include <QSignalSpy>
#include <QTest>

using namespace QeriPlayerQt;

class TestPlayQueue : public QObject {
    Q_OBJECT

private:
    static Song makeSong(const QString &id, const QString &name = {})
    {
        Song s;
        s.id = id;
        s.name = name.isEmpty() ? id : name;
        s.artist = QStringLiteral("Artist");
        s.album = QStringLiteral("Album");
        return s;
    }

    static QVector<Song> makeSongs(int count)
    {
        QVector<Song> songs;
        songs.reserve(count);
        for (int i = 0; i < count; ++i) {
            songs.append(makeSong(QString::number(i), QStringLiteral("Song %1").arg(i)));
        }
        return songs;
    }

private Q_SLOTS:
    void init()
    {
        // Each test gets a fresh queue
    }

    // --- Empty queue ---

    void emptyQueue_currentSong_returnsNullopt()
    {
        PlayQueue q;
        QVERIFY(!q.currentSong().has_value());
        QCOMPARE(q.currentIndex(), 0);
        QVERIFY(q.isEmpty());
        QCOMPARE(q.size(), 0);
    }

    void emptyQueue_next_returnsNullopt()
    {
        PlayQueue q;
        QVERIFY(!q.next().has_value());
    }

    void emptyQueue_prev_returnsNullopt()
    {
        PlayQueue q;
        QVERIFY(!q.prev().has_value());
    }

    // --- Single song ---

    void singleSong_currentSong_returnsSong()
    {
        PlayQueue q;
        q.setSongs({makeSong("A")});
        QCOMPARE(q.size(), 1);
        QCOMPARE(q.currentSong()->id, QStringLiteral("A"));
        QCOMPARE(q.currentIndex(), 0);
    }

    void singleSong_nextWithRepeatOff_returnsNullopt()
    {
        PlayQueue q;
        q.setSongs({makeSong("A")});
        q.setRepeatMode(RepeatMode::Off);
        QVERIFY(!q.next().has_value());
        QCOMPARE(q.currentIndex(), 0);
    }

    void singleSong_nextWithRepeatAll_returnsSameSong()
    {
        PlayQueue q;
        q.setSongs({makeSong("A")});
        q.setRepeatMode(RepeatMode::All);
        auto next = q.next();
        QVERIFY(next.has_value());
        QCOMPARE(next->id, QStringLiteral("A"));
        QCOMPARE(q.currentIndex(), 0);
    }

    void singleSong_nextWithRepeatOne_returnsSameSong()
    {
        PlayQueue q;
        q.setSongs({makeSong("A")});
        q.setRepeatMode(RepeatMode::One);
        auto next = q.next();
        QVERIFY(next.has_value());
        QCOMPARE(next->id, QStringLiteral("A"));
    }

    void singleSong_prev_returnsNullopt()
    {
        PlayQueue q;
        q.setSongs({makeSong("A")});
        QVERIFY(!q.prev().has_value());
    }

    // --- Multi-song navigation ---

    void multiSong_nextAdvancesIndex()
    {
        PlayQueue q;
        q.setSongs(makeSongs(5));
        QCOMPARE(q.currentIndex(), 0);

        auto next = q.next();
        QVERIFY(next.has_value());
        QCOMPARE(next->id, QStringLiteral("1"));
        QCOMPARE(q.currentIndex(), 1);
    }

    void multiSong_prevMovesBack()
    {
        PlayQueue q;
        q.setSongs(makeSongs(5));
        q.setCurrentIndex(3);
        QCOMPARE(q.currentIndex(), 3);

        auto prev = q.prev();
        QVERIFY(prev.has_value());
        QCOMPARE(prev->id, QStringLiteral("2"));
        QCOMPARE(q.currentIndex(), 2);
    }

    void multiSong_prevAtStart_returnsNullopt()
    {
        PlayQueue q;
        q.setSongs(makeSongs(5));
        QVERIFY(!q.prev().has_value());
        QCOMPARE(q.currentIndex(), 0);
    }

    void multiSong_nextAtEndWithRepeatOff_returnsNullopt()
    {
        PlayQueue q;
        q.setSongs(makeSongs(3));
        q.setCurrentIndex(2);
        q.setRepeatMode(RepeatMode::Off);
        QVERIFY(!q.next().has_value());
        QCOMPARE(q.currentIndex(), 2);
    }

    void multiSong_nextAtEndWithRepeatAll_wraps()
    {
        PlayQueue q;
        q.setSongs(makeSongs(3));
        q.setCurrentIndex(2);
        q.setRepeatMode(RepeatMode::All);
        auto next = q.next();
        QVERIFY(next.has_value());
        QCOMPARE(next->id, QStringLiteral("0"));
        QCOMPARE(q.currentIndex(), 0);
    }

    // --- Repeat modes ---

    void repeatOne_staysOnCurrentSong()
    {
        PlayQueue q;
        q.setSongs(makeSongs(3));
        q.setCurrentIndex(1);
        q.setRepeatMode(RepeatMode::One);

        auto next1 = q.next();
        QVERIFY(next1.has_value());
        QCOMPARE(next1->id, QStringLiteral("1"));

        auto next2 = q.next();
        QVERIFY(next2.has_value());
        QCOMPARE(next2->id, QStringLiteral("1"));
    }

    void repeatAll_wrapsAround()
    {
        PlayQueue q;
        q.setSongs(makeSongs(3));
        q.setRepeatMode(RepeatMode::All);

        q.next();             // 0 -> 1
        q.next();             // 1 -> 2
        auto next = q.next(); // 2 -> 0
        QVERIFY(next.has_value());
        QCOMPARE(next->id, QStringLiteral("0"));
    }

    // --- Shuffle ---

    void shuffle_enableChangesOrder()
    {
        PlayQueue q;
        q.setSongs(makeSongs(10));
        q.setRepeatMode(RepeatMode::All);
        q.setShuffleEnabled(true);
        QVERIFY(q.isShuffleEnabled());

        // Collect all songs: current + 9 next() calls
        QStringList ids;
        auto current = q.currentSong();
        QVERIFY(current.has_value());
        ids.append(current->id);

        for (int i = 0; i < 9; ++i) {
            auto song = q.next();
            QVERIFY(song.has_value());
            ids.append(song->id);
        }

        // All 10 songs should be retrieved
        QCOMPARE(ids.size(), 10);

        // All IDs should be unique (no duplicates)
        QCOMPARE(QSet<QString>(ids.begin(), ids.end()).size(), 10);

        // Order should not be strictly sequential (shuffle changed it)
        QStringList sequential;
        for (int i = 0; i < 10; ++i) {
            sequential.append(QString::number(i));
        }
        QVERIFY(ids != sequential);
    }

    void shuffle_disableRestoresOrder()
    {
        PlayQueue q;
        q.setSongs(makeSongs(5));
        q.setShuffleEnabled(true);
        q.setShuffleEnabled(false);

        // After disabling, order should be sequential from current
        // But the underlying song list is unchanged
        const auto &songs = q.songs();
        QCOMPARE(songs.size(), 5);
        QCOMPARE(songs.at(0).id, QStringLiteral("0"));
        QCOMPARE(songs.at(1).id, QStringLiteral("1"));
    }

    void shuffle_preservesOriginalOrder()
    {
        PlayQueue q;
        auto songs = makeSongs(5);
        q.setSongs(songs);
        q.setShuffleEnabled(true);

        // Navigate a few times
        q.next();
        q.next();

        q.setShuffleEnabled(false);

        // Original order preserved
        for (int i = 0; i < 5; ++i) {
            QCOMPARE(q.songs().at(i).id, QString::number(i));
        }
    }

    // --- Song management ---

    void setSongs_resetsCurrentIndex()
    {
        PlayQueue q;
        q.setSongs(makeSongs(5));
        q.setCurrentIndex(3);
        q.setSongs(makeSongs(3));
        QCOMPARE(q.currentIndex(), 0);
    }

    void addSong_appendsToQueue()
    {
        PlayQueue q;
        q.setSongs(makeSongs(3));
        q.addSong(makeSong("new", "New Song"));
        QCOMPARE(q.size(), 4);
        QCOMPARE(q.songs().at(3).id, QStringLiteral("new"));
    }

    void removeAt_removesSong()
    {
        PlayQueue q;
        q.setSongs(makeSongs(5));
        q.removeAt(2);
        QCOMPARE(q.size(), 4);
        QCOMPARE(q.songs().at(2).id, QStringLiteral("3"));
    }

    void removeAt_currentSong_advancesToNext()
    {
        PlayQueue q;
        q.setSongs(makeSongs(5));
        q.setCurrentIndex(2);
        q.removeAt(2);
        // Current should advance to what was index 3 (now index 2)
        QCOMPARE(q.currentSong()->id, QStringLiteral("3"));
    }

    void removeAt_lastCurrentSong_staysAtEnd()
    {
        PlayQueue q;
        q.setSongs(makeSongs(3));
        q.setCurrentIndex(2);
        q.removeAt(2);
        QCOMPARE(q.currentIndex(), 1);
        QCOMPARE(q.currentSong()->id, QStringLiteral("1"));
    }

    void removeAt_beforeCurrent_adjustsIndex()
    {
        PlayQueue q;
        q.setSongs(makeSongs(5));
        q.setCurrentIndex(3);
        q.removeAt(1);
        QCOMPARE(q.currentIndex(), 2);
        QCOMPARE(q.currentSong()->id, QStringLiteral("3"));
    }

    void clear_emptiesQueue()
    {
        PlayQueue q;
        q.setSongs(makeSongs(5));
        q.clear();
        QVERIFY(q.isEmpty());
        QCOMPARE(q.size(), 0);
        QVERIFY(!q.currentSong().has_value());
    }

    void moveSong_reorders()
    {
        PlayQueue q;
        q.setSongs(makeSongs(5));
        q.moveSong(0, 3);
        QCOMPARE(q.songs().at(0).id, QStringLiteral("1"));
        QCOMPARE(q.songs().at(3).id, QStringLiteral("0"));
    }

    // --- Jump to index ---

    void setCurrentIndex_jumpsToPosition()
    {
        PlayQueue q;
        q.setSongs(makeSongs(5));
        q.setCurrentIndex(4);
        QCOMPARE(q.currentIndex(), 4);
        QCOMPARE(q.currentSong()->id, QStringLiteral("4"));
    }

    void setCurrentIndex_outOfBounds_ignored()
    {
        PlayQueue q;
        q.setSongs(makeSongs(5));
        q.setCurrentIndex(10);
        QCOMPARE(q.currentIndex(), 0);

        q.setCurrentIndex(-1);
        QCOMPARE(q.currentIndex(), 0);
    }

    // --- Signals ---

    void setSongs_emitsQueueChangedAndCurrentChanged()
    {
        PlayQueue q;
        QSignalSpy queueSpy(&q, &PlayQueue::queueChanged);
        QSignalSpy currentSpy(&q, &PlayQueue::currentChanged);
        q.setSongs(makeSongs(3));
        QCOMPARE(queueSpy.count(), 1);
        QCOMPARE(currentSpy.count(), 1);
    }

    void addSong_emitsQueueChanged()
    {
        PlayQueue q;
        q.setSongs(makeSongs(3));
        QSignalSpy spy(&q, &PlayQueue::queueChanged);
        q.addSong(makeSong("new"));
        QCOMPARE(spy.count(), 1);
    }

    void next_emitsCurrentChanged()
    {
        PlayQueue q;
        q.setSongs(makeSongs(3));
        QSignalSpy spy(&q, &PlayQueue::currentChanged);
        q.next();
        QCOMPARE(spy.count(), 1);
    }

    void setRepeatMode_emitsRepeatChanged()
    {
        PlayQueue q;
        QSignalSpy spy(&q, &PlayQueue::repeatChanged);
        q.setRepeatMode(RepeatMode::All);
        QCOMPARE(spy.count(), 1);
    }

    void setShuffleEnabled_emitsShuffleChanged()
    {
        PlayQueue q;
        QSignalSpy spy(&q, &PlayQueue::shuffleChanged);
        q.setShuffleEnabled(true);
        QCOMPARE(spy.count(), 1);
    }

    // --- Persistence ---

    void toPersistedState_savesState()
    {
        PlayQueue q;
        q.setSongs(makeSongs(5));
        q.setCurrentIndex(2);
        q.setRepeatMode(RepeatMode::All);
        q.setShuffleEnabled(true);

        auto state = q.toPersistedState();
        QCOMPARE(state.playlist.size(), 5);
        QCOMPARE(state.currentIndex, 2);
        QCOMPARE(state.repeatMode, RepeatMode::All);
        QCOMPARE(state.shuffleEnabled, true);
    }

    void loadFromState_restoresState()
    {
        PersistedPlayerState state;
        state.playlist = makeSongs(3);
        state.currentIndex = 1;
        state.repeatMode = RepeatMode::One;
        state.shuffleEnabled = false;

        PlayQueue q;
        q.loadFromState(state);

        QCOMPARE(q.size(), 3);
        QCOMPARE(q.currentIndex(), 1);
        QCOMPARE(q.repeatMode(), RepeatMode::One);
        QCOMPARE(q.isShuffleEnabled(), false);
        QCOMPARE(q.currentSong()->id, QStringLiteral("1"));
    }
};

QTEST_MAIN(TestPlayQueue)
#include "TestPlayQueue.moc"
