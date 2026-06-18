/// @file TestLocalPlaylistDetailViewModel.cpp
/// @brief Unit tests for LocalPlaylistDetailViewModel

#include "domain/Playlist.h"
#include "domain/Song.h"
#include "repo/IPlaylistRepository.h"
#include "repo/ISongRepository.h"
#include "viewmodel/LocalPlaylistDetailViewModel.h"

#include <QSignalSpy>
#include <QTest>

using namespace QeriPlayerQt;

class MockPlaylistRepo : public IPlaylistRepository {
public:
    QVector<PlaylistSummary> findAll() override
    {
        return {};
    }
    std::optional<Playlist> findById(const QString &id) override
    {
        auto it = m_playlists.find(id);
        if (it != m_playlists.end())
            return it.value();
        return std::nullopt;
    }
    Playlist create(const QString &name, MusicPlatform) override
    {
        Playlist p;
        p.id = "new-id";
        p.name = name;
        return p;
    }
    void updateMetadata(const QString &id, const QString &name, const QString &, const QString &) override
    {
        m_renamedIds[id] = name;
    }
    void remove(const QString &id) override
    {
        m_removedIds.append(id);
    }
    bool addSong(const QString &, const QString &, int) override
    {
        return true;
    }
    void removeSong(const QString &, const QString &) override
    {
        m_removeSongCalled = true;
    }
    void reorderSongs(const QString &, const QStringList &) override
    {
        m_reorderCalled = true;
    }
    int songCount(const QString &) override
    {
        return 0;
    }

    QHash<QString, Playlist> m_playlists;
    QStringList m_removedIds;
    QHash<QString, QString> m_renamedIds;
    bool m_removeSongCalled = false;
    bool m_reorderCalled = false;
};

class MockSongRepo : public ISongRepository {
public:
    std::optional<Song> findById(const QString &) override
    {
        return std::nullopt;
    }
    QVector<Song> findByIds(const QStringList &) override
    {
        return {};
    }
    void save(const Song &) override { }
    void saveBatch(const QVector<Song> &) override { }
    void remove(const QString &) override { }
    bool exists(const QString &) override
    {
        return false;
    }
    QVector<Song> findByPlatform(MusicPlatform) override
    {
        return {};
    }
    QVector<Song> search(const QString &, int) override
    {
        return {};
    }
};

class TestLocalPlaylistDetailViewModel : public QObject {
    Q_OBJECT

private:
    Song makeSong(const QString &id, const QString &name)
    {
        Song s;
        s.id = id;
        s.name = name;
        return s;
    }

private Q_SLOTS:
    void loadPlaylist_found();
    void loadPlaylist_notFound();
    void deletePlaylist();
    void rename();
    void playSong();
    void playAll();
};

void TestLocalPlaylistDetailViewModel::loadPlaylist_found()
{
    MockPlaylistRepo playlistRepo;
    MockSongRepo songRepo;

    Playlist playlist;
    playlist.id = "abc";
    playlist.name = "My Playlist";
    playlist.songs = {makeSong("1", "Song A"), makeSong("2", "Song B")};
    playlistRepo.m_playlists["abc"] = playlist;

    LocalPlaylistDetailViewModel vm(&playlistRepo, &songRepo);
    QSignalSpy idSpy(&vm, &LocalPlaylistDetailViewModel::playlistIdChanged);
    QSignalSpy nameSpy(&vm, &LocalPlaylistDetailViewModel::playlistNameChanged);

    vm.loadPlaylist("abc");

    QCOMPARE(vm.playlistId(), QStringLiteral("abc"));
    QCOMPARE(vm.playlistName(), QStringLiteral("My Playlist"));
    QCOMPARE(vm.songs()->count(), 2);
    QCOMPARE(idSpy.count(), 1);
    QCOMPARE(nameSpy.count(), 1);
}

void TestLocalPlaylistDetailViewModel::loadPlaylist_notFound()
{
    MockPlaylistRepo playlistRepo;
    MockSongRepo songRepo;

    LocalPlaylistDetailViewModel vm(&playlistRepo, &songRepo);
    QSignalSpy errorSpy(&vm, &LocalPlaylistDetailViewModel::errorChanged);

    vm.loadPlaylist("nonexistent");

    QVERIFY(vm.hasError());
    QCOMPARE(vm.error().type(), ViewModelError::ErrorType::NotFound);
    QCOMPARE(errorSpy.count(), 1);
}

void TestLocalPlaylistDetailViewModel::deletePlaylist()
{
    MockPlaylistRepo playlistRepo;
    MockSongRepo songRepo;

    Playlist playlist;
    playlist.id = "abc";
    playlist.name = "Test";
    playlistRepo.m_playlists["abc"] = playlist;

    LocalPlaylistDetailViewModel vm(&playlistRepo, &songRepo);
    vm.loadPlaylist("abc");

    QSignalSpy spy(&vm, &LocalPlaylistDetailViewModel::playlistDeleted);
    vm.deletePlaylist();

    QCOMPARE(playlistRepo.m_removedIds.size(), 1);
    QCOMPARE(spy.count(), 1);
}

void TestLocalPlaylistDetailViewModel::rename()
{
    MockPlaylistRepo playlistRepo;
    MockSongRepo songRepo;

    Playlist playlist;
    playlist.id = "abc";
    playlist.name = "Old Name";
    playlistRepo.m_playlists["abc"] = playlist;

    LocalPlaylistDetailViewModel vm(&playlistRepo, &songRepo);
    vm.loadPlaylist("abc");

    QSignalSpy spy(&vm, &LocalPlaylistDetailViewModel::playlistNameChanged);
    vm.rename("New Name");

    QCOMPARE(vm.playlistName(), QStringLiteral("New Name"));
    QCOMPARE(playlistRepo.m_renamedIds["abc"], QStringLiteral("New Name"));
    QCOMPARE(spy.count(), 1);
}

void TestLocalPlaylistDetailViewModel::playSong()
{
    MockPlaylistRepo playlistRepo;
    MockSongRepo songRepo;

    Playlist playlist;
    playlist.id = "abc";
    playlist.name = "Test";
    playlist.songs = {makeSong("1", "Song A"), makeSong("2", "Song B")};
    playlistRepo.m_playlists["abc"] = playlist;

    LocalPlaylistDetailViewModel vm(&playlistRepo, &songRepo);
    vm.loadPlaylist("abc");

    QSignalSpy spy(&vm, &LocalPlaylistDetailViewModel::requestPlay);
    vm.playSong(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().value<Song>().name, QStringLiteral("Song B"));
}

void TestLocalPlaylistDetailViewModel::playAll()
{
    MockPlaylistRepo playlistRepo;
    MockSongRepo songRepo;

    Playlist playlist;
    playlist.id = "abc";
    playlist.name = "Test";
    playlist.songs = {makeSong("1", "Song A"), makeSong("2", "Song B")};
    playlistRepo.m_playlists["abc"] = playlist;

    LocalPlaylistDetailViewModel vm(&playlistRepo, &songRepo);
    vm.loadPlaylist("abc");

    QSignalSpy spy(&vm, &LocalPlaylistDetailViewModel::requestPlayPlaylist);
    vm.playAll();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().value<QVector<Song>>().size(), 2);
    QCOMPARE(spy.first().at(1).toInt(), 0);
}

QTEST_MAIN(TestLocalPlaylistDetailViewModel)
#include "TestLocalPlaylistDetailViewModel.moc"
