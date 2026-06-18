/// @file TestSongRepository.cpp
/// @brief Tests for SongRepository

#include "core/database/DatabaseManager.h"
#include "repo/SongRepository.h"
#include "repo/SqlRowMapper.h"

#include <QTest>

using namespace QeriPlayerQt;

class TestSongRepository : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<DatabaseManager> createDb();

private Q_SLOTS:
    void save_andFindById();
    void findById_notFound_returnsEmpty();
    void findByIds_partialMatch();
    void saveBatch_insertsAll();
    void remove_deletesSong();
    void exists_returnsCorrectly();
    void findByPlatform_filtersCorrectly();
    void search_byName();
    void search_byArtist();
    void search_byAlbum();
    void search_respectsLimit();
    void save_updatesExistingSong();
};

std::unique_ptr<DatabaseManager> TestSongRepository::createDb()
{
    auto db = std::make_unique<DatabaseManager>();
    db->open(QString(":memory:"));
    return db;
}

static Song makeSong(const QString &id, const QString &name = "Song", const QString &artist = "Artist",
                     const QString &album = "Album", MusicPlatform platform = MusicPlatform::NetEase)
{
    Song s;
    s.id = id;
    s.name = name;
    s.artist = artist;
    s.album = album;
    s.platform = platform;
    s.durationMs = 180000;
    return s;
}

void TestSongRepository::save_andFindById()
{
    auto db = createDb();
    SongRepository repo(db.get());

    Song original = makeSong("s1", "My Song", "My Artist");
    repo.save(original);

    auto found = repo.findById("s1");
    QVERIFY(found.has_value());
    QCOMPARE(found->id, QStringLiteral("s1"));
    QCOMPARE(found->name, QStringLiteral("My Song"));
    QCOMPARE(found->artist, QStringLiteral("My Artist"));
}

void TestSongRepository::findById_notFound_returnsEmpty()
{
    auto db = createDb();
    SongRepository repo(db.get());

    auto found = repo.findById("nonexistent");
    QVERIFY(!found.has_value());
}

void TestSongRepository::findByIds_partialMatch()
{
    auto db = createDb();
    SongRepository repo(db.get());

    repo.save(makeSong("s1"));
    repo.save(makeSong("s2"));

    auto results = repo.findByIds({"s1", "s2", "s3"});
    QCOMPARE(results.size(), 2);
}

void TestSongRepository::saveBatch_insertsAll()
{
    auto db = createDb();
    SongRepository repo(db.get());

    QVector<Song> songs;
    for (int i = 0; i < 100; ++i) {
        songs.append(makeSong(QString("batch_%1").arg(i), QString("Song %1").arg(i)));
    }
    repo.saveBatch(songs);

    auto results = repo.findByIds(QStringList {"batch_0", "batch_50", "batch_99"});
    QCOMPARE(results.size(), 3);
}

void TestSongRepository::remove_deletesSong()
{
    auto db = createDb();
    SongRepository repo(db.get());

    repo.save(makeSong("s1"));
    QVERIFY(repo.exists("s1"));

    repo.remove("s1");
    QVERIFY(!repo.exists("s1"));
}

void TestSongRepository::exists_returnsCorrectly()
{
    auto db = createDb();
    SongRepository repo(db.get());

    QVERIFY(!repo.exists("s1"));
    repo.save(makeSong("s1"));
    QVERIFY(repo.exists("s1"));
}

void TestSongRepository::findByPlatform_filtersCorrectly()
{
    auto db = createDb();
    SongRepository repo(db.get());

    repo.save(makeSong("ne1", "NE Song", "A", "Alb", MusicPlatform::NetEase));
    repo.save(makeSong("yt1", "YT Song", "A", "Alb", MusicPlatform::YouTube));
    repo.save(makeSong("ne2", "NE Song 2", "A", "Alb", MusicPlatform::NetEase));

    auto netease = repo.findByPlatform(MusicPlatform::NetEase);
    QCOMPARE(netease.size(), 2);

    auto youtube = repo.findByPlatform(MusicPlatform::YouTube);
    QCOMPARE(youtube.size(), 1);
}

void TestSongRepository::search_byName()
{
    auto db = createDb();
    SongRepository repo(db.get());

    repo.save(makeSong("s1", "Beatles Song", "Some Artist"));
    repo.save(makeSong("s2", "Another Song", "Beatles"));
    repo.save(makeSong("s3", "Unrelated", "Other"));

    auto results = repo.search("Beatles");
    QCOMPARE(results.size(), 2);
}

void TestSongRepository::search_byArtist()
{
    auto db = createDb();
    SongRepository repo(db.get());

    repo.save(makeSong("s1", "Song A", "John Lennon"));
    repo.save(makeSong("s2", "Song B", "Paul McCartney"));
    repo.save(makeSong("s3", "Song C", "Other"));

    auto results = repo.search("Lennon");
    QCOMPARE(results.size(), 1);
}

void TestSongRepository::search_byAlbum()
{
    auto db = createDb();
    SongRepository repo(db.get());

    repo.save(makeSong("s1", "Song", "Artist", "Abbey Road"));
    repo.save(makeSong("s2", "Song2", "Artist2", "Other Album"));

    auto results = repo.search("Abbey");
    QCOMPARE(results.size(), 1);
}

void TestSongRepository::search_respectsLimit()
{
    auto db = createDb();
    SongRepository repo(db.get());

    for (int i = 0; i < 50; ++i) {
        repo.save(makeSong(QString("s%1").arg(i), "Match Song"));
    }

    auto results = repo.search("Match", 10);
    QCOMPARE(results.size(), 10);
}

void TestSongRepository::save_updatesExistingSong()
{
    auto db = createDb();
    SongRepository repo(db.get());

    Song song = makeSong("s1", "Original Name");
    repo.save(song);

    song.name = "Updated Name";
    repo.save(song);

    auto found = repo.findById("s1");
    QVERIFY(found.has_value());
    QCOMPARE(found->name, QStringLiteral("Updated Name"));
}

QTEST_MAIN(TestSongRepository)
#include "TestSongRepository.moc"
