/// @file TestNeteasePlaylistDetailViewModel.cpp
/// @brief Unit tests for NeteasePlaylistDetailViewModel

#include "domain/Song.h"
#include "repo/IPlaylistRepository.h"
#include "repo/ISongRepository.h"
#include "viewmodel/NeteasePlaylistDetailViewModel.h"

#include <QSignalSpy>
#include <QTest>

using namespace QeriPlayerQt;

// Simplified tests - NeteaseClient requires full infra, so test what we can

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
    void saveBatch(const QVector<Song> &songs) override
    {
        m_savedSongs.append(songs);
    }
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
    QVector<Song> m_savedSongs;
};

class MockPlaylistRepo : public IPlaylistRepository {
public:
    QVector<PlaylistSummary> findAll() override
    {
        return {};
    }
    std::optional<Playlist> findById(const QString &) override
    {
        return std::nullopt;
    }
    Playlist create(const QString &name, MusicPlatform) override
    {
        Playlist p;
        p.id = "local-1";
        p.name = name;
        m_createdName = name;
        return p;
    }
    void updateMetadata(const QString &, const QString &, const QString &, const QString &) override { }
    void remove(const QString &) override { }
    bool addSong(const QString &, const QString &, int) override
    {
        m_addSongCount++;
        return true;
    }
    void removeSong(const QString &, const QString &) override { }
    void reorderSongs(const QString &, const QStringList &) override { }
    int songCount(const QString &) override
    {
        return 0;
    }
    QString m_createdName;
    int m_addSongCount = 0;
};

class TestNeteasePlaylistDetailViewModel : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initialState();
    void saveToLocal_emptyPlaylist();
};

void TestNeteasePlaylistDetailViewModel::initialState()
{
    MockSongRepo songRepo;
    MockPlaylistRepo playlistRepo;

    NeteasePlaylistDetailViewModel vm(nullptr, &songRepo, &playlistRepo);

    QVERIFY(vm.headerName().isEmpty());
    QVERIFY(vm.headerCoverUrl().isEmpty());
    QCOMPARE(vm.headerTrackCount(), 0);
    QCOMPARE(vm.songs()->count(), 0);
    QVERIFY(!vm.isLoading());
    QVERIFY(!vm.hasError());
}

void TestNeteasePlaylistDetailViewModel::saveToLocal_emptyPlaylist()
{
    MockSongRepo songRepo;
    MockPlaylistRepo playlistRepo;

    NeteasePlaylistDetailViewModel vm(nullptr, &songRepo, &playlistRepo);

    // saveToLocal with no data should be no-op
    vm.saveToLocal();

    QCOMPARE(playlistRepo.m_addSongCount, 0);
}

QTEST_MAIN(TestNeteasePlaylistDetailViewModel)
#include "TestNeteasePlaylistDetailViewModel.moc"
