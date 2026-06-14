/// @file TestDomainModels.cpp
/// @brief Unit tests for domain models
/// @date 2024-01-15

#include "domain/Domain.h"

#include <QTest>
#include <QVariant>

using namespace NeriPlayerQt;

class TestDomainModels : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // MusicPlatform enum
    void musicPlatform_comparison();
    void musicPlatform_defaultIsUnknown();

    // SearchType enum
    void searchType_defaultIsSong();

    // PlaybackState enum
    void playbackState_playingNotStopped();

    // RepeatMode enum
    void repeatMode_hasThreeValues();

    // AudioQuality enum
    void audioQuality_ordering();

    // Song
    void song_defaultConstruction();
    void song_parameterizedConstruction();
    void song_copySemantics();

    // Album
    void album_defaultConstruction();

    // Artist
    void artist_parameterizedConstruction();

    // Playlist
    void playlist_withSongs();

    // Lyrics
    void lyrics_ordering();
    void lyrics_wordTiming();

    // SearchResult
    void searchResult_defaultConstruction();

    // Q_DECLARE_METATYPE
    void song_metatype();
};

void TestDomainModels::musicPlatform_comparison()
{
    QCOMPARE(MusicPlatform::NetEase, MusicPlatform::NetEase);
    QVERIFY(MusicPlatform::NetEase != MusicPlatform::Bilibili);
    QVERIFY(MusicPlatform::Unknown != MusicPlatform::YouTube);
}

void TestDomainModels::musicPlatform_defaultIsUnknown()
{
    MusicPlatform p { };
    QCOMPARE(p, MusicPlatform::Unknown);
}

void TestDomainModels::searchType_defaultIsSong()
{
    SearchType t { };
    QCOMPARE(t, SearchType::Song); // default-constructed enum is 0 = Song
}

void TestDomainModels::playbackState_playingNotStopped()
{
    PlaybackState s = PlaybackState::Playing;
    QVERIFY(s != PlaybackState::Stopped);
}

void TestDomainModels::repeatMode_hasThreeValues()
{
    QCOMPARE(static_cast<int>(RepeatMode::Off), 0);
    QCOMPARE(static_cast<int>(RepeatMode::One), 1);
    QCOMPARE(static_cast<int>(RepeatMode::All), 2);
}

void TestDomainModels::audioQuality_ordering()
{
    QVERIFY(AudioQuality::Lossless > AudioQuality::High);
    QVERIFY(AudioQuality::High > AudioQuality::Standard);
    QVERIFY(AudioQuality::Standard > AudioQuality::Low);
}

void TestDomainModels::song_defaultConstruction()
{
    Song s;
    QVERIFY(s.id.isEmpty());
    QVERIFY(s.name.isEmpty());
    QCOMPARE(s.durationMs, 0);
    QCOMPARE(s.platform, MusicPlatform::Unknown);
}

void TestDomainModels::song_parameterizedConstruction()
{
    Song s;
    s.id = QStringLiteral("1");
    s.name = QStringLiteral("Test");
    s.artist = QStringLiteral("Artist");
    s.album = QStringLiteral("Album");
    s.durationMs = 180000;

    QCOMPARE(s.id, QStringLiteral("1"));
    QCOMPARE(s.name, QStringLiteral("Test"));
    QCOMPARE(s.artist, QStringLiteral("Artist"));
    QCOMPARE(s.album, QStringLiteral("Album"));
    QCOMPARE(s.durationMs, 180000);
    QCOMPARE(s.platform, MusicPlatform::Unknown);
}

void TestDomainModels::song_copySemantics()
{
    Song s;
    s.id = QStringLiteral("1");
    s.name = QStringLiteral("Test");
    s.artist = QStringLiteral("Artist");
    s.durationMs = 180000;

    Song copy = s;
    QCOMPARE(copy.id, s.id);
    QCOMPARE(copy.name, s.name);
    QCOMPARE(copy.artist, s.artist);
    QCOMPARE(copy.durationMs, s.durationMs);
    QCOMPARE(copy.platform, s.platform);
}

void TestDomainModels::album_defaultConstruction()
{
    Album a;
    QCOMPARE(a.size, 0);
    QCOMPARE(a.platform, MusicPlatform::Unknown);
}

void TestDomainModels::artist_parameterizedConstruction()
{
    Artist a;
    a.id = QStringLiteral("a1");
    a.name = QStringLiteral("Some Artist");

    QCOMPARE(a.id, QStringLiteral("a1"));
    QCOMPARE(a.name, QStringLiteral("Some Artist"));
}

void TestDomainModels::playlist_withSongs()
{
    Song s1, s2, s3;
    s1.id = QStringLiteral("1");
    s2.id = QStringLiteral("2");
    s3.id = QStringLiteral("3");

    Playlist p;
    p.name = QStringLiteral("My Playlist");
    p.songs = { s1, s2, s3 };
    p.songCount = p.songs.size();

    QCOMPARE(p.songCount, 3);
    QCOMPARE(p.songs.size(), 3);
    QCOMPARE(p.songs[0].id, QStringLiteral("1"));
    QCOMPARE(p.songs[2].id, QStringLiteral("3"));
}

void TestDomainModels::lyrics_ordering()
{
    Lyrics lyrics;
    lyrics.rawText = QStringLiteral("[00:00.000]Hello\n[00:05.000]World");
    lyrics.lines = { { 0, 5000, QStringLiteral("Hello"), { } }, { 5000, 10000, QStringLiteral("World"), { } } };

    QCOMPARE(lyrics.lines[0].startTimeMs, 0);
    QCOMPARE(lyrics.lines[0].endTimeMs, 5000);
    QCOMPARE(lyrics.lines[0].text, QStringLiteral("Hello"));
    QCOMPARE(lyrics.lines[1].startTimeMs, 5000);
    QCOMPARE(lyrics.lines[1].text, QStringLiteral("World"));
}

void TestDomainModels::lyrics_wordTiming()
{
    Lyrics lyrics;
    LyricLine line;
    line.startTimeMs = 1000;
    line.endTimeMs = 3000;
    line.text = QStringLiteral("Hello World");
    line.words = { { QStringLiteral("Hello"), 1000, 2000 }, { QStringLiteral("World"), 2000, 3000 } };
    lyrics.lines.append(line);

    QCOMPARE(lyrics.lines[0].words.size(), 2);
    QCOMPARE(lyrics.lines[0].words[0].text, QStringLiteral("Hello"));
    QCOMPARE(lyrics.lines[0].words[0].startTimeMs, 1000);
    QCOMPARE(lyrics.lines[0].words[0].endTimeMs, 2000);
    QCOMPARE(lyrics.lines[0].words[1].text, QStringLiteral("World"));
    QCOMPARE(lyrics.lines[0].words[1].startTimeMs, 2000);
}

void TestDomainModels::searchResult_defaultConstruction()
{
    SearchResult r;
    QVERIFY(r.songs.isEmpty());
    QVERIFY(r.albums.isEmpty());
    QVERIFY(r.artists.isEmpty());
    QVERIFY(r.playlists.isEmpty());
    QCOMPARE(r.totalCount, 0);
    QCOMPARE(r.hasMore, false);
}

void TestDomainModels::song_metatype()
{
    Song s;
    s.id = QStringLiteral("1");
    QVariant v = QVariant::fromValue(s);
    Song retrieved = v.value<Song>();
    QCOMPARE(retrieved.id, QStringLiteral("1"));
}

QTEST_MAIN(TestDomainModels)
#include "TestDomainModels.moc"
