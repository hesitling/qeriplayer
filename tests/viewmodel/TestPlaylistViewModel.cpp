/// @file TestPlaylistViewModel.cpp
/// @brief Unit tests for PlaylistViewModel (in-memory SQLite)

#include "core/database/DatabaseManager.h"
#include "domain/Playlist.h"
#include "domain/PlaylistSummary.h"
#include "repo/PlaylistRepository.h"
#include "viewmodel/IPlaylistLibraryClient.h"
#include "viewmodel/PlaylistViewModel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

#include <memory>

using namespace QeriPlayerQt;

class MockPlaylistLibraryClient : public IPlaylistLibraryClient {
public:
    QVector<Playlist> remotePlaylists;
    QJsonObject remoteAlbums;

    QCoro::Task<ApiResult<QVector<Playlist>>> getUserPlaylists(const QString &) override
    {
        co_return ApiResult<QVector<Playlist>>(remotePlaylists);
    }

    QCoro::Task<ApiResult<QJsonObject>> getUserStarredAlbums(const QString &, int = 1000, int = 0) override
    {
        co_return ApiResult<QJsonObject>(remoteAlbums);
    }
};

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
    void openLocalPlaylist_validIndex_emitsSignal();
    void openNeteasePlaylist_validIndex_emitsSignal();
    void openNeteaseAlbum_validIndex_emitsSignal();
    void openPlaylist_invalidIndex_noSignal();
    void loadLocalPlaylists_repoException_doesNotCrash();
    void createLocalPlaylist_repoException_setsError();
    void deleteLocalPlaylist_repoException_setsError();
    void renameLocalPlaylist_repoException_setsError();
};

void TestPlaylistViewModel::initialState()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    MockPlaylistLibraryClient client;

    PlaylistViewModel vm(&repo, &client);

    QVERIFY(vm.localPlaylists().isEmpty());
    QVERIFY(vm.neteasePlaylists().isEmpty());
    QVERIFY(!vm.isLoading());
    QVERIFY(!vm.hasError());
}

void TestPlaylistViewModel::loadLocalPlaylists()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    MockPlaylistLibraryClient client;

    repo.create("Playlist A");
    repo.create("Playlist B");

    PlaylistViewModel vm(&repo, &client);
    QSignalSpy spy(&vm, &PlaylistViewModel::localPlaylistsChanged);

    vm.loadLocalPlaylists();

    QCOMPARE(vm.localPlaylists().size(), 2);
    QCOMPARE(spy.count(), 1);
}

void TestPlaylistViewModel::loadLocalPlaylists_empty()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    MockPlaylistLibraryClient client;

    PlaylistViewModel vm(&repo, &client);
    vm.loadLocalPlaylists();

    QCOMPARE(vm.localPlaylists().size(), 0);
}

void TestPlaylistViewModel::createLocalPlaylist()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    MockPlaylistLibraryClient client;

    PlaylistViewModel vm(&repo, &client);
    vm.createLocalPlaylist("New Playlist");

    auto all = repo.findAll();
    QCOMPARE(all.size(), 1);
    QCOMPARE(all.first().name, QStringLiteral("New Playlist"));
}

void TestPlaylistViewModel::deleteLocalPlaylist()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    MockPlaylistLibraryClient client;

    auto pl = repo.create("To Delete");

    PlaylistViewModel vm(&repo, &client);
    vm.deleteLocalPlaylist(pl.id);

    QVERIFY(!repo.findById(pl.id).has_value());
}

void TestPlaylistViewModel::renameLocalPlaylist()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    MockPlaylistLibraryClient client;

    auto pl = repo.create("Old Name");

    PlaylistViewModel vm(&repo, &client);
    vm.loadLocalPlaylists();
    vm.renameLocalPlaylist(pl.id, "New Name");

    auto found = repo.findById(pl.id);
    QVERIFY(found.has_value());
    QCOMPARE(found->name, QStringLiteral("New Name"));
}

void TestPlaylistViewModel::clearError()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    MockPlaylistLibraryClient client;

    PlaylistViewModel vm(&repo, &client);

    QSignalSpy spy(&vm, &PlaylistViewModel::errorChanged);
    vm.clearError();

    QVERIFY(!vm.hasError());
    QCOMPARE(spy.count(), 1);
}

void TestPlaylistViewModel::localPlaylistSelected_signal()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    MockPlaylistLibraryClient client;

    PlaylistViewModel vm(&repo, &client);
    QSignalSpy spy(&vm, &PlaylistViewModel::localPlaylistSelected);

    Q_EMIT vm.localPlaylistSelected("abc");

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), QStringLiteral("abc"));
}

void TestPlaylistViewModel::neteasePlaylistSelected_signal()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    MockPlaylistLibraryClient client;

    PlaylistViewModel vm(&repo, &client);
    QSignalSpy spy(&vm, &PlaylistViewModel::neteasePlaylistSelected);

    PlaylistSummary summary;
    summary.id = "100";
    summary.name = "Test";
    Q_EMIT vm.neteasePlaylistSelected(summary);

    QCOMPARE(spy.count(), 1);
}

void TestPlaylistViewModel::openLocalPlaylist_validIndex_emitsSignal()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    MockPlaylistLibraryClient client;

    auto playlistA = repo.create("Playlist A");
    repo.create("Playlist B");

    PlaylistViewModel vm(&repo, &client);
    vm.loadLocalPlaylists();

    QSignalSpy spy(&vm, &PlaylistViewModel::localPlaylistSelected);
    vm.openLocalPlaylist(0);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), playlistA.id);
}

void TestPlaylistViewModel::openNeteasePlaylist_validIndex_emitsSignal()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    MockPlaylistLibraryClient client;

    Playlist playlistA;
    playlistA.id = QStringLiteral("200");
    playlistA.name = QStringLiteral("Remote Playlist A");
    playlistA.songCount = 3;
    Playlist playlistB;
    playlistB.id = QStringLiteral("201");
    playlistB.name = QStringLiteral("Remote Playlist B");
    playlistB.songCount = 7;
    client.remotePlaylists = {playlistA, playlistB};

    PlaylistViewModel vm(&repo, &client);
    vm.loadNeteasePlaylists();

    QSignalSpy selectSpy(&vm, &PlaylistViewModel::neteasePlaylistSelected);
    QTRY_COMPARE(vm.neteasePlaylists().size(), 2);
    vm.openNeteasePlaylist(1);

    QCOMPARE(selectSpy.count(), 1);
    QCOMPARE(selectSpy.takeFirst().at(0).value<PlaylistSummary>().id, QStringLiteral("201"));
}

void TestPlaylistViewModel::openNeteaseAlbum_validIndex_emitsSignal()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    MockPlaylistLibraryClient client;

    QJsonArray playlistArray;
    QJsonObject album;
    album.insert(QStringLiteral("id"), QStringLiteral("300"));
    album.insert(QStringLiteral("name"), QStringLiteral("Remote Album"));
    album.insert(QStringLiteral("coverImgUrl"), QStringLiteral("https://example.com/album.jpg"));
    album.insert(QStringLiteral("trackCount"), 8);
    playlistArray.append(album);
    client.remoteAlbums.insert(QStringLiteral("playlist"), playlistArray);

    PlaylistViewModel vm(&repo, &client);
    vm.loadNeteaseAlbums();

    QSignalSpy selectSpy(&vm, &PlaylistViewModel::neteaseAlbumSelected);
    QTRY_COMPARE(vm.neteaseAlbums().size(), 1);
    vm.openNeteaseAlbum(0);

    QCOMPARE(selectSpy.count(), 1);
    QCOMPARE(selectSpy.takeFirst().at(0).value<AlbumSummary>().id, QStringLiteral("300"));
}

void TestPlaylistViewModel::openPlaylist_invalidIndex_noSignal()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    MockPlaylistLibraryClient client;

    repo.create("Playlist A");

    PlaylistViewModel vm(&repo, &client);
    vm.loadLocalPlaylists();

    QSignalSpy localSpy(&vm, &PlaylistViewModel::localPlaylistSelected);
    QSignalSpy playlistSpy(&vm, &PlaylistViewModel::neteasePlaylistSelected);
    QSignalSpy albumSpy(&vm, &PlaylistViewModel::neteaseAlbumSelected);

    vm.openLocalPlaylist(999);
    vm.openNeteasePlaylist(999);
    vm.openNeteaseAlbum(999);

    QCOMPARE(localSpy.count(), 0);
    QCOMPARE(playlistSpy.count(), 0);
    QCOMPARE(albumSpy.count(), 0);
}

void TestPlaylistViewModel::loadLocalPlaylists_repoException_doesNotCrash()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    MockPlaylistLibraryClient client;

    PlaylistViewModel vm(&repo, &client);
    db->close();
    vm.loadLocalPlaylists();

    QVERIFY(vm.localPlaylists().isEmpty());
}

void TestPlaylistViewModel::createLocalPlaylist_repoException_setsError()
{
    auto db = createDb();
    PlaylistRepository repo(db.get());
    MockPlaylistLibraryClient client;

    PlaylistViewModel vm(&repo, &client);
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
    MockPlaylistLibraryClient client;

    repo.create("To Delete");

    PlaylistViewModel vm(&repo, &client);
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
    MockPlaylistLibraryClient client;

    auto pl = repo.create("Old Name");

    PlaylistViewModel vm(&repo, &client);
    db->close();

    QSignalSpy errorSpy(&vm, &PlaylistViewModel::errorChanged);
    vm.renameLocalPlaylist(pl.id, "New Name");

    QVERIFY(vm.hasError());
    QCOMPARE(vm.error().type(), ViewModelError::ErrorType::Database);
    QVERIFY(errorSpy.count() >= 1);
}

QTEST_MAIN(TestPlaylistViewModel)
#include "TestPlaylistViewModel.moc"
