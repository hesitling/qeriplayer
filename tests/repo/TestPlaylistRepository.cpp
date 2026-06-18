/// @file TestPlaylistRepository.cpp
/// @brief Tests for PlaylistRepository

#include "core/database/DatabaseManager.h"
#include "repo/PlaylistRepository.h"
#include "repo/SongRepository.h"

#include <QTest>

using namespace QeriPlayerQt;

class TestPlaylistRepository : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<DatabaseManager> createDb();
    void insertSong(DatabaseManager *db, const QString &id, const QString &name = "Song");

private Q_SLOTS:
    void create_makesPlaylist();
    void findAll_returnsAll();
    void findById_withSongs();
    void findById_notFound_returnsEmpty();
    void updateMetadata_updatesFields();
    void remove_deletesPlaylist();
    void addSong_appendsSong();
    void addSong_atSpecificPosition();
    void addSong_duplicate_noOp();
    void removeSong_removesAndAdjustsPositions();
    void reorderSongs_reorders();
    void songCount_returnsCorrectly();
};

std::unique_ptr<DatabaseManager> TestPlaylistRepository::createDb()
{
    auto db = std::make_unique<DatabaseManager>();
    db->open(QString(":memory:"));
    return db;
}

void TestPlaylistRepository::insertSong(DatabaseManager *db, const QString &id, const QString &name)
{
    db->exec("INSERT OR REPLACE INTO songs_cache (id, platform, name, artist, album, duration_ms) "
             "VALUES (?, 'NetEase', ?, 'Artist', 'Album', 180000)",
             {id, name});
}

void TestPlaylistRepository::create_makesPlaylist()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    auto pl = repo.create("My Playlist");
    QVERIFY(!pl.id.isEmpty());
    QCOMPARE(pl.name, QStringLiteral("My Playlist"));
    QCOMPARE(pl.songCount, 0);

    // Verify in DB
    auto rows = db->exec("SELECT name, platform FROM playlists WHERE id = ?", {pl.id});
    QCOMPARE(rows.size(), 1);
    QCOMPARE(rows[0][0].toString(), pl.name);
    QCOMPARE(rows[0][1].toString(), QStringLiteral("Unknown"));
}

void TestPlaylistRepository::findAll_returnsAll()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    repo.create("PL1");
    repo.create("PL2");
    repo.create("PL3");

    auto all = repo.findAll();
    QCOMPARE(all.size(), 3);
}

void TestPlaylistRepository::findById_withSongs()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    SongRepository songRepo(db.get());

    songRepo.save(Song {.id = "s1", .name = "Song 1", .durationMs = 120000});
    songRepo.save(Song {.id = "s2", .name = "Song 2", .durationMs = 180000});

    auto pl = repo.create("Test PL");
    repo.addSong(pl.id, "s1");
    repo.addSong(pl.id, "s2");

    auto found = repo.findById(pl.id);
    QVERIFY(found.has_value());
    QCOMPARE(found->songs.size(), 2);
    QCOMPARE(found->songs[0].id, QStringLiteral("s1"));
    QCOMPARE(found->songs[1].id, QStringLiteral("s2"));
}

void TestPlaylistRepository::findById_notFound_returnsEmpty()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    auto found = repo.findById("nonexistent");
    QVERIFY(!found.has_value());
}

void TestPlaylistRepository::updateMetadata_updatesFields()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    auto pl = repo.create("Original");
    repo.updateMetadata(pl.id, "New Name", "A description", "https://cover.url");

    auto rows = db->exec("SELECT name, description, cover_url FROM playlists WHERE id = ?", {pl.id});
    QCOMPARE(rows[0][0].toString(), QStringLiteral("New Name"));
    QCOMPARE(rows[0][1].toString(), QStringLiteral("A description"));
    QCOMPARE(rows[0][2].toString(), QStringLiteral("https://cover.url"));
}

void TestPlaylistRepository::remove_deletesPlaylist()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    auto pl = repo.create("To Delete");
    repo.remove(pl.id);

    auto rows = db->exec("SELECT id FROM playlists WHERE id = ?", {pl.id});
    QCOMPARE(rows.size(), 0);
}

void TestPlaylistRepository::addSong_appendsSong()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    SongRepository songRepo(db.get());

    songRepo.save(Song {.id = "s1", .name = "Song", .durationMs = 120000});

    auto pl = repo.create("Test");
    bool added = repo.addSong(pl.id, "s1");
    QVERIFY(added);

    QCOMPARE(repo.songCount(pl.id), 1);
}

void TestPlaylistRepository::addSong_atSpecificPosition()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    SongRepository songRepo(db.get());

    songRepo.save(Song {.id = "s1", .name = "Song 1", .durationMs = 120000});
    songRepo.save(Song {.id = "s2", .name = "Song 2", .durationMs = 120000});
    songRepo.save(Song {.id = "s3", .name = "Song 3", .durationMs = 120000});

    auto pl = repo.create("Test");
    repo.addSong(pl.id, "s1");
    repo.addSong(pl.id, "s3");
    repo.addSong(pl.id, "s2", 1); // Insert at position 1

    auto found = repo.findById(pl.id);
    QVERIFY(found.has_value());
    QCOMPARE(found->songs.size(), 3);
    QCOMPARE(found->songs[0].id, QStringLiteral("s1"));
    QCOMPARE(found->songs[1].id, QStringLiteral("s2"));
    QCOMPARE(found->songs[2].id, QStringLiteral("s3"));
}

void TestPlaylistRepository::addSong_duplicate_noOp()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    SongRepository songRepo(db.get());

    songRepo.save(Song {.id = "s1", .name = "Song", .durationMs = 120000});

    auto pl = repo.create("Test");
    repo.addSong(pl.id, "s1");
    bool added = repo.addSong(pl.id, "s1");
    QVERIFY(!added);
    QCOMPARE(repo.songCount(pl.id), 1);
}

void TestPlaylistRepository::removeSong_removesAndAdjustsPositions()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    SongRepository songRepo(db.get());

    songRepo.save(Song {.id = "s1", .name = "Song 1", .durationMs = 120000});
    songRepo.save(Song {.id = "s2", .name = "Song 2", .durationMs = 120000});
    songRepo.save(Song {.id = "s3", .name = "Song 3", .durationMs = 120000});

    auto pl = repo.create("Test");
    repo.addSong(pl.id, "s1");
    repo.addSong(pl.id, "s2");
    repo.addSong(pl.id, "s3");

    repo.removeSong(pl.id, "s2");

    auto found = repo.findById(pl.id);
    QVERIFY(found.has_value());
    QCOMPARE(found->songs.size(), 2);
    QCOMPARE(found->songs[0].id, QStringLiteral("s1"));
    QCOMPARE(found->songs[1].id, QStringLiteral("s3"));
}

void TestPlaylistRepository::reorderSongs_reorders()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    SongRepository songRepo(db.get());

    songRepo.save(Song {.id = "s1", .name = "Song 1", .durationMs = 120000});
    songRepo.save(Song {.id = "s2", .name = "Song 2", .durationMs = 120000});
    songRepo.save(Song {.id = "s3", .name = "Song 3", .durationMs = 120000});

    auto pl = repo.create("Test");
    repo.addSong(pl.id, "s1");
    repo.addSong(pl.id, "s2");
    repo.addSong(pl.id, "s3");

    repo.reorderSongs(pl.id, {"s3", "s1", "s2"});

    auto found = repo.findById(pl.id);
    QVERIFY(found.has_value());
    QCOMPARE(found->songs[0].id, QStringLiteral("s3"));
    QCOMPARE(found->songs[1].id, QStringLiteral("s1"));
    QCOMPARE(found->songs[2].id, QStringLiteral("s2"));
}

void TestPlaylistRepository::songCount_returnsCorrectly()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    SongRepository songRepo(db.get());

    songRepo.save(Song {.id = "s1", .name = "Song 1", .durationMs = 120000});
    songRepo.save(Song {.id = "s2", .name = "Song 2", .durationMs = 120000});

    auto pl = repo.create("Test");
    QCOMPARE(repo.songCount(pl.id), 0);

    repo.addSong(pl.id, "s1");
    QCOMPARE(repo.songCount(pl.id), 1);

    repo.addSong(pl.id, "s2");
    QCOMPARE(repo.songCount(pl.id), 2);
}

QTEST_MAIN(TestPlaylistRepository)
#include "TestPlaylistRepository.moc"
