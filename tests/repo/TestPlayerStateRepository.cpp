/// @file TestPlayerStateRepository.cpp
/// @brief Tests for PlayerStateRepository

#include "core/database/DatabaseManager.h"
#include "repo/PlayerStateRepository.h"

#include <QTest>

using namespace QeriPlayerQt;

class TestPlayerStateRepository : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<DatabaseManager> createDb();

private Q_SLOTS:
    void save_andLoad();
    void load_empty_returnsNull();
    void save_overwritesPreviousState();
    void clear_removesState();
    void roundTrip_allFields();
};

std::unique_ptr<DatabaseManager> TestPlayerStateRepository::createDb()
{
    auto db = std::make_unique<DatabaseManager>();
    db->open(QString(":memory:"));
    return db;
}

void TestPlayerStateRepository::save_andLoad()
{
    auto db = createDb();
    PlayerStateRepository repo(db.get());

    PersistedPlayerState state;
    state.currentIndex = 3;
    state.mediaUrl = "https://media.example.com/song.mp3";
    state.positionMs = 45000;
    state.shouldResumePlayback = true;
    state.repeatMode = RepeatMode::All;
    state.shuffleEnabled = true;

    Song song;
    song.id = "s1";
    song.name = "Song 1";
    song.durationMs = 120000;
    state.playlist = {song};

    repo.save(state);

    auto loaded = repo.load();
    QVERIFY(loaded.has_value());
    QCOMPARE(loaded->currentIndex, 3);
    QCOMPARE(loaded->mediaUrl, QStringLiteral("https://media.example.com/song.mp3"));
    QCOMPARE(loaded->positionMs, 45000);
    QCOMPARE(loaded->shouldResumePlayback, true);
    QCOMPARE(loaded->repeatMode, RepeatMode::All);
    QCOMPARE(loaded->shuffleEnabled, true);
    QCOMPARE(loaded->playlist.size(), 1);
    QCOMPARE(loaded->playlist[0].id, QStringLiteral("s1"));
}

void TestPlayerStateRepository::load_empty_returnsNull()
{
    auto db = createDb();
    PlayerStateRepository repo(db.get());

    auto loaded = repo.load();
    QVERIFY(!loaded.has_value());
}

void TestPlayerStateRepository::save_overwritesPreviousState()
{
    auto db = createDb();
    PlayerStateRepository repo(db.get());

    PersistedPlayerState state1;
    state1.currentIndex = 1;
    state1.positionMs = 10000;
    repo.save(state1);

    PersistedPlayerState state2;
    state2.currentIndex = 5;
    state2.positionMs = 99000;
    repo.save(state2);

    auto loaded = repo.load();
    QVERIFY(loaded.has_value());
    QCOMPARE(loaded->currentIndex, 5);
    QCOMPARE(loaded->positionMs, 99000);

    // Only one row in player_state
    auto rows = db->exec("SELECT COUNT(*) FROM player_state");
    QCOMPARE(rows[0][0].toInt(), 1);
}

void TestPlayerStateRepository::clear_removesState()
{
    auto db = createDb();
    PlayerStateRepository repo(db.get());

    PersistedPlayerState state;
    state.currentIndex = 1;
    repo.save(state);

    repo.clear();

    auto loaded = repo.load();
    QVERIFY(!loaded.has_value());
}

void TestPlayerStateRepository::roundTrip_allFields()
{
    auto db = createDb();
    PlayerStateRepository repo(db.get());

    PersistedPlayerState state;
    Song song1;
    song1.id = "s1";
    song1.name = "Song 1";
    song1.artist = "Artist 1";
    song1.album = "Album 1";
    song1.durationMs = 120000;
    song1.platform = MusicPlatform::NetEase;
    Song song2;
    song2.id = "s2";
    song2.name = "Song 2";
    song2.durationMs = 180000;
    state.playlist = {song1, song2};
    state.currentIndex = 1;
    state.mediaUrl = "https://example.com/play.mp3";
    state.positionMs = 30000;
    state.shouldResumePlayback = true;
    state.repeatMode = RepeatMode::One;
    state.shuffleEnabled = false;

    repo.save(state);

    auto loaded = repo.load();
    QVERIFY(loaded.has_value());
    QCOMPARE(loaded->playlist.size(), 2);
    QCOMPARE(loaded->playlist[0].id, QStringLiteral("s1"));
    QCOMPARE(loaded->playlist[0].name, QStringLiteral("Song 1"));
    QCOMPARE(loaded->playlist[0].artist, QStringLiteral("Artist 1"));
    QCOMPARE(loaded->playlist[0].album, QStringLiteral("Album 1"));
    QCOMPARE(loaded->playlist[0].albumId, QString());
    QCOMPARE(loaded->playlist[0].durationMs, 120000);
    QCOMPARE(loaded->playlist[0].platform, MusicPlatform::NetEase);
    QCOMPARE(loaded->playlist[1].id, QStringLiteral("s2"));
    QCOMPARE(loaded->playlist[1].name, QStringLiteral("Song 2"));
    QCOMPARE(loaded->playlist[1].durationMs, 180000);
    QCOMPARE(loaded->currentIndex, 1);
    QCOMPARE(loaded->mediaUrl, QStringLiteral("https://example.com/play.mp3"));
    QCOMPARE(loaded->positionMs, 30000);
    QCOMPARE(loaded->shouldResumePlayback, true);
    QCOMPARE(loaded->repeatMode, RepeatMode::One);
    QCOMPARE(loaded->shuffleEnabled, false);
}

QTEST_MAIN(TestPlayerStateRepository)
#include "TestPlayerStateRepository.moc"
