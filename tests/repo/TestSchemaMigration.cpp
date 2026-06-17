/// @file TestSchemaMigration.cpp
/// @brief Tests for v1→v2 schema migration

#include "core/database/DatabaseManager.h"

#include <sqlite3.h>

#include <QTemporaryDir>
#include <QTest>

using namespace NeriPlayerQt;

class TestSchemaMigration : public QObject {
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    // v2 schema verification
    void v2Schema_songsCacheHas27Columns();
    void v2Schema_playerStateTableExists();
    void v2Schema_playlistsHasNewColumns();

    // Migration from v1 to v2
    void migrateV1ToV2_preservesExistingSongs();
    void migrateV1ToV2_renamesColumns();
    void migrateV1ToV2_playerStateSingletonConstraint();
};

void TestSchemaMigration::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
}

void TestSchemaMigration::cleanupTestCase()
{
    // temp dir auto-cleans
}

void TestSchemaMigration::v2Schema_songsCacheHas27Columns()
{
    QString path = m_tempDir.filePath("v2_columns.db");

    DatabaseManager db;
    QVERIFY(db.open(path));
    QCOMPARE(db.schemaVersion(), 2);

    auto rows = db.exec("PRAGMA table_info(songs_cache)");
    QCOMPARE(rows.size(), 27);

    // Verify column names
    QStringList colNames;
    for (const auto &row : rows) {
        colNames << row[1].toString();
    }
    QVERIFY(colNames.contains("id"));
    QVERIFY(colNames.contains("platform"));
    QVERIFY(colNames.contains("name"));
    QVERIFY(colNames.contains("artist"));
    QVERIFY(colNames.contains("album"));
    QVERIFY(colNames.contains("album_id"));
    QVERIFY(colNames.contains("duration_ms"));
    QVERIFY(colNames.contains("cover_url"));
    QVERIFY(colNames.contains("media_uri"));
    QVERIFY(colNames.contains("custom_name"));
    QVERIFY(colNames.contains("custom_artist"));
    QVERIFY(colNames.contains("custom_cover_url"));
    QVERIFY(colNames.contains("original_name"));
    QVERIFY(colNames.contains("original_artist"));
    QVERIFY(colNames.contains("original_cover_url"));
    QVERIFY(colNames.contains("local_file_name"));
    QVERIFY(colNames.contains("local_file_path"));
    QVERIFY(colNames.contains("matched_lyric_source"));
    QVERIFY(colNames.contains("matched_song_id"));
    QVERIFY(colNames.contains("user_lyric_offset_ms"));
    QVERIFY(colNames.contains("lyrics_json"));
    QVERIFY(colNames.contains("channel_id"));
    QVERIFY(colNames.contains("audio_id"));
    QVERIFY(colNames.contains("sub_audio_id"));
    QVERIFY(colNames.contains("extra_json"));
    QVERIFY(colNames.contains("cached_at"));
    QVERIFY(colNames.contains("last_played_at"));

    // Verify renamed columns don't exist
    QVERIFY(!colNames.contains("title"));
    QVERIFY(!colNames.contains("playback_url"));
    QVERIFY(!colNames.contains("duration"));

    db.close();
}

void TestSchemaMigration::v2Schema_playerStateTableExists()
{
    QString path = m_tempDir.filePath("v2_player_state.db");

    DatabaseManager db;
    QVERIFY(db.open(path));

    auto rows = db.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='player_state'");
    QCOMPARE(rows.size(), 1);

    // Verify columns
    auto cols = db.exec("PRAGMA table_info(player_state)");
    QCOMPARE(cols.size(), 9);

    QStringList colNames;
    for (const auto &row : cols) {
        colNames << row[1].toString();
    }
    QVERIFY(colNames.contains("id"));
    QVERIFY(colNames.contains("playlist_json"));
    QVERIFY(colNames.contains("current_index"));
    QVERIFY(colNames.contains("media_url"));
    QVERIFY(colNames.contains("position_ms"));
    QVERIFY(colNames.contains("should_resume"));
    QVERIFY(colNames.contains("repeat_mode"));
    QVERIFY(colNames.contains("shuffle_enabled"));
    QVERIFY(colNames.contains("updated_at"));

    db.close();
}

void TestSchemaMigration::v2Schema_playlistsHasNewColumns()
{
    QString path = m_tempDir.filePath("v2_playlists.db");

    DatabaseManager db;
    QVERIFY(db.open(path));

    auto rows = db.exec("PRAGMA table_info(playlists)");
    QStringList colNames;
    for (const auto &row : rows) {
        colNames << row[1].toString();
    }
    QVERIFY(colNames.contains("custom_cover_url"));
    QVERIFY(colNames.contains("modified_at"));

    db.close();
}

void TestSchemaMigration::migrateV1ToV2_preservesExistingSongs()
{
    // Create a v1 database with some songs
    QString path = m_tempDir.filePath("migrate_preserve.db");

    {
        // Create v1 database manually
        sqlite3 *db = nullptr;
        QCOMPARE(sqlite3_open(path.toUtf8().constData(), &db), SQLITE_OK);

        const char *v1Schema = R"SQL(
            CREATE TABLE IF NOT EXISTS schema_version (version INTEGER NOT NULL PRIMARY KEY);
            INSERT INTO schema_version (version) VALUES (1);

            CREATE TABLE IF NOT EXISTS songs_cache (
                id           TEXT PRIMARY KEY,
                platform     TEXT,
                title        TEXT,
                artist       TEXT,
                album        TEXT,
                duration     INTEGER,
                cover_url    TEXT,
                playback_url TEXT,
                extra_json   TEXT,
                cached_at    TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );

            CREATE TABLE IF NOT EXISTS playlists (
                id         TEXT PRIMARY KEY,
                platform   TEXT,
                name       TEXT,
                description TEXT,
                cover_url  TEXT,
                song_count INTEGER DEFAULT 0,
                owner      TEXT
            );

            CREATE TABLE IF NOT EXISTS playlist_songs (
                playlist_id TEXT,
                song_id     TEXT,
                position    INTEGER,
                PRIMARY KEY (playlist_id, song_id),
                FOREIGN KEY (playlist_id) REFERENCES playlists(id) ON DELETE CASCADE,
                FOREIGN KEY (song_id) REFERENCES songs_cache(id) ON DELETE CASCADE
            );

            CREATE TABLE IF NOT EXISTS settings (
                key   TEXT PRIMARY KEY,
                value TEXT
            );

            CREATE TABLE IF NOT EXISTS play_history (
                id       INTEGER PRIMARY KEY AUTOINCREMENT,
                song_id  TEXT,
                played_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
        )SQL";

        char *errMsg = nullptr;
        int rc = sqlite3_exec(db, v1Schema, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            QString err = errMsg ? QString::fromUtf8(errMsg) : "unknown";
            sqlite3_free(errMsg);
            sqlite3_close(db);
            QFAIL(qPrintable("v1 schema creation failed: " + err));
        }

        // Insert test songs
        const char *insertSql = R"SQL(
            INSERT INTO songs_cache (id, platform, title, artist, album, duration, cover_url, playback_url, extra_json)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);
        )SQL";

        sqlite3_stmt *stmt = nullptr;
        sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, "song1", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, "NetEase", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, "Test Song", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, "Test Artist", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, "Test Album", -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 6, 180000);
        sqlite3_bind_text(stmt, 7, "https://cover.url", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, "https://play.url", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 9, "{}", -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        sqlite3_close(db);
    }

    // Now open with our DatabaseManager — should migrate to v2
    DatabaseManager db;
    QVERIFY(db.open(path));
    QCOMPARE(db.schemaVersion(), 2);

    // Verify data was preserved with new column names
    auto rows = db.exec("SELECT id, name, artist, album, duration_ms, media_uri FROM songs_cache WHERE id = 'song1'");
    QCOMPARE(rows.size(), 1);
    QCOMPARE(rows[0][0].toString(), QStringLiteral("song1"));
    QCOMPARE(rows[0][1].toString(), QStringLiteral("Test Song"));
    QCOMPARE(rows[0][2].toString(), QStringLiteral("Test Artist"));
    QCOMPARE(rows[0][3].toString(), QStringLiteral("Test Album"));
    QCOMPARE(rows[0][4].toInt(), 180000);
    QCOMPARE(rows[0][5].toString(), QStringLiteral("https://play.url"));

    db.close();
}

void TestSchemaMigration::migrateV1ToV2_renamesColumns()
{
    // Verify that the old column names don't exist after migration
    QString path = m_tempDir.filePath("migrate_rename.db");

    {
        sqlite3 *db = nullptr;
        QCOMPARE(sqlite3_open(path.toUtf8().constData(), &db), SQLITE_OK);

        const char *v1Schema = R"SQL(
            CREATE TABLE IF NOT EXISTS schema_version (version INTEGER NOT NULL PRIMARY KEY);
            INSERT INTO schema_version (version) VALUES (1);
            CREATE TABLE IF NOT EXISTS songs_cache (
                id TEXT PRIMARY KEY, platform TEXT, title TEXT, artist TEXT, album TEXT,
                duration INTEGER, cover_url TEXT, playback_url TEXT, extra_json TEXT,
                cached_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
            CREATE TABLE IF NOT EXISTS playlists (
                id TEXT PRIMARY KEY, platform TEXT, name TEXT, description TEXT,
                cover_url TEXT, song_count INTEGER DEFAULT 0, owner TEXT
            );
            CREATE TABLE IF NOT EXISTS playlist_songs (
                playlist_id TEXT, song_id TEXT, position INTEGER,
                PRIMARY KEY (playlist_id, song_id),
                FOREIGN KEY (playlist_id) REFERENCES playlists(id) ON DELETE CASCADE,
                FOREIGN KEY (song_id) REFERENCES songs_cache(id) ON DELETE CASCADE
            );
            CREATE TABLE IF NOT EXISTS settings (key TEXT PRIMARY KEY, value TEXT);
            CREATE TABLE IF NOT EXISTS play_history (
                id INTEGER PRIMARY KEY AUTOINCREMENT, song_id TEXT,
                played_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
        )SQL";

        char *errMsg = nullptr;
        sqlite3_exec(db, v1Schema, nullptr, nullptr, &errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
    }

    DatabaseManager db;
    QVERIFY(db.open(path));

    auto rows = db.exec("PRAGMA table_info(songs_cache)");
    QStringList colNames;
    for (const auto &row : rows) {
        colNames << row[1].toString();
    }

    // Old names should not exist
    QVERIFY(!colNames.contains("title"));
    QVERIFY(!colNames.contains("playback_url"));
    QVERIFY(!colNames.contains("duration"));

    // New names should exist
    QVERIFY(colNames.contains("name"));
    QVERIFY(colNames.contains("media_uri"));
    QVERIFY(colNames.contains("duration_ms"));

    db.close();
}

void TestSchemaMigration::migrateV1ToV2_playerStateSingletonConstraint()
{
    QString path = m_tempDir.filePath("migrate_singleton.db");

    DatabaseManager db;
    QVERIFY(db.open(path));

    // Insert row with id=1 should work
    db.exec("INSERT INTO player_state (id, position_ms) VALUES (1, 1000)");
    auto rows = db.exec("SELECT id FROM player_state");
    QCOMPARE(rows.size(), 1);

    // Insert row with id=2 should fail
    QVERIFY_EXCEPTION_THROWN(
        db.exec("INSERT INTO player_state (id, position_ms) VALUES (2, 2000)"),
        DatabaseError
    );

    // Still only one row
    rows = db.exec("SELECT id FROM player_state");
    QCOMPARE(rows.size(), 1);

    db.close();
}

QTEST_MAIN(TestSchemaMigration)
#include "TestSchemaMigration.moc"
