/// @file TestNeteaseParser.cpp
/// @brief Unit tests for NeteaseParser using recorded JSON fixtures

#include "api/netease/NeteaseParser.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

using namespace QeriPlayerQt;

static QJsonObject loadFixture(const QString &name)
{
    QFile file(QStringLiteral(FIXTURES_DIR "/") + name);
    if (!file.open(QIODevice::ReadOnly)) {
        qFatal("Cannot open fixture: %s", qPrintable(file.fileName()));
    }
    return QJsonDocument::fromJson(file.readAll()).object();
}

class TestNeteaseParser : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void parseSong_validJson();
    void parseSong_emptyId();
    void parseSong_multipleArtists();
    void parseSongs_array();
    void parseAlbum_validJson();
    void parseAlbum_withSongs();
    void parseArtist_validJson();
    void parsePlaylist_validJson();
    void parsePlaylistDetail_withTracks();
    void parseLyrics_lrcFormat();
    void parseSearchResult_songs();
    void parseSearchResult_playlists();
    void parseSearchResult_albums();
    void parseSearchResult_artists();
    void parseLoginResult_validJson();
    void parseSongUrl_success();
    void parseSongUrl_requiresLogin();
};

void TestNeteaseParser::parseSong_validJson()
{
    QJsonObject json = loadFixture(QStringLiteral("song_detail.json"));
    Song song = NeteaseParser::parseSong(json);

    QCOMPARE(song.id, QStringLiteral("12345"));
    QCOMPARE(song.name, QStringLiteral("Test Song"));
    QCOMPARE(song.artist, QStringLiteral("Test Artist"));
    QCOMPARE(song.album, QStringLiteral("Test Album"));
    QCOMPARE(song.albumId, QStringLiteral("200"));
    QCOMPARE(song.durationMs, 240000);
    QCOMPARE(song.coverUrl, QUrl("https://example.com/cover.jpg"));
    QCOMPARE(song.platform, MusicPlatform::NetEase);
}

void TestNeteaseParser::parseSong_emptyId()
{
    QJsonObject json;
    json[QLatin1String("id")] = 0;
    json[QLatin1String("name")] = QStringLiteral("Empty ID Song");

    // Should not throw, but log warning
    Song song = NeteaseParser::parseSong(json);
    QCOMPARE(song.name, QStringLiteral("Empty ID Song"));
}

void TestNeteaseParser::parseSong_multipleArtists()
{
    QJsonObject json;
    json[QLatin1String("id")] = 99999;
    json[QLatin1String("name")] = QStringLiteral("Multi Artist Song");

    QJsonArray artists;
    QJsonObject a1;
    a1[QLatin1String("id")] = 1;
    a1[QLatin1String("name")] = QStringLiteral("Artist One");
    artists.append(a1);
    QJsonObject a2;
    a2[QLatin1String("id")] = 2;
    a2[QLatin1String("name")] = QStringLiteral("Artist Two");
    artists.append(a2);
    json[QLatin1String("ar")] = artists;

    QJsonObject al;
    al[QLatin1String("id")] = 100;
    al[QLatin1String("name")] = QStringLiteral("Test Album");
    al[QLatin1String("picUrl")] = QStringLiteral("https://example.com/pic.jpg");
    json[QLatin1String("al")] = al;
    json[QLatin1String("dt")] = 180000;

    Song song = NeteaseParser::parseSong(json);
    QCOMPARE(song.artist, QStringLiteral("Artist One, Artist Two"));
}

void TestNeteaseParser::parseSongs_array()
{
    QJsonObject json = loadFixture(QStringLiteral("search_songs.json"));
    QJsonArray array = json[QLatin1String("result")].toObject()[QLatin1String("songs")].toArray();

    QVector<Song> songs = NeteaseParser::parseSongs(array);
    QCOMPARE(songs.size(), 2);
    QCOMPARE(songs[0].name, QStringLiteral("Search Result Song 1"));
    QCOMPARE(songs[1].name, QStringLiteral("Search Result Song 2"));
    QCOMPARE(songs[1].artist, QStringLiteral("Artist B, Artist C"));
}

void TestNeteaseParser::parseAlbum_validJson()
{
    QJsonObject json = loadFixture(QStringLiteral("album_detail.json"));
    Album album = NeteaseParser::parseAlbum(json);

    QCOMPARE(album.id, QStringLiteral("500"));
    QCOMPARE(album.name, QStringLiteral("Test Album Full"));
    QCOMPARE(album.artist, QStringLiteral("Album Artist"));
    QCOMPARE(album.size, 10);
    QCOMPARE(album.platform, MusicPlatform::NetEase);
}

void TestNeteaseParser::parseAlbum_withSongs()
{
    QJsonObject json = loadFixture(QStringLiteral("album_detail.json"));
    QVector<Song> songs;
    Album album = NeteaseParser::parseAlbumWithSongs(json, songs);

    QCOMPARE(album.id, QStringLiteral("500"));
    QCOMPARE(songs.size(), 2);
    QCOMPARE(songs[0].name, QStringLiteral("Album Track 1"));
    QCOMPARE(songs[1].name, QStringLiteral("Album Track 2"));
}

void TestNeteaseParser::parseArtist_validJson()
{
    QJsonObject json = loadFixture(QStringLiteral("artist_detail.json"));
    Artist artist = NeteaseParser::parseArtist(json);

    QCOMPARE(artist.id, QStringLiteral("700"));
    QCOMPARE(artist.name, QStringLiteral("Test Artist"));
    QCOMPARE(artist.avatarUrl, QUrl("https://example.com/artist.jpg"));
    QCOMPARE(artist.description, QStringLiteral("A test artist for unit testing"));
    QCOMPARE(artist.platform, MusicPlatform::NetEase);
}

void TestNeteaseParser::parsePlaylist_validJson()
{
    QJsonObject json = loadFixture(QStringLiteral("playlist_detail.json"));
    Playlist playlist = NeteaseParser::parsePlaylist(json);

    QCOMPARE(playlist.id, QStringLiteral("800"));
    QCOMPARE(playlist.name, QStringLiteral("Test Playlist"));
    QCOMPARE(playlist.description, QStringLiteral("A test playlist"));
    QCOMPARE(playlist.coverUrl, QUrl("https://example.com/playlist.jpg"));
    QCOMPARE(playlist.songCount, 2);
    QCOMPARE(playlist.owner, QStringLiteral("TestUser"));
    QCOMPARE(playlist.modifiedAt, 1700000000000LL);
    QCOMPARE(playlist.platform, MusicPlatform::NetEase);
    QVERIFY(playlist.songs.isEmpty()); // parsePlaylist doesn't populate songs
}

void TestNeteaseParser::parsePlaylistDetail_withTracks()
{
    QJsonObject json = loadFixture(QStringLiteral("playlist_detail.json"));
    Playlist playlist = NeteaseParser::parsePlaylistDetail(json);

    QCOMPARE(playlist.id, QStringLiteral("800"));
    QCOMPARE(playlist.songs.size(), 2);
    QCOMPARE(playlist.songs[0].name, QStringLiteral("Playlist Song 1"));
    QCOMPARE(playlist.songs[1].name, QStringLiteral("Playlist Song 2"));
}

void TestNeteaseParser::parseLyrics_lrcFormat()
{
    QJsonObject json = loadFixture(QStringLiteral("lyrics.json"));
    Lyrics lyrics = NeteaseParser::parseLyrics(json);

    QVERIFY(!lyrics.rawText.isEmpty());
    QCOMPARE(lyrics.lines.size(), 5); // Header line + 4 lyric lines

    QCOMPARE(lyrics.lines[0].text, QStringLiteral("Test Song - Test Artist"));
    QCOMPARE(lyrics.lines[0].startTimeMs, 0);

    QCOMPARE(lyrics.lines[1].text, QStringLiteral("First line of lyrics"));
    QCOMPARE(lyrics.lines[1].startTimeMs, 5000);

    QCOMPARE(lyrics.lines[2].text, QStringLiteral("Second line of lyrics"));
    QCOMPARE(lyrics.lines[2].startTimeMs, 10500);

    QCOMPARE(lyrics.lines[3].text, QStringLiteral("Third line"));
    QCOMPARE(lyrics.lines[3].startTimeMs, 15000);

    QCOMPARE(lyrics.lines[4].text, QStringLiteral("Fourth line"));
    QCOMPARE(lyrics.lines[4].startTimeMs, 20000);

    // End times
    QCOMPARE(lyrics.lines[0].endTimeMs, 5000);
    QCOMPARE(lyrics.lines[4].endTimeMs, 25000); // Last line + 5s
}

void TestNeteaseParser::parseSearchResult_songs()
{
    QJsonObject json = loadFixture(QStringLiteral("search_songs.json"));
    SearchResult result = NeteaseParser::parseSearchResult(json, SearchType::Song);

    QCOMPARE(result.songs.size(), 2);
    QCOMPARE(result.totalCount, 2);
    QCOMPARE(result.albums.size(), 0);
    QCOMPARE(result.artists.size(), 0);
    QCOMPARE(result.playlists.size(), 0);
}

void TestNeteaseParser::parseSearchResult_playlists()
{
    QJsonObject json;
    QJsonObject resultObj;
    QJsonArray playlists;

    QJsonObject pl;
    pl[QLatin1String("id")] = 999;
    pl[QLatin1String("name")] = QStringLiteral("Search Playlist");
    pl[QLatin1String("coverImgUrl")] = QStringLiteral("https://example.com/sp.jpg");
    pl[QLatin1String("trackCount")] = 5;
    playlists.append(pl);

    resultObj[QLatin1String("playlists")] = playlists;
    resultObj[QLatin1String("playlistCount")] = 1;
    json[QLatin1String("result")] = resultObj;

    SearchResult result = NeteaseParser::parseSearchResult(json, SearchType::Playlist);
    QCOMPARE(result.playlists.size(), 1);
    QCOMPARE(result.playlists[0].name, QStringLiteral("Search Playlist"));
    QCOMPARE(result.totalCount, 1);
}

void TestNeteaseParser::parseSearchResult_albums()
{
    QJsonObject json;
    QJsonObject resultObj;
    QJsonArray albums;

    QJsonObject al;
    al[QLatin1String("id")] = 888;
    al[QLatin1String("name")] = QStringLiteral("Search Album");
    al[QLatin1String("artist")] = QJsonObject {{QLatin1String("name"), QStringLiteral("Album Artist")}};
    al[QLatin1String("picUrl")] = QStringLiteral("https://example.com/sa.jpg");
    al[QLatin1String("size")] = 12;
    albums.append(al);

    resultObj[QLatin1String("albums")] = albums;
    resultObj[QLatin1String("albumCount")] = 1;
    json[QLatin1String("result")] = resultObj;

    SearchResult result = NeteaseParser::parseSearchResult(json, SearchType::Album);
    QCOMPARE(result.albums.size(), 1);
    QCOMPARE(result.albums[0].name, QStringLiteral("Search Album"));
}

void TestNeteaseParser::parseSearchResult_artists()
{
    QJsonObject json;
    QJsonObject resultObj;
    QJsonArray artists;

    QJsonObject ar;
    ar[QLatin1String("id")] = 777;
    ar[QLatin1String("name")] = QStringLiteral("Search Artist");
    ar[QLatin1String("picUrl")] = QStringLiteral("https://example.com/sar.jpg");
    artists.append(ar);

    resultObj[QLatin1String("artists")] = artists;
    resultObj[QLatin1String("artistCount")] = 1;
    json[QLatin1String("result")] = resultObj;

    SearchResult result = NeteaseParser::parseSearchResult(json, SearchType::Artist);
    QCOMPARE(result.artists.size(), 1);
    QCOMPARE(result.artists[0].name, QStringLiteral("Search Artist"));
}

void TestNeteaseParser::parseLoginResult_validJson()
{
    QJsonObject json = loadFixture(QStringLiteral("login.json"));
    LoginResult result = NeteaseParser::parseLoginResult(json);

    QCOMPARE(result.userId, QStringLiteral("12345678"));
    QCOMPARE(result.nickname, QStringLiteral("TestUser"));
    QCOMPARE(result.avatarUrl, QUrl("https://example.com/avatar.jpg"));
    QCOMPARE(result.cookie, QStringLiteral("MUSIC_U=fake_token_for_testing"));
}

void TestNeteaseParser::parseSongUrl_success()
{
    QJsonObject json = loadFixture(QStringLiteral("song_url.json"));
    SongUrlResult result = NeteaseParser::parseSongUrl(json);

    QCOMPARE(result.status, SongUrlResult::Status::Success);
    QVERIFY(!result.url.isEmpty());
    QCOMPARE(result.durationMs, 240000);
    QCOMPARE(result.mimeType, QStringLiteral("mp3"));
    QCOMPARE(result.audioInfo.bitrateKbps, 320);
    QCOMPARE(result.expectedContentLength, 9600000);
    QCOMPARE(result.cacheKeyOverride, QStringLiteral("abc123def456"));
}

void TestNeteaseParser::parseSongUrl_requiresLogin()
{
    QJsonObject json;
    QJsonArray data;
    QJsonObject entry;
    entry[QLatin1String("id")] = 12345;
    entry[QLatin1String("url")] = QJsonValue::Null; // Null URL = requires login
    data.append(entry);
    json[QLatin1String("data")] = data;

    SongUrlResult result = NeteaseParser::parseSongUrl(json);
    QCOMPARE(result.status, SongUrlResult::Status::RequiresLogin);
    QVERIFY(result.url.isEmpty());
}

QTEST_MAIN(TestNeteaseParser)
#include "TestNeteaseParser.moc"
