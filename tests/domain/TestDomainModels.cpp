/// @file TestDomainModels.cpp
/// @brief Unit tests for domain models

#include "domain/Domain.h"
#include "domain/ListenTogether.h"

#include <QTest>
#include <QVariant>

using namespace QeriPlayerQt;

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

    // PlaybackAudioSource enum
    void playbackAudioSource_defaultIsLocal();

    // BiliPlaylistKind enum
    void biliPlaylistKind_distinctValues();

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

    // SongIdentity
    void songIdentity_stableKey();
    void songIdentity_equality();

    // SongUrlResult
    void songUrlResult_defaultIsFailure();
    void songUrlResult_success();

    // PlaylistSummary / AlbumSummary / BiliPlaylist
    void playlistSummary_defaultConstruction();
    void albumSummary_defaultConstruction();
    void biliPlaylist_defaultConstruction();
    void playlistSummary_qmlProperties();
    void albumSummary_qmlProperties();

    // PersistedPlayerState
    void persistedPlayerState_defaultConstruction();

    // Listen Together
    void listenTogether_stableKey_netease();
    void listenTogether_stableKey_bilibili();
    void listenTogether_stableKey_youtube();
    void listenTogether_songToTrack();
    void listenTogether_trackToSong();
    void listenTogether_roundTrip();

    // Q_DECLARE_METATYPE
    void song_metatype();
};

// --- Enum tests ---

void TestDomainModels::musicPlatform_comparison()
{
    QCOMPARE(MusicPlatform::NetEase, MusicPlatform::NetEase);
    QVERIFY(MusicPlatform::NetEase != MusicPlatform::Bilibili);
    QVERIFY(MusicPlatform::Unknown != MusicPlatform::YouTube);
}

void TestDomainModels::musicPlatform_defaultIsUnknown()
{
    MusicPlatform p {};
    QCOMPARE(p, MusicPlatform::Unknown);
}

void TestDomainModels::searchType_defaultIsSong()
{
    SearchType t {};
    QCOMPARE(t, SearchType::Song);
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

void TestDomainModels::playbackAudioSource_defaultIsLocal()
{
    PlaybackAudioSource src {};
    QCOMPARE(src, PlaybackAudioSource::Local);
}

void TestDomainModels::biliPlaylistKind_distinctValues()
{
    QVERIFY(BiliPlaylistKind::CreatedFavorite != BiliPlaylistKind::CollectedFavorite);
    QVERIFY(BiliPlaylistKind::CollectedFavorite != BiliPlaylistKind::Collection);
    QVERIFY(BiliPlaylistKind::CreatedFavorite != BiliPlaylistKind::Collection);
}

// --- Song tests ---

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

// --- Album / Artist / Playlist ---

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
    p.songs = {s1, s2, s3};
    p.songCount = p.songs.size();

    QCOMPARE(p.songCount, 3);
    QCOMPARE(p.songs.size(), 3);
    QCOMPARE(p.songs[0].id, QStringLiteral("1"));
    QCOMPARE(p.songs[2].id, QStringLiteral("3"));
}

// --- Lyrics ---

void TestDomainModels::lyrics_ordering()
{
    Lyrics lyrics;
    lyrics.rawText = QStringLiteral("[00:00.000]Hello\n[00:05.000]World");
    lyrics.lines = {{0, 5000, QStringLiteral("Hello"), {}}, {5000, 10000, QStringLiteral("World"), {}}};

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
    line.words = {{QStringLiteral("Hello"), 1000, 2000}, {QStringLiteral("World"), 2000, 3000}};
    lyrics.lines.append(line);

    QCOMPARE(lyrics.lines[0].words.size(), 2);
    QCOMPARE(lyrics.lines[0].words[0].text, QStringLiteral("Hello"));
    QCOMPARE(lyrics.lines[0].words[0].startTimeMs, 1000);
    QCOMPARE(lyrics.lines[0].words[0].endTimeMs, 2000);
    QCOMPARE(lyrics.lines[0].words[1].text, QStringLiteral("World"));
    QCOMPARE(lyrics.lines[0].words[1].startTimeMs, 2000);
}

// --- SearchResult ---

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

// --- SongIdentity ---

void TestDomainModels::songIdentity_stableKey()
{
    SongIdentity id1 {QStringLiteral("1"), QStringLiteral("Album"), QStringLiteral("http://example.com")};
    SongIdentity id2 {QStringLiteral("1"), QStringLiteral("Album"), QStringLiteral("http://example.com")};
    QCOMPARE(id1.stableKey(), id2.stableKey());
    QVERIFY(!id1.stableKey().isEmpty());
}

void TestDomainModels::songIdentity_equality()
{
    SongIdentity a {QStringLiteral("1"), QStringLiteral("Album"), QStringLiteral("http://example.com")};
    SongIdentity b {QStringLiteral("1"), QStringLiteral("Album"), QStringLiteral("http://example.com")};
    SongIdentity c {QStringLiteral("2"), QStringLiteral("Album"), QStringLiteral("http://example.com")};
    QCOMPARE(a, b);
    QVERIFY(a != c);
}

// --- SongUrlResult ---

void TestDomainModels::songUrlResult_defaultIsFailure()
{
    SongUrlResult r;
    QCOMPARE(r.status, SongUrlResult::Status::Failure);
    QVERIFY(r.url.isEmpty());
}

void TestDomainModels::songUrlResult_success()
{
    SongUrlResult r;
    r.status = SongUrlResult::Status::Success;
    r.url = QStringLiteral("http://example.com/stream");
    r.durationMs = 180000;
    QCOMPARE(r.status, SongUrlResult::Status::Success);
    QCOMPARE(r.url, QStringLiteral("http://example.com/stream"));
    QCOMPARE(r.durationMs, 180000);
}

// --- Summary models ---

void TestDomainModels::playlistSummary_defaultConstruction()
{
    PlaylistSummary ps;
    QCOMPARE(ps.playCount, 0);
    QCOMPARE(ps.trackCount, 0);
}

void TestDomainModels::albumSummary_defaultConstruction()
{
    AlbumSummary as;
    QCOMPARE(as.size, 0);
}

void TestDomainModels::biliPlaylist_defaultConstruction()
{
    BiliPlaylist bp;
    QCOMPARE(bp.kind, BiliPlaylistKind::CreatedFavorite);
    QCOMPARE(bp.count, 0);
}

void TestDomainModels::playlistSummary_qmlProperties()
{
    PlaylistSummary summary;
    summary.id = QStringLiteral("pl-1");
    summary.name = QStringLiteral("Road Trip");
    summary.coverUrl = QUrl(QStringLiteral("https://example.com/cover.jpg"));
    summary.trackCount = 12;

    QVariant variant = QVariant::fromValue(summary);
    const PlaylistSummary restored = variant.value<PlaylistSummary>();
    QCOMPARE(restored.id, QStringLiteral("pl-1"));
    QCOMPARE(restored.name, QStringLiteral("Road Trip"));
    QCOMPARE(restored.coverUrl, QUrl(QStringLiteral("https://example.com/cover.jpg")));
    QCOMPARE(restored.trackCount, 12);

    const QMetaObject *metaObject = QMetaType::fromType<PlaylistSummary>().metaObject();
    QVERIFY(metaObject != nullptr);
    QVERIFY(metaObject->indexOfProperty("id") >= 0);
    QVERIFY(metaObject->indexOfProperty("name") >= 0);
    QVERIFY(metaObject->indexOfProperty("coverUrl") >= 0);
    QVERIFY(metaObject->indexOfProperty("playCount") >= 0);
    QVERIFY(metaObject->indexOfProperty("trackCount") >= 0);
}

void TestDomainModels::albumSummary_qmlProperties()
{
    AlbumSummary summary;
    summary.id = QStringLiteral("al-1");
    summary.name = QStringLiteral("Favorites");
    summary.coverUrl = QUrl(QStringLiteral("https://example.com/album.jpg"));
    summary.size = 8;

    QVariant variant = QVariant::fromValue(summary);
    const AlbumSummary restored = variant.value<AlbumSummary>();
    QCOMPARE(restored.id, QStringLiteral("al-1"));
    QCOMPARE(restored.name, QStringLiteral("Favorites"));
    QCOMPARE(restored.coverUrl, QUrl(QStringLiteral("https://example.com/album.jpg")));
    QCOMPARE(restored.size, 8);

    const QMetaObject *metaObject = QMetaType::fromType<AlbumSummary>().metaObject();
    QVERIFY(metaObject != nullptr);
    QVERIFY(metaObject->indexOfProperty("id") >= 0);
    QVERIFY(metaObject->indexOfProperty("name") >= 0);
    QVERIFY(metaObject->indexOfProperty("coverUrl") >= 0);
    QVERIFY(metaObject->indexOfProperty("size") >= 0);
}

// --- PersistedPlayerState ---

void TestDomainModels::persistedPlayerState_defaultConstruction()
{
    PersistedPlayerState ps;
    QCOMPARE(ps.currentIndex, 0);
    QCOMPARE(ps.positionMs, 0);
    QCOMPARE(ps.shouldResumePlayback, false);
    QCOMPARE(ps.repeatMode, RepeatMode::Off);
    QCOMPARE(ps.shuffleEnabled, false);
}

// --- Listen Together tests ---

void TestDomainModels::listenTogether_stableKey_netease()
{
    // NetEase: channelId:audioId
    QString key = buildStableTrackKey(QStringLiteral("netease"), QStringLiteral("12345"));
    QCOMPARE(key, QStringLiteral("netease:12345"));
}

void TestDomainModels::listenTogether_stableKey_bilibili()
{
    // Bilibili with subAudioId: channelId:audioId:subAudioId
    QString key = buildStableTrackKey(QStringLiteral("bilibili"), QStringLiteral("67890"), QStringLiteral("p1"));
    QCOMPARE(key, QStringLiteral("bilibili:67890:p1"));

    // Bilibili without subAudioId
    QString key2 = buildStableTrackKey(QStringLiteral("bilibili"), QStringLiteral("67890"));
    QCOMPARE(key2, QStringLiteral("bilibili:67890"));
}

void TestDomainModels::listenTogether_stableKey_youtube()
{
    // YouTube with playlistContextId: channelId:audioId:playlistContextId
    QString key
        = buildStableTrackKey(QStringLiteral("youtubeMusic"), QStringLiteral("abc"), {}, QStringLiteral("PLxyz"));
    QCOMPARE(key, QStringLiteral("youtubeMusic:abc:PLxyz"));

    // YouTube without playlistContextId
    QString key2 = buildStableTrackKey(QStringLiteral("youtubeMusic"), QStringLiteral("abc"));
    QCOMPARE(key2, QStringLiteral("youtubeMusic:abc"));
}

void TestDomainModels::listenTogether_songToTrack()
{
    Song song;
    song.id = QStringLiteral("42");
    song.name = QStringLiteral("Test Song");
    song.artist = QStringLiteral("Artist");
    song.album = QStringLiteral("Album");
    song.durationMs = 180000;
    song.channelId = QStringLiteral("netease");
    song.audioId = QStringLiteral("42");
    song.mediaUri = QUrl(QStringLiteral("http://example.com/stream"));

    ListenTogetherTrack track = songToTrack(song);
    QCOMPARE(track.name, QStringLiteral("Test Song"));
    QCOMPARE(track.artist, QStringLiteral("Artist"));
    QCOMPARE(track.album, QStringLiteral("Album"));
    QCOMPARE(track.durationMs, 180000);
    QCOMPARE(track.channelId, QStringLiteral("netease"));
    QCOMPARE(track.audioId, QStringLiteral("42"));
    QCOMPARE(track.stableKey, QStringLiteral("netease:42"));
}

void TestDomainModels::listenTogether_trackToSong()
{
    ListenTogetherTrack track;
    track.stableKey = QStringLiteral("netease:99");
    track.channelId = QStringLiteral("netease");
    track.audioId = QStringLiteral("99");
    track.name = QStringLiteral("Converted");
    track.artist = QStringLiteral("Some Artist");
    track.album = QStringLiteral("Some Album");
    track.durationMs = 240000;
    track.mediaUri = QStringLiteral("http://example.com/media");

    Song song = trackToSong(track);
    QCOMPARE(song.name, QStringLiteral("Converted"));
    QCOMPARE(song.artist, QStringLiteral("Some Artist"));
    QCOMPARE(song.album, QStringLiteral("Some Album"));
    QCOMPARE(song.durationMs, 240000);
    QCOMPARE(song.channelId, QStringLiteral("netease"));
    QCOMPARE(song.audioId, QStringLiteral("99"));
}

void TestDomainModels::listenTogether_roundTrip()
{
    // Build a Song, convert to track, convert back, verify key fields match
    Song original;
    original.id = QStringLiteral("100");
    original.name = QStringLiteral("Round Trip");
    original.artist = QStringLiteral("RT Artist");
    original.album = QStringLiteral("RT Album");
    original.durationMs = 300000;
    original.channelId = QStringLiteral("netease");
    original.audioId = QStringLiteral("100");
    original.mediaUri = QUrl(QStringLiteral("http://example.com/rt"));

    ListenTogetherTrack track = songToTrack(original);
    Song restored = trackToSong(track);

    QCOMPARE(restored.name, original.name);
    QCOMPARE(restored.artist, original.artist);
    QCOMPARE(restored.album, original.album);
    QCOMPARE(restored.durationMs, original.durationMs);
    QCOMPARE(restored.channelId, original.channelId);
    QCOMPARE(restored.audioId, original.audioId);
}

// --- Metatype ---

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
