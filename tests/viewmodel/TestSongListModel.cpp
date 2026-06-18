/// @file TestSongListModel.cpp
/// @brief Unit tests for SongListModel

#include "domain/Song.h"
#include "viewmodel/SongListModel.h"

#include <QSignalSpy>
#include <QTest>

using namespace QeriPlayerQt;

class TestSongListModel : public QObject {
    Q_OBJECT

private:
    Song makeSong(const QString &id, const QString &name, const QString &artist = "Artist",
                  const QString &album = "Album", qint64 durationMs = 180000)
    {
        Song song;
        song.id = id;
        song.name = name;
        song.artist = artist;
        song.album = album;
        song.durationMs = durationMs;
        song.platform = MusicPlatform::NetEase;
        return song;
    }

private Q_SLOTS:
    void emptyModel();
    void setSongs();
    void setSongs_emitsCountChanged();
    void appendSongs();
    void appendSongs_toEmpty();
    void appendSongs_emitsCountChanged();
    void appendSongs_emptyList();
    void clear();
    void clear_emitsCountChanged();
    void songAt_validIndex();
    void songAt_outOfBounds();
    void songs();
    void count();
    void roleNames();
    void data_roles();
    void data_invalidIndex();
    void data_isPlayingRole();
    void setPlayingIndex();
    void setPlayingIndex_emitsDataChanged();
    void setPlayingIndex_sameIndex();
    void setSongs_resetsPlayingIndex();
    void setPlayingIndex_invalidIndex();
};

void TestSongListModel::emptyModel()
{
    SongListModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.count(), 0);
    QVERIFY(model.songs().isEmpty());
}

void TestSongListModel::setSongs()
{
    SongListModel model;
    QVector<Song> songs = {makeSong("1", "Song A"), makeSong("2", "Song B"), makeSong("3", "Song C")};

    model.setSongs(songs);

    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.count(), 3);
    QCOMPARE(model.songAt(0).name, QStringLiteral("Song A"));
    QCOMPARE(model.songAt(1).name, QStringLiteral("Song B"));
    QCOMPARE(model.songAt(2).name, QStringLiteral("Song C"));
}

void TestSongListModel::setSongs_emitsCountChanged()
{
    SongListModel model;
    QSignalSpy spy(&model, &SongListModel::countChanged);

    model.setSongs({makeSong("1", "A")});
    QCOMPARE(spy.count(), 1);

    // Same count — should still emit (resetModel always emits)
    model.setSongs({makeSong("2", "B")});
    QCOMPARE(spy.count(), 2);
}

void TestSongListModel::appendSongs()
{
    SongListModel model;
    model.setSongs({makeSong("1", "A"), makeSong("2", "B")});

    model.appendSongs({makeSong("3", "C"), makeSong("4", "D")});

    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(model.songAt(2).name, QStringLiteral("C"));
    QCOMPARE(model.songAt(3).name, QStringLiteral("D"));
}

void TestSongListModel::appendSongs_toEmpty()
{
    SongListModel model;
    model.appendSongs({makeSong("1", "A")});

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.songAt(0).name, QStringLiteral("A"));
}

void TestSongListModel::appendSongs_emitsCountChanged()
{
    SongListModel model;
    QSignalSpy spy(&model, &SongListModel::countChanged);

    model.appendSongs({makeSong("1", "A")});
    QCOMPARE(spy.count(), 1);
}

void TestSongListModel::appendSongs_emptyList()
{
    SongListModel model;
    model.setSongs({makeSong("1", "A")});

    QSignalSpy spy(&model, &SongListModel::countChanged);
    model.appendSongs({});

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(spy.count(), 0); // No change
}

void TestSongListModel::clear()
{
    SongListModel model;
    model.setSongs({makeSong("1", "A"), makeSong("2", "B")});

    model.clear();

    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.count(), 0);
    QVERIFY(model.songs().isEmpty());
}

void TestSongListModel::clear_emitsCountChanged()
{
    SongListModel model;
    model.setSongs({makeSong("1", "A")});

    QSignalSpy spy(&model, &SongListModel::countChanged);
    model.clear();

    QCOMPARE(spy.count(), 1);
}

void TestSongListModel::songAt_validIndex()
{
    SongListModel model;
    model.setSongs({makeSong("1", "A"), makeSong("2", "B")});

    QCOMPARE(model.songAt(0).name, QStringLiteral("A"));
    QCOMPARE(model.songAt(1).name, QStringLiteral("B"));
}

void TestSongListModel::songAt_outOfBounds()
{
    SongListModel model;
    model.setSongs({makeSong("1", "A")});

    Song song = model.songAt(-1);
    QVERIFY(song.id.isEmpty());

    song = model.songAt(5);
    QVERIFY(song.id.isEmpty());
}

void TestSongListModel::songs()
{
    SongListModel model;
    QVector<Song> input = {makeSong("1", "A"), makeSong("2", "B")};
    model.setSongs(input);

    QVector<Song> output = model.songs();
    QCOMPARE(output.size(), 2);
    QCOMPARE(output[0].name, QStringLiteral("A"));
    QCOMPARE(output[1].name, QStringLiteral("B"));
}

void TestSongListModel::count()
{
    SongListModel model;
    QCOMPARE(model.count(), 0);

    model.setSongs({makeSong("1", "A"), makeSong("2", "B"), makeSong("3", "C")});
    QCOMPARE(model.count(), 3);
}

void TestSongListModel::roleNames()
{
    SongListModel model;
    QHash<int, QByteArray> roles = model.roleNames();

    QCOMPARE(roles[SongListModel::IdRole], QByteArray("id"));
    QCOMPARE(roles[SongListModel::NameRole], QByteArray("name"));
    QCOMPARE(roles[SongListModel::ArtistRole], QByteArray("artist"));
    QCOMPARE(roles[SongListModel::AlbumRole], QByteArray("album"));
    QCOMPARE(roles[SongListModel::DurationMsRole], QByteArray("durationMs"));
    QCOMPARE(roles[SongListModel::CoverUrlRole], QByteArray("coverUrl"));
    QCOMPARE(roles[SongListModel::PlatformRole], QByteArray("platform"));
    QCOMPARE(roles[SongListModel::IsPlayingRole], QByteArray("isPlaying"));
}

void TestSongListModel::data_roles()
{
    SongListModel model;
    model.setSongs({makeSong("123", "Test Song", "Test Artist", "Test Album", 240000)});

    QModelIndex idx = model.index(0, 0);
    QCOMPARE(model.data(idx, SongListModel::IdRole).toString(), QStringLiteral("123"));
    QCOMPARE(model.data(idx, SongListModel::NameRole).toString(), QStringLiteral("Test Song"));
    QCOMPARE(model.data(idx, SongListModel::ArtistRole).toString(), QStringLiteral("Test Artist"));
    QCOMPARE(model.data(idx, SongListModel::AlbumRole).toString(), QStringLiteral("Test Album"));
    QCOMPARE(model.data(idx, SongListModel::DurationMsRole).toLongLong(), 240000);
    QCOMPARE(model.data(idx, SongListModel::PlatformRole).value<MusicPlatform>(), MusicPlatform::NetEase);
}

void TestSongListModel::data_invalidIndex()
{
    SongListModel model;
    model.setSongs({makeSong("1", "A")});

    QModelIndex invalid;
    QVERIFY(!model.data(invalid, SongListModel::NameRole).isValid());

    QModelIndex outOfBounds = model.index(5, 0);
    QVERIFY(!model.data(outOfBounds, SongListModel::NameRole).isValid());
}

void TestSongListModel::data_isPlayingRole()
{
    SongListModel model;
    model.setSongs({makeSong("1", "A"), makeSong("2", "B"), makeSong("3", "C")});

    // No playing index set
    QModelIndex idx0 = model.index(0, 0);
    QCOMPARE(model.data(idx0, SongListModel::IsPlayingRole).toBool(), false);

    // Set playing index to 1
    model.setPlayingIndex(1);

    QCOMPARE(model.data(model.index(0, 0), SongListModel::IsPlayingRole).toBool(), false);
    QCOMPARE(model.data(model.index(1, 0), SongListModel::IsPlayingRole).toBool(), true);
    QCOMPARE(model.data(model.index(2, 0), SongListModel::IsPlayingRole).toBool(), false);
}

void TestSongListModel::setPlayingIndex()
{
    SongListModel model;
    model.setSongs({makeSong("1", "A"), makeSong("2", "B"), makeSong("3", "C")});

    model.setPlayingIndex(2);
    QCOMPARE(model.data(model.index(2, 0), SongListModel::IsPlayingRole).toBool(), true);

    model.setPlayingIndex(0);
    QCOMPARE(model.data(model.index(0, 0), SongListModel::IsPlayingRole).toBool(), true);
    QCOMPARE(model.data(model.index(2, 0), SongListModel::IsPlayingRole).toBool(), false);
}

void TestSongListModel::setPlayingIndex_emitsDataChanged()
{
    SongListModel model;
    model.setSongs({makeSong("1", "A"), makeSong("2", "B"), makeSong("3", "C")});

    QSignalSpy spy(&model, &SongListModel::dataChanged);

    model.setPlayingIndex(1);
    // Should emit dataChanged for old (-1, none) and new (1)
    QVERIFY(spy.count() >= 1);
}

void TestSongListModel::setPlayingIndex_sameIndex()
{
    SongListModel model;
    model.setSongs({makeSong("1", "A"), makeSong("2", "B")});
    model.setPlayingIndex(1);

    QSignalSpy spy(&model, &SongListModel::dataChanged);
    model.setPlayingIndex(1); // Same index

    QCOMPARE(spy.count(), 0); // No signal
}

void TestSongListModel::setSongs_resetsPlayingIndex()
{
    SongListModel model;
    model.setSongs({makeSong("1", "A"), makeSong("2", "B"), makeSong("3", "C")});

    // Set playing index to 1
    model.setPlayingIndex(1);
    QCOMPARE(model.data(model.index(1, 0), SongListModel::IsPlayingRole).toBool(), true);

    // Replace songs — playing index should reset
    model.setSongs({makeSong("4", "D"), makeSong("5", "E")});

    // No item should be playing
    QCOMPARE(model.data(model.index(0, 0), SongListModel::IsPlayingRole).toBool(), false);
    QCOMPARE(model.data(model.index(1, 0), SongListModel::IsPlayingRole).toBool(), false);
}

void TestSongListModel::setPlayingIndex_invalidIndex()
{
    SongListModel model;
    model.setSongs({makeSong("1", "A"), makeSong("2", "B")});

    // Set valid index first
    model.setPlayingIndex(0);
    QCOMPARE(model.data(model.index(0, 0), SongListModel::IsPlayingRole).toBool(), true);

    // Try invalid indices — should be rejected
    model.setPlayingIndex(-2);
    QCOMPARE(model.data(model.index(0, 0), SongListModel::IsPlayingRole).toBool(), true); // unchanged

    model.setPlayingIndex(5);
    QCOMPARE(model.data(model.index(0, 0), SongListModel::IsPlayingRole).toBool(), true); // unchanged

    // Valid index should still work
    model.setPlayingIndex(1);
    QCOMPARE(model.data(model.index(1, 0), SongListModel::IsPlayingRole).toBool(), true);
    QCOMPARE(model.data(model.index(0, 0), SongListModel::IsPlayingRole).toBool(), false);
}

QTEST_MAIN(TestSongListModel)
#include "TestSongListModel.moc"
