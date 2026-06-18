/// @file TestNeteaseE2E.cpp
/// @brief End-to-end tests for NeteaseClient against the real NetEase API
///
/// Usage:
///   NETEASE_COOKIE="MUSIC_U=xxx; __csrf=yyy" ./TestNeteaseE2E
///
/// Without credentials, all tests are skipped.

#include "api/netease/NeteaseClient.h"
#include "api/netease/NeteaseCrypto.h"
#include "core/network/HttpClient.h"

#include <QCoreApplication>
#include <QCoroTask>
#include <QJsonArray>
#include <QJsonObject>
#include <QTest>
#include <QThread>

using namespace QeriPlayerQt;

static void delay(int ms)
{
    QThread::msleep(ms);
}

/// Assert ApiResult is success; on failure, report the error message and fail the test.
/// Unlike QVERIFY2, this does NOT eagerly evaluate result.error() on success,
/// which would trigger the assert in ApiResult::error().
#define QVERIFY_RESULT(result)                                                                                         \
    do {                                                                                                               \
        if (!(result).isSuccess()) {                                                                                   \
            QByteArray _msg = QString::fromUtf8("%1").arg((result).error().message()).toUtf8();                        \
            QTest::qFail(_msg.constData(), __FILE__, __LINE__);                                                        \
            return;                                                                                                    \
        }                                                                                                              \
    } while (false)

class TestNeteaseE2E : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<HttpClient> m_http;
    std::unique_ptr<NeteaseClient> m_client;
    QString m_cookie;

    // Shared test data
    QString m_firstSongId;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    // Auth
    void testLogin();

    // Read operations
    void testSearchSongs();
    void testGetSongDetail();
    void testGetSongUrl();
    void testGetLyrics();
    void testGetPlaylistDetail();
    void testGetAlbumDetail();
    void testGetUserPlaylists();

    // User operations
    void testLikeSong();
    void testUnlikeSong();
    void testGetLikedSongIds();
    void testGetCurrentUserAccount();
    void testGetCurrentUserId();

    // Download
    void testGetSongDownloadUrl();

    // High Quality Playlists
    void testGetHighQualityTags();
    void testGetRecommendedPlaylists();
    void testGetHighQualityPlaylists();

    // DJ Radio
    void testGetDjRadioDetail();

    // Related Playlists
    void testGetRelatedPlaylists();

    // User Playlist Wrappers
    void testGetUserCreatedPlaylists();
    void testGetUserSubscribedPlaylists();
    void testGetLikedPlaylistId();
    void testGetUserStarredAlbums();

    // Cleanup
    void testLogout();
};

void TestNeteaseE2E::initTestCase()
{
    m_cookie = QString::fromUtf8(qgetenv("NETEASE_COOKIE"));
    if (m_cookie.isEmpty()) {
        QSKIP("Set NETEASE_COOKIE to run E2E tests");
    }

    m_http = std::make_unique<HttpClient>();
    m_client = std::make_unique<NeteaseClient>(m_http.get());
    m_client->setCookies(m_cookie);
    QCoro::waitFor(m_client->ensureWeapiSession());
}

void TestNeteaseE2E::cleanupTestCase()
{
    m_client.reset();
    m_http.reset();
}

// ─── Auth ───────────────────────────────────────────────────────────────────

void TestNeteaseE2E::testLogin()
{
    QVERIFY(m_client->isAuthenticated());
}

// ─── Read Operations ────────────────────────────────────────────────────────

void TestNeteaseE2E::testSearchSongs()
{
    auto result = QCoro::waitFor(m_client->searchSongs(QStringLiteral("s62"), 10));

    QVERIFY_RESULT(result);

    const auto &sr = result.data();
    QVERIFY2(!sr.songs.isEmpty(), "Search returned empty songs");
    QVERIFY2(sr.totalCount > 0, "totalCount is 0");

    m_firstSongId = sr.songs.first().id;
}

void TestNeteaseE2E::testGetSongDetail()
{
    if (m_firstSongId.isEmpty()) {
        QSKIP("No song ID from search");
    }

    auto result = QCoro::waitFor(m_client->getSongDetail(m_firstSongId));

    QVERIFY_RESULT(result);

    const auto &song = result.data();
    QCOMPARE(song.platform, MusicPlatform::NetEase);
    QVERIFY2(!song.name.isEmpty(), qPrintable(QStringLiteral("Empty name for song %1").arg(m_firstSongId)));
    QVERIFY2(!song.artist.isEmpty(), qPrintable(QStringLiteral("Empty artist for song %1").arg(m_firstSongId)));
    QVERIFY2(song.durationMs > 0,
             qPrintable(QStringLiteral("durationMs=%1 for song %2").arg(song.durationMs).arg(m_firstSongId)));
}

void TestNeteaseE2E::testGetSongUrl()
{
    if (m_firstSongId.isEmpty()) {
        QSKIP("No song ID from search");
    }

    auto result = QCoro::waitFor(m_client->getSongUrl(m_firstSongId, AudioQuality::High));

    QVERIFY_RESULT(result);

    const auto &urlResult = result.data();
    QVERIFY2(urlResult.status == SongUrlResult::Status::Success,
             qPrintable(QStringLiteral("Song URL status=%1 (expected Success=0, RequiresLogin=2)")
                            .arg(static_cast<int>(urlResult.status))));

    QVERIFY2(
        !urlResult.url.isEmpty(),
        qPrintable(
            QStringLiteral("Empty URL for song %1, br=%2").arg(m_firstSongId).arg(urlResult.audioInfo.bitrateKbps)));
}

void TestNeteaseE2E::testGetLyrics()
{
    if (m_firstSongId.isEmpty()) {
        QSKIP("No song ID from search");
    }

    auto result = QCoro::waitFor(m_client->getLyrics(m_firstSongId));

    QVERIFY_RESULT(result);
}

void TestNeteaseE2E::testGetPlaylistDetail()
{
    auto result = QCoro::waitFor(m_client->getPlaylistDetail(QStringLiteral("3778678")));

    QVERIFY_RESULT(result);

    const auto &playlist = result.data();
    QVERIFY2(!playlist.name.isEmpty(), "Playlist name is empty");
    QVERIFY2(!playlist.songs.isEmpty(), qPrintable(QStringLiteral("Playlist %1 has no songs").arg(playlist.name)));
    QCOMPARE(playlist.platform, MusicPlatform::NetEase);
}

void TestNeteaseE2E::testGetAlbumDetail()
{
    auto result = QCoro::waitFor(m_client->getAlbumDetail(QStringLiteral("21506")));

    QVERIFY_RESULT(result);
    QVERIFY2(!result.data().isEmpty(), "Album has no songs");
}

void TestNeteaseE2E::testGetUserPlaylists()
{
    auto result = QCoro::waitFor(m_client->getUserPlaylists(QStringLiteral("1")));

    if (result.isError()) {
        // May fail due to privacy settings
        return;
    }
}

// ─── User Operations ──────────────────────────────────────────────────────

void TestNeteaseE2E::testLikeSong()
{
    if (m_firstSongId.isEmpty()) {
        QSKIP("No song ID from search");
    }

    auto result = QCoro::waitFor(m_client->likeSong(m_firstSongId));

    if (!result.isSuccess()) {
        QFAIL(qPrintable(
            QStringLiteral("likeSong failed (code=%1): %2").arg(result.error().code()).arg(result.error().message())));
    }
}

void TestNeteaseE2E::testUnlikeSong()
{
    if (m_firstSongId.isEmpty()) {
        QSKIP("No song ID from search");
    }

    auto result = QCoro::waitFor(m_client->unlikeSong(m_firstSongId));

    if (!result.isSuccess()) {
        QFAIL(qPrintable(QStringLiteral("unlikeSong failed (code=%1): %2")
                             .arg(result.error().code())
                             .arg(result.error().message())));
    }
}

void TestNeteaseE2E::testGetLikedSongIds()
{
    auto accountResult = QCoro::waitFor(m_client->getCurrentUserAccount());
    if (accountResult.isError()) {
        QSKIP("Cannot get user account");
    }

    QJsonObject profile = accountResult.data()[QLatin1String("profile")].toObject();
    QString userId = QString::number(profile[QLatin1String("userId")].toVariant().toLongLong());

    auto result = QCoro::waitFor(m_client->getLikedSongIds(userId));

    if (!result.isSuccess()) {
        QFAIL(qPrintable(QStringLiteral("getLikedSongIds failed (code=%1): %2")
                             .arg(result.error().code())
                             .arg(result.error().message())));
    }
}

void TestNeteaseE2E::testGetCurrentUserAccount()
{
    auto result = QCoro::waitFor(m_client->getCurrentUserAccount());

    QVERIFY_RESULT(result);

    QJsonObject profile = result.data()[QLatin1String("profile")].toObject();
    QVERIFY2(!profile[QLatin1String("userId")].isUndefined(), "Missing userId in profile");
}

void TestNeteaseE2E::testGetCurrentUserId()
{
    auto result = QCoro::waitFor(m_client->getCurrentUserId());

    QVERIFY_RESULT(result);

    QVERIFY2(result.data() > 0, qPrintable(QStringLiteral("Invalid userId: %1").arg(result.data())));
}

void TestNeteaseE2E::testGetSongDownloadUrl()
{
    if (m_firstSongId.isEmpty()) {
        QSKIP("No song ID from search");
    }

    auto result = QCoro::waitFor(m_client->getSongDownloadUrl(m_firstSongId, QStringLiteral("standard")));

    QVERIFY_RESULT(result);

    QJsonArray dataArray = result.data()[QLatin1String("data")].toArray();
    QVERIFY2(!dataArray.isEmpty(), qPrintable(QStringLiteral("No download URL for song %1").arg(m_firstSongId)));
}

void TestNeteaseE2E::testGetHighQualityTags()
{
    auto result = QCoro::waitFor(m_client->getHighQualityTags());

    if (result.isError()) {
        QSKIP(qPrintable(QStringLiteral("getHighQualityTags failed: %1").arg(result.error().message())));
    }
}

void TestNeteaseE2E::testGetRecommendedPlaylists()
{
    auto result = QCoro::waitFor(m_client->getRecommendedPlaylists());

    if (result.isError()) {
        QSKIP(qPrintable(QStringLiteral("getRecommendedPlaylists failed: %1").arg(result.error().message())));
    }
}

void TestNeteaseE2E::testGetHighQualityPlaylists()
{
    auto result = QCoro::waitFor(m_client->getHighQualityPlaylists(QStringLiteral("全部"), 10));

    if (result.isError()) {
        QSKIP(qPrintable(QStringLiteral("getHighQualityPlaylists failed: %1").arg(result.error().message())));
    }
}

void TestNeteaseE2E::testGetDjRadioDetail()
{
    // Use a known DJ radio ID (same endpoint as playlist detail)
    auto result = QCoro::waitFor(m_client->getDjRadioDetail(QStringLiteral("3778678")));

    QVERIFY_RESULT(result);

    QJsonObject playlist = result.data()[QLatin1String("playlist")].toObject();
    QVERIFY2(!playlist[QLatin1String("name")].toString().isEmpty(), "DJ radio detail missing name");
}

void TestNeteaseE2E::testGetRelatedPlaylists()
{
    // Use the hot playlist 3778678
    auto result = QCoro::waitFor(m_client->getRelatedPlaylists(QStringLiteral("3778678")));

    QVERIFY_RESULT(result);

    QJsonArray playlists = result.data()[QLatin1String("playlists")].toArray();
    // Related playlists may be empty for some playlists, so just check code
    QCOMPARE(result.data()[QLatin1String("code")].toInt(), 200);
}

void TestNeteaseE2E::testGetUserCreatedPlaylists()
{
    auto accountResult = QCoro::waitFor(m_client->getCurrentUserAccount());
    if (accountResult.isError()) {
        QSKIP("Cannot get user account");
    }

    QJsonObject profile = accountResult.data()[QLatin1String("profile")].toObject();
    QString userId = QString::number(profile[QLatin1String("userId")].toVariant().toLongLong());

    auto result = QCoro::waitFor(m_client->getUserCreatedPlaylists(userId, 10));

    QVERIFY_RESULT(result);
}

void TestNeteaseE2E::testGetUserSubscribedPlaylists()
{
    auto accountResult = QCoro::waitFor(m_client->getCurrentUserAccount());
    if (accountResult.isError()) {
        QSKIP("Cannot get user account");
    }

    QJsonObject profile = accountResult.data()[QLatin1String("profile")].toObject();
    QString userId = QString::number(profile[QLatin1String("userId")].toVariant().toLongLong());

    auto result = QCoro::waitFor(m_client->getUserSubscribedPlaylists(userId, 10));

    QVERIFY_RESULT(result);
}

void TestNeteaseE2E::testGetLikedPlaylistId()
{
    auto accountResult = QCoro::waitFor(m_client->getCurrentUserAccount());
    if (accountResult.isError()) {
        QSKIP("Cannot get user account");
    }

    QJsonObject profile = accountResult.data()[QLatin1String("profile")].toObject();
    QString userId = QString::number(profile[QLatin1String("userId")].toVariant().toLongLong());

    auto result = QCoro::waitFor(m_client->getLikedPlaylistId(userId));

    QVERIFY_RESULT(result);

    QVERIFY2(!result.data().isEmpty(), "Liked playlist ID is empty");
}

void TestNeteaseE2E::testGetUserStarredAlbums()
{
    auto accountResult = QCoro::waitFor(m_client->getCurrentUserAccount());
    if (accountResult.isError()) {
        QSKIP("Cannot get user account");
    }

    QJsonObject profile = accountResult.data()[QLatin1String("profile")].toObject();
    QString userId = QString::number(profile[QLatin1String("userId")].toVariant().toLongLong());

    auto result = QCoro::waitFor(m_client->getUserStarredAlbums(userId, 10));

    // May fail if user has no starred albums or API structure differs
    if (result.isError()) {
        QSKIP(qPrintable(QStringLiteral("getUserStarredAlbums failed: %1").arg(result.error().message())));
    }

    QCOMPARE(result.data()[QLatin1String("code")].toInt(), 200);
}

void TestNeteaseE2E::testLogout()
{
    auto result = QCoro::waitFor(m_client->logout());

    QVERIFY_RESULT(result);
    QVERIFY(!m_client->isAuthenticated());
}

QTEST_MAIN(TestNeteaseE2E)
#include "TestNeteaseE2E.moc"
