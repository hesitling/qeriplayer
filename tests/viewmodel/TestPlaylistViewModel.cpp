/// @file TestPlaylistViewModel.cpp
/// @brief Unit tests for PlaylistViewModel (in-memory SQLite)

#include "core/database/DatabaseManager.h"
#include "domain/Playlist.h"
#include "domain/PlaylistSummary.h"
#include "repo/PlaylistRepository.h"
#include "viewmodel/PlaylistViewModel.h"

#include <QSignalSpy>
#include <QTest>

#include <memory>

using namespace QeriPlayerQt;

class TestPlaylistViewModel : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<DatabaseManager> createDb()
    {
        auto db = std::make_unique<DatabaseManager>();
        db->open(QString(":memory:"));
        return db;
    }

private Q_SLOTS:
    void initialState();
    void loadLocalPlaylists();
    void loadLocalPlaylists_empty();
    void createLocalPlaylist();
    void deleteLocalPlaylist();
    void renameLocalPlaylist();
    void clearError();
    void localPlaylistSelected_signal();
    void neteasePlaylistSelected_signal();
    void loadLocalPlaylists_repoException_doesNotCrash();
    void createLocalPlaylist_repoException_setsError();
    void deleteLocalPlaylist_repoException_setsError();
    void renameLocalPlaylist_repoException_setsError();
};

void TestPlaylistViewModel::initialState()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    PlaylistViewModel vm(&repo, nullptr);

    QVERIFY(vm.localPlaylists().isEmpty());
    QVERIFY(vm.neteasePlaylists().isEmpty());
    QVERIFY(!vm.isLoading());
    QVERIFY(!vm.hasError());
}

void TestPlaylistViewModel::loadLocalPlaylists()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    repo.create("Playlist A");
    repo.create("Playlist B");

    PlaylistViewModel vm(&repo, nullptr);
    QSignalSpy spy(&vm, &PlaylistViewModel::localPlaylistsChanged);

    vm.loadLocalPlaylists();

    QCOMPARE(vm.localPlaylists().size(), 2);
    QCOMPARE(spy.count(), 1);
}

void TestPlaylistViewModel::loadLocalPlaylists_empty()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    PlaylistViewModel vm(&repo, nullptr);
    vm.loadLocalPlaylists();

    QCOMPARE(vm.localPlaylists().size(), 0);
}

void TestPlaylistViewModel::createLocalPlaylist()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    PlaylistViewModel vm(&repo, nullptr);
    vm.createLocalPlaylist("New Playlist");

    // Verify persisted via repo
    auto all = repo.findAll();
    QCOMPARE(all.size(), 1);
    QCOMPARE(all.first().name, QStringLiteral("New Playlist"));
}

void TestPlaylistViewModel::deleteLocalPlaylist()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    auto pl = repo.create("To Delete");

    PlaylistViewModel vm(&repo, nullptr);
    vm.deleteLocalPlaylist(pl.id);

    // Verify deleted from DB
    QVERIFY(!repo.findById(pl.id).has_value());
}

void TestPlaylistViewModel::renameLocalPlaylist()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    auto pl = repo.create("Old Name");

    PlaylistViewModel vm(&repo, nullptr);
    vm.loadLocalPlaylists(); // Populate the list first
    vm.renameLocalPlaylist(pl.id, "New Name");

    // Verify persisted
    auto found = repo.findById(pl.id);
    QVERIFY(found.has_value());
    QCOMPARE(found->name, QStringLiteral("New Name"));
}

void TestPlaylistViewModel::clearError()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    PlaylistViewModel vm(&repo, nullptr);

    QSignalSpy spy(&vm, &PlaylistViewModel::errorChanged);
    vm.clearError();

    QVERIFY(!vm.hasError());
    QCOMPARE(spy.count(), 1);
}

void TestPlaylistViewModel::localPlaylistSelected_signal()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    PlaylistViewModel vm(&repo, nullptr);
    QSignalSpy spy(&vm, &PlaylistViewModel::localPlaylistSelected);

    Q_EMIT vm.localPlaylistSelected("abc");

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), QStringLiteral("abc"));
}

void TestPlaylistViewModel::neteasePlaylistSelected_signal()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    PlaylistViewModel vm(&repo, nullptr);
    QSignalSpy spy(&vm, &PlaylistViewModel::neteasePlaylistSelected);

    PlaylistSummary summary;
    summary.id = "100";
    summary.name = "Test";
    Q_EMIT vm.neteasePlaylistSelected(summary);

    QCOMPARE(spy.count(), 1);
}

void TestPlaylistViewModel::loadLocalPlaylists_repoException_doesNotCrash()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    PlaylistViewModel vm(&repo, nullptr);

    // Close DB — findAll() throws, caught internally by loadLocalPlaylistsImpl
    db->close();

    vm.loadLocalPlaylists();

    // Local playlists should remain empty since the repo threw
    QVERIFY(vm.localPlaylists().isEmpty());
}

void TestPlaylistViewModel::createLocalPlaylist_repoException_setsError()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    PlaylistViewModel vm(&repo, nullptr);

    db->close();

    QSignalSpy errorSpy(&vm, &PlaylistViewModel::errorChanged);
    vm.createLocalPlaylist("New Playlist");

    QVERIFY(vm.hasError());
    QCOMPARE(vm.error().type(), ViewModelError::ErrorType::Database);
    QVERIFY(errorSpy.count() >= 1);
}

void TestPlaylistViewModel::deleteLocalPlaylist_repoException_setsError()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    repo.create("To Delete");

    PlaylistViewModel vm(&repo, nullptr);

    db->close();

    QSignalSpy errorSpy(&vm, &PlaylistViewModel::errorChanged);
    vm.deleteLocalPlaylist("nonexistent");

    QVERIFY(vm.hasError());
    QCOMPARE(vm.error().type(), ViewModelError::ErrorType::Database);
    QVERIFY(errorSpy.count() >= 1);
}

void TestPlaylistViewModel::renameLocalPlaylist_repoException_setsError()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());

    auto pl = repo.create("Old Name");

    PlaylistViewModel vm(&repo, nullptr);

    db->close();

    QSignalSpy errorSpy(&vm, &PlaylistViewModel::errorChanged);
    vm.renameLocalPlaylist(pl.id, "New Name");

    QVERIFY(vm.hasError());
    QCOMPARE(vm.error().type(), ViewModelError::ErrorType::Database);
    QVERIFY(errorSpy.count() >= 1);
}

QTEST_MAIN(TestPlaylistViewModel)
#include "TestPlaylistViewModel.moc"
