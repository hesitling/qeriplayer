/// @file TestSqlRowMapper.cpp
/// @brief Tests for SqlRowMapper Song↔QVariant and JSON serialization

#include "core/database/DatabaseManager.h"
#include "domain/PersistedPlayerState.h"
#include "domain/PlaylistSummary.h"
#include "domain/Song.h"
#include "repo/SqlRowMapper.h"

#include <QJsonDocument>
#include <QTest>

using namespace QeriPlayerQt;

class TestSqlRowMapper : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void toSong_mapsAllColumns();
    void songToInsertParams_count();
    void songToUpdateParams_count();
    void roundTrip_songThroughDB();
    void roundTrip_songWithLyrics();
    void roundTrip_songWithExtra();
    void toPlaylistSummary_mapsColumns();
    void playerStateJson_roundTrip();
    void toSong_emptyRow_returnsDefaults();
};

static Song createFullSong()
{
    Song song;
    song.id = "test-id-123";
    song.platform = MusicPlatform::NetEase;
    song.name = "Test Song";
    song.artist = "Test Artist";
    song.album = "Test Album";
    song.albumId = "album-456";
    song.durationMs = 180000;
    song.coverUrl = QUrl("https://cover.example.com/img.jpg");
    song.mediaUri = QUrl("https://media.example.com/play.mp3");
    song.customName = "Custom Name";
    song.customArtist = "Custom Artist";
    song.customCoverUrl = "https://custom.cover/url";
    song.originalName = "Original Name";
    song.originalArtist = "Original Artist";
    song.originalCoverUrl = "https://original.cover/url";
    song.localFileName = "song.mp3";
    song.localFilePath = "/path/to/song.mp3";
    song.matchedLyricSource = MusicPlatform::NetEase;
    song.matchedSongId = "matched-789";
    song.userLyricOffsetMs = 500;
    song.channelId = "ch-1";
    song.audioId = "audio-2";
    song.subAudioId = "sub-3";
    return song;
}

void TestSqlRowMapper::toSong_mapsAllColumns()
{
    // Insert a song via SQL and read it back through toSong()
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    Song original = createFullSong();
    auto params = SqlRowMapper::songToInsertParams(original);
    db.exec("INSERT INTO songs_cache (id, platform, name, artist, album, album_id, duration_ms, "
            "cover_url, media_uri, custom_name, custom_artist, custom_cover_url, "
            "original_name, original_artist, original_cover_url, "
            "local_file_name, local_file_path, matched_lyric_source, matched_song_id, "
            "user_lyric_offset_ms, lyrics_json, channel_id, audio_id, sub_audio_id, extra_json) "
            "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
            params);

    auto rows = db.exec("SELECT * FROM songs_cache WHERE id = ?", {original.id});
    QCOMPARE(rows.size(), 1);

    Song loaded = SqlRowMapper::toSong(rows[0]);
    QCOMPARE(loaded.id, original.id);
    QCOMPARE(loaded.name, original.name);
    QCOMPARE(loaded.artist, original.artist);
    QCOMPARE(loaded.album, original.album);
    QCOMPARE(loaded.albumId, original.albumId);
    QCOMPARE(loaded.durationMs, original.durationMs);
    QCOMPARE(loaded.coverUrl, original.coverUrl);
    QCOMPARE(loaded.mediaUri, original.mediaUri);
    QCOMPARE(loaded.platform, original.platform);
    QCOMPARE(loaded.customName, original.customName);
    QCOMPARE(loaded.customArtist, original.customArtist);
    QCOMPARE(loaded.customCoverUrl, original.customCoverUrl);
    QCOMPARE(loaded.originalName, original.originalName);
    QCOMPARE(loaded.originalArtist, original.originalArtist);
    QCOMPARE(loaded.originalCoverUrl, original.originalCoverUrl);
    QCOMPARE(loaded.localFileName, original.localFileName);
    QCOMPARE(loaded.localFilePath, original.localFilePath);
    QCOMPARE(loaded.matchedLyricSource, original.matchedLyricSource);
    QCOMPARE(loaded.matchedSongId, original.matchedSongId);
    QCOMPARE(loaded.userLyricOffsetMs, original.userLyricOffsetMs);
    QCOMPARE(loaded.channelId, original.channelId);
    QCOMPARE(loaded.audioId, original.audioId);
    QCOMPARE(loaded.subAudioId, original.subAudioId);

    db.close();
}

void TestSqlRowMapper::songToInsertParams_count()
{
    Song song = createFullSong();
    auto params = SqlRowMapper::songToInsertParams(song);
    // 25 columns (id through extra_json, excluding cached_at and last_played_at which have defaults)
    QCOMPARE(params.size(), 25);
}

void TestSqlRowMapper::songToUpdateParams_count()
{
    Song song = createFullSong();
    auto params = SqlRowMapper::songToUpdateParams(song);
    // UPDATE params: all columns except id, plus id at the end for WHERE clause
    QCOMPARE(params.size(), 25);
}

void TestSqlRowMapper::roundTrip_songThroughDB()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    Song original = createFullSong();
    auto params = SqlRowMapper::songToInsertParams(original);
    db.exec("INSERT INTO songs_cache (id, platform, name, artist, album, album_id, duration_ms, "
            "cover_url, media_uri, custom_name, custom_artist, custom_cover_url, "
            "original_name, original_artist, original_cover_url, "
            "local_file_name, local_file_path, matched_lyric_source, matched_song_id, "
            "user_lyric_offset_ms, lyrics_json, channel_id, audio_id, sub_audio_id, extra_json) "
            "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
            params);

    auto rows = db.exec("SELECT * FROM songs_cache WHERE id = ?", {original.id});
    QVERIFY(!rows.isEmpty());
    Song loaded = SqlRowMapper::toSong(rows[0]);

    QCOMPARE(loaded.id, original.id);
    QCOMPARE(loaded.name, original.name);
    QCOMPARE(loaded.artist, original.artist);
    QCOMPARE(loaded.durationMs, original.durationMs);
    QCOMPARE(loaded.platform, original.platform);

    db.close();
}

void TestSqlRowMapper::roundTrip_songWithLyrics()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    Song song = createFullSong();
    Lyrics lyrics;
    lyrics.rawText = "[00:01.00]Hello world\n[00:05.00]Second line";
    LyricLine line1;
    line1.startTimeMs = 1000;
    line1.endTimeMs = 5000;
    line1.text = "Hello world";
    LyricWord w1;
    w1.text = "Hello";
    w1.startTimeMs = 1000;
    w1.endTimeMs = 2500;
    LyricWord w2;
    w2.text = "world";
    w2.startTimeMs = 2500;
    w2.endTimeMs = 5000;
    line1.words = {w1, w2};
    LyricLine line2;
    line2.startTimeMs = 5000;
    line2.endTimeMs = 10000;
    line2.text = "Second line";
    lyrics.lines = {line1, line2};
    song.lyrics = lyrics;

    auto params = SqlRowMapper::songToInsertParams(song);
    db.exec("INSERT INTO songs_cache (id, platform, name, artist, album, album_id, duration_ms, "
            "cover_url, media_uri, custom_name, custom_artist, custom_cover_url, "
            "original_name, original_artist, original_cover_url, "
            "local_file_name, local_file_path, matched_lyric_source, matched_song_id, "
            "user_lyric_offset_ms, lyrics_json, channel_id, audio_id, sub_audio_id, extra_json) "
            "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
            params);

    auto rows = db.exec("SELECT * FROM songs_cache WHERE id = ?", {song.id});
    QVERIFY(!rows.isEmpty());
    Song loaded = SqlRowMapper::toSong(rows[0]);

    QCOMPARE(loaded.lyrics.rawText, lyrics.rawText);
    QCOMPARE(loaded.lyrics.lines.size(), 2);
    QCOMPARE(loaded.lyrics.lines[0].text, QStringLiteral("Hello world"));
    QCOMPARE(loaded.lyrics.lines[0].startTimeMs, 1000);
    QCOMPARE(loaded.lyrics.lines[0].words.size(), 2);
    QCOMPARE(loaded.lyrics.lines[0].words[0].text, QStringLiteral("Hello"));
    QCOMPARE(loaded.lyrics.lines[0].words[0].startTimeMs, 1000);
    QCOMPARE(loaded.lyrics.lines[0].words[0].endTimeMs, 2500);
    QCOMPARE(loaded.lyrics.lines[0].words[1].text, QStringLiteral("world"));
    QCOMPARE(loaded.lyrics.lines[0].words[1].startTimeMs, 2500);
    QCOMPARE(loaded.lyrics.lines[1].text, QStringLiteral("Second line"));
    QCOMPARE(loaded.lyrics.lines[1].words.size(), 0);

    db.close();
}

void TestSqlRowMapper::roundTrip_songWithExtra()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    Song song = createFullSong();
    song.extra["foo"] = "bar";
    song.extra["count"] = 42;

    auto params = SqlRowMapper::songToInsertParams(song);
    db.exec("INSERT INTO songs_cache (id, platform, name, artist, album, album_id, duration_ms, "
            "cover_url, media_uri, custom_name, custom_artist, custom_cover_url, "
            "original_name, original_artist, original_cover_url, "
            "local_file_name, local_file_path, matched_lyric_source, matched_song_id, "
            "user_lyric_offset_ms, lyrics_json, channel_id, audio_id, sub_audio_id, extra_json) "
            "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
            params);

    auto rows = db.exec("SELECT * FROM songs_cache WHERE id = ?", {song.id});
    QVERIFY(!rows.isEmpty());
    Song loaded = SqlRowMapper::toSong(rows[0]);

    QCOMPARE(loaded.extra["foo"].toString(), QStringLiteral("bar"));
    QCOMPARE(loaded.extra["count"].toInt(), 42);

    db.close();
}

void TestSqlRowMapper::toPlaylistSummary_mapsColumns()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    db.exec("INSERT INTO playlists (id, name, cover_url, song_count) VALUES (?, ?, ?, ?)",
            {"pl-1", "My Playlist", "https://cover.url", 5});

    auto rows = db.exec("SELECT * FROM playlists WHERE id = 'pl-1'");
    QCOMPARE(rows.size(), 1);

    PlaylistSummary ps = SqlRowMapper::toPlaylistSummary(rows[0]);
    QCOMPARE(ps.id, QStringLiteral("pl-1"));
    QCOMPARE(ps.name, QStringLiteral("My Playlist"));
    QCOMPARE(ps.trackCount, 5);

    db.close();
}

void TestSqlRowMapper::playerStateJson_roundTrip()
{
    PersistedPlayerState state;
    state.currentIndex = 3;
    state.mediaUrl = "https://media.example.com/song.mp3";
    state.positionMs = 45000;
    state.shouldResumePlayback = true;
    state.repeatMode = RepeatMode::All;
    state.shuffleEnabled = true;

    Song song1;
    song1.id = "s1";
    song1.name = "Song 1";
    song1.artist = "Artist 1";
    song1.durationMs = 120000;
    state.playlist = {song1};

    QVariantMap json = SqlRowMapper::playerStateToJson(state);
    PersistedPlayerState restored = SqlRowMapper::playerStateFromJson(json);

    QCOMPARE(restored.currentIndex, state.currentIndex);
    QCOMPARE(restored.mediaUrl, state.mediaUrl);
    QCOMPARE(restored.positionMs, state.positionMs);
    QCOMPARE(restored.shouldResumePlayback, state.shouldResumePlayback);
    QCOMPARE(restored.repeatMode, state.repeatMode);
    QCOMPARE(restored.shuffleEnabled, state.shuffleEnabled);
    QCOMPARE(restored.playlist.size(), 1);
    QCOMPARE(restored.playlist[0].id, QStringLiteral("s1"));
    QCOMPARE(restored.playlist[0].name, QStringLiteral("Song 1"));
}

void TestSqlRowMapper::toSong_emptyRow_returnsDefaults()
{
    // A row with all empty/null values
    QueryRow row(27, QVariant());

    Song song = SqlRowMapper::toSong(row);
    QVERIFY(song.id.isEmpty());
    QCOMPARE(song.platform, MusicPlatform::Unknown);
    QCOMPARE(song.durationMs, 0);
    QCOMPARE(song.userLyricOffsetMs, 0);
}

QTEST_MAIN(TestSqlRowMapper)
#include "TestSqlRowMapper.moc"
