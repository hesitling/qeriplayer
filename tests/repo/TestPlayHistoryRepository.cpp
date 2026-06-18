/// @file TestPlayHistoryRepository.cpp
/// @brief Tests for PlayHistoryRepository

#include "core/database/DatabaseManager.h"
#include "repo/PlayHistoryRepository.h"
#include "repo/SongRepository.h"

#include <QTest>

using namespace QeriPlayerQt;

class TestPlayHistoryRepository : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<DatabaseManager> createDb();
    void insertSong(DatabaseManager *db, const QString &id, const QString &name = "Song");

private Q_SLOTS:
    void record_insertsHistory();
    void record_updatesLastPlayedAt();
    void recent_returnsDistinctSongs();
    void recent_orderedByMostRecent();
    void recent_respectsLimit();
    void clear_deletesAllHistory();
    void remove_specificSongs();
    void playCount_returnsCorrectly();
    void playCount_unplayed_returnsZero();
};

std::unique_ptr<DatabaseManager> TestPlayHistoryRepository::createDb()
{
    auto db = std::make_unique<DatabaseManager>();
    db->open(QString(":memory:"));
    return db;
}

void TestPlayHistoryRepository::insertSong(DatabaseManager *db, const QString &id, const QString &name)
{
    db->exec("INSERT OR REPLACE INTO songs_cache (id, platform, name, artist, album, duration_ms) "
             "VALUES (?, 'NetEase', ?, 'Artist', 'Album', 180000)",
             {id, name});
}

void TestPlayHistoryRepository::record_insertsHistory()
{
    auto db = createDb();
    insertSong(db.get(), "s1");
    PlayHistoryRepository repo(db.get());

    repo.record("s1");

    auto rows = db->exec("SELECT song_id FROM play_history");
    QCOMPARE(rows.size(), 1);
    QCOMPARE(rows[0][0].toString(), QStringLiteral("s1"));
}

void TestPlayHistoryRepository::record_updatesLastPlayedAt()
{
    auto db = createDb();
    insertSong(db.get(), "s1");
    PlayHistoryRepository repo(db.get());

    // Initially null
    auto before = db->exec("SELECT last_played_at FROM songs_cache WHERE id = 's1'");
    QVERIFY(before[0][0].isNull());

    repo.record("s1");

    auto after = db->exec("SELECT last_played_at FROM songs_cache WHERE id = 's1'");
    QVERIFY(!after[0][0].isNull());
}

void TestPlayHistoryRepository::recent_returnsDistinctSongs()
{
    auto db = createDb();
    insertSong(db.get(), "s1", "Song 1");
    insertSong(db.get(), "s2", "Song 2");
    PlayHistoryRepository repo(db.get());

    // Play s1 twice, s2 once
    repo.record("s1");
    repo.record("s2");
    repo.record("s1");

    auto recent = repo.recent(10);
    QCOMPARE(recent.size(), 2);

    // Verify the returned songs are the expected distinct ones
    QStringList ids;
    for (const auto &s : recent) {
        ids << s.id;
    }
    QVERIFY(ids.contains("s1"));
    QVERIFY(ids.contains("s2"));
}

void TestPlayHistoryRepository::recent_orderedByMostRecent()
{
    auto db = createDb();
    insertSong(db.get(), "s1", "Song 1");
    insertSong(db.get(), "s2", "Song 2");
    PlayHistoryRepository repo(db.get());

    // Use explicit timestamps to ensure ordering
    db->exec("INSERT INTO play_history (song_id, played_at) VALUES (?, '2020-01-01 10:00:00')", {"s1"});
    db->exec("INSERT INTO play_history (song_id, played_at) VALUES (?, '2020-01-01 11:00:00')", {"s2"});

    auto recent = repo.recent(10);
    QCOMPARE(recent.size(), 2);
    // Most recent first
    QCOMPARE(recent[0].id, QStringLiteral("s2"));
    QCOMPARE(recent[1].id, QStringLiteral("s1"));
}

void TestPlayHistoryRepository::recent_respectsLimit()
{
    auto db = createDb();
    for (int i = 0; i < 10; ++i) {
        insertSong(db.get(), QString("s%1").arg(i));
    }
    PlayHistoryRepository repo(db.get());

    for (int i = 0; i < 10; ++i) {
        repo.record(QString("s%1").arg(i));
    }

    auto recent = repo.recent(5);
    QCOMPARE(recent.size(), 5);
}

void TestPlayHistoryRepository::clear_deletesAllHistory()
{
    auto db = createDb();
    insertSong(db.get(), "s1");
    PlayHistoryRepository repo(db.get());

    repo.record("s1");
    QCOMPARE(db->exec("SELECT * FROM play_history").size(), 1);

    repo.clear();
    QCOMPARE(db->exec("SELECT * FROM play_history").size(), 0);
}

void TestPlayHistoryRepository::remove_specificSongs()
{
    auto db = createDb();
    insertSong(db.get(), "s1");
    insertSong(db.get(), "s2");
    insertSong(db.get(), "s3");
    PlayHistoryRepository repo(db.get());

    repo.record("s1");
    repo.record("s2");
    repo.record("s3");

    repo.remove({"s1", "s3"});

    auto rows = db->exec("SELECT song_id FROM play_history");
    QCOMPARE(rows.size(), 1);
    QCOMPARE(rows[0][0].toString(), QStringLiteral("s2"));
}

void TestPlayHistoryRepository::playCount_returnsCorrectly()
{
    auto db = createDb();
    insertSong(db.get(), "s1");
    PlayHistoryRepository repo(db.get());

    QCOMPARE(repo.playCount("s1"), 0);

    repo.record("s1");
    repo.record("s1");
    repo.record("s1");

    QCOMPARE(repo.playCount("s1"), 3);
}

void TestPlayHistoryRepository::playCount_unplayed_returnsZero()
{
    auto db = createDb();
    PlayHistoryRepository repo(db.get());

    QCOMPARE(repo.playCount("nonexistent"), 0);
}

QTEST_MAIN(TestPlayHistoryRepository)
#include "TestPlayHistoryRepository.moc"
