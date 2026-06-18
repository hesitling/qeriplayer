/// @file TestLocalPlaylistDetailViewModel.cpp
/// @brief Unit tests for LocalPlaylistDetailViewModel (in-memory SQLite)

#include "core/database/DatabaseManager.h"
#include "domain/Playlist.h"
#include "domain/Song.h"
#include "repo/PlaylistRepository.h"
#include "repo/SongRepository.h"
#include "viewmodel/LocalPlaylistDetailViewModel.h"

#include <QSignalSpy>
#include <QTest>

#include <memory>

using namespace QeriPlayerQt;

class TestLocalPlaylistDetailViewModel : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<DatabaseManager> createDb()
    {
        auto db = std::make_unique<DatabaseManager>();
        db->open(QString(":memory:"));
        return db;
    }

    void insertSongRow(DatabaseManager *db, const QString &id, const QString &name = "Song")
    {
        db->exec("INSERT OR REPLACE INTO songs_cache (id, platform, name, artist, album, duration_ms) "
                 "VALUES (?, 'NetEase', ?, 'Artist', 'Album', 180000)",
                 {id, name});
    }

private Q_SLOTS:
    void loadPlaylist_found();
    void loadPlaylist_notFound();
    void deletePlaylist();
    void rename();
    void playSong();
    void playAll();
    void addSong_repoException_setsError();
    void removeSong_repoException_setsError();
    void reorderSongs_repoException_setsError();
};

void TestLocalPlaylistDetailViewModel::loadPlaylist_found()
{
    auto db = createDb();
    PlaylistRepository playlistRepo(db.get());
    SongRepository songRepo(db.get());

    auto pl = playlistRepo.create("My Playlist");
    insertSongRow(db.get(), "s1", "Song A");
    insertSongRow(db.get(), "s2", "Song B");
    playlistRepo.addSong(pl.id, "s1");
    playlistRepo.addSong(pl.id, "s2");

    LocalPlaylistDetailViewModel vm(&playlistRepo, &songRepo);
    QSignalSpy idSpy(&vm, &LocalPlaylistDetailViewModel::playlistIdChanged);
    QSignalSpy nameSpy(&vm, &LocalPlaylistDetailViewModel::playlistNameChanged);

    vm.loadPlaylist(pl.id);

    QCOMPARE(vm.playlistId(), pl.id);
    QCOMPARE(vm.playlistName(), QStringLiteral("My Playlist"));
    QCOMPARE(vm.songs()->count(), 2);
    QCOMPARE(idSpy.count(), 1);
    QCOMPARE(nameSpy.count(), 1);
}

void TestLocalPlaylistDetailViewModel::loadPlaylist_notFound()
{
    auto db = createDb();
    PlaylistRepository playlistRepo(db.get());
    SongRepository songRepo(db.get());

    LocalPlaylistDetailViewModel vm(&playlistRepo, &songRepo);
    QSignalSpy errorSpy(&vm, &LocalPlaylistDetailViewModel::errorChanged);

    vm.loadPlaylist("nonexistent");

    QVERIFY(vm.hasError());
    QCOMPARE(vm.error().type(), ViewModelError::ErrorType::NotFound);
    QCOMPARE(errorSpy.count(), 1);
}

void TestLocalPlaylistDetailViewModel::deletePlaylist()
{
    auto db = createDb();
    PlaylistRepository playlistRepo(db.get());
    SongRepository songRepo(db.get());

    auto pl = playlistRepo.create("To Delete");

    LocalPlaylistDetailViewModel vm(&playlistRepo, &songRepo);
    vm.loadPlaylist(pl.id);

    QSignalSpy spy(&vm, &LocalPlaylistDetailViewModel::playlistDeleted);
    vm.deletePlaylist();

    QCOMPARE(spy.count(), 1);
    // Verify deleted from DB
    QVERIFY(!playlistRepo.findById(pl.id).has_value());
}

void TestLocalPlaylistDetailViewModel::rename()
{
    auto db = createDb();
    PlaylistRepository playlistRepo(db.get());
    SongRepository songRepo(db.get());

    auto pl = playlistRepo.create("Old Name");

    LocalPlaylistDetailViewModel vm(&playlistRepo, &songRepo);
    vm.loadPlaylist(pl.id);

    QSignalSpy spy(&vm, &LocalPlaylistDetailViewModel::playlistNameChanged);
    vm.rename("New Name");

    QCOMPARE(vm.playlistName(), QStringLiteral("New Name"));
    QCOMPARE(spy.count(), 1);
    // Verify persisted
    auto found = playlistRepo.findById(pl.id);
    QVERIFY(found.has_value());
    QCOMPARE(found->name, QStringLiteral("New Name"));
}

void TestLocalPlaylistDetailViewModel::playSong()
{
    auto db = createDb();
    PlaylistRepository playlistRepo(db.get());
    SongRepository songRepo(db.get());

    auto pl = playlistRepo.create("Test PL");
    insertSongRow(db.get(), "s1", "Song A");
    insertSongRow(db.get(), "s2", "Song B");
    playlistRepo.addSong(pl.id, "s1");
    playlistRepo.addSong(pl.id, "s2");

    LocalPlaylistDetailViewModel vm(&playlistRepo, &songRepo);
    vm.loadPlaylist(pl.id);

    QSignalSpy spy(&vm, &LocalPlaylistDetailViewModel::requestPlay);
    vm.playSong(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().value<Song>().name, QStringLiteral("Song B"));
}

void TestLocalPlaylistDetailViewModel::playAll()
{
    auto db = createDb();
    PlaylistRepository playlistRepo(db.get());
    SongRepository songRepo(db.get());

    auto pl = playlistRepo.create("Test PL");
    insertSongRow(db.get(), "s1", "Song A");
    insertSongRow(db.get(), "s2", "Song B");
    playlistRepo.addSong(pl.id, "s1");
    playlistRepo.addSong(pl.id, "s2");

    LocalPlaylistDetailViewModel vm(&playlistRepo, &songRepo);
    vm.loadPlaylist(pl.id);

    QSignalSpy spy(&vm, &LocalPlaylistDetailViewModel::requestPlayPlaylist);
    vm.playAll();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().value<QVector<Song>>().size(), 2);
    QCOMPARE(spy.first().at(1).toInt(), 0);
}

void TestLocalPlaylistDetailViewModel::addSong_repoException_setsError()
{
    auto db = createDb();
    PlaylistRepository playlistRepo(db.get());
    SongRepository songRepo(db.get());

    auto pl = playlistRepo.create("Test PL");
    insertSongRow(db.get(), "s1", "Song 1");
    playlistRepo.addSong(pl.id, "s1");

    LocalPlaylistDetailViewModel vm(&playlistRepo, &songRepo);
    vm.loadPlaylist(pl.id);

    db->close();

    QSignalSpy errorSpy(&vm, &LocalPlaylistDetailViewModel::errorChanged);
    vm.addSong("s2");

    QVERIFY(vm.hasError());
    QCOMPARE(vm.error().type(), ViewModelError::ErrorType::Database);
    QVERIFY(errorSpy.count() >= 1);
}

void TestLocalPlaylistDetailViewModel::removeSong_repoException_setsError()
{
    auto db = createDb();
    PlaylistRepository playlistRepo(db.get());
    SongRepository songRepo(db.get());

    auto pl = playlistRepo.create("Test PL");
    insertSongRow(db.get(), "s1", "Song 1");
    playlistRepo.addSong(pl.id, "s1");

    LocalPlaylistDetailViewModel vm(&playlistRepo, &songRepo);
    vm.loadPlaylist(pl.id);

    db->close();

    QSignalSpy errorSpy(&vm, &LocalPlaylistDetailViewModel::errorChanged);
    vm.removeSong("s1");

    QVERIFY(vm.hasError());
    QCOMPARE(vm.error().type(), ViewModelError::ErrorType::Database);
    QVERIFY(errorSpy.count() >= 1);
}

void TestLocalPlaylistDetailViewModel::reorderSongs_repoException_setsError()
{
    auto db = createDb();
    PlaylistRepository playlistRepo(db.get());
    SongRepository songRepo(db.get());

    auto pl = playlistRepo.create("Test PL");
    insertSongRow(db.get(), "s1", "Song 1");
    insertSongRow(db.get(), "s2", "Song 2");
    playlistRepo.addSong(pl.id, "s1");
    playlistRepo.addSong(pl.id, "s2");

    LocalPlaylistDetailViewModel vm(&playlistRepo, &songRepo);
    vm.loadPlaylist(pl.id);

    db->close();

    QSignalSpy errorSpy(&vm, &LocalPlaylistDetailViewModel::errorChanged);
    vm.reorderSongs({"s2", "s1"});

    QVERIFY(vm.hasError());
    QCOMPARE(vm.error().type(), ViewModelError::ErrorType::Database);
    QVERIFY(errorSpy.count() >= 1);
}

QTEST_MAIN(TestLocalPlaylistDetailViewModel)
#include "TestLocalPlaylistDetailViewModel.moc"
