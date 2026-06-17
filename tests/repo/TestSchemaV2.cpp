/// @file TestSchemaV2.cpp
/// @brief Tests for v2 schema (songs_cache, player_state, playlists)

#include "core/database/DatabaseManager.h"

#include <QTest>

using namespace NeriPlayerQt;

class TestSchemaV2 : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void songsCache_has27Columns();
    void songsCache_columnNames();
    void playerState_exists();
    void playerState_singletonConstraint();
    void playlists_hasNewColumns();
    void allTables_created();
};

void TestSchemaV2::songsCache_has27Columns()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    auto rows = db.exec("PRAGMA table_info(songs_cache)");
    QCOMPARE(rows.size(), 27);
    db.close();
}

void TestSchemaV2::songsCache_columnNames()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    auto rows = db.exec("PRAGMA table_info(songs_cache)");
    QStringList names;
    for (const auto &row : rows) {
        names << row[1].toString();
    }

    // Core columns
    QVERIFY(names.contains("id"));
    QVERIFY(names.contains("platform"));
    QVERIFY(names.contains("name"));
    QVERIFY(names.contains("artist"));
    QVERIFY(names.contains("album"));
    QVERIFY(names.contains("album_id"));
    QVERIFY(names.contains("duration_ms"));
    QVERIFY(names.contains("cover_url"));
    QVERIFY(names.contains("media_uri"));

    // Customization columns
    QVERIFY(names.contains("custom_name"));
    QVERIFY(names.contains("custom_artist"));
    QVERIFY(names.contains("custom_cover_url"));
    QVERIFY(names.contains("original_name"));
    QVERIFY(names.contains("original_artist"));
    QVERIFY(names.contains("original_cover_url"));

    // Local file columns
    QVERIFY(names.contains("local_file_name"));
    QVERIFY(names.contains("local_file_path"));

    // Lyric matching columns
    QVERIFY(names.contains("matched_lyric_source"));
    QVERIFY(names.contains("matched_song_id"));
    QVERIFY(names.contains("user_lyric_offset_ms"));
    QVERIFY(names.contains("lyrics_json"));

    // Platform identifiers
    QVERIFY(names.contains("channel_id"));
    QVERIFY(names.contains("audio_id"));
    QVERIFY(names.contains("sub_audio_id"));

    // Extra data
    QVERIFY(names.contains("extra_json"));

    // Timestamps
    QVERIFY(names.contains("cached_at"));
    QVERIFY(names.contains("last_played_at"));

    // Old names must not exist
    QVERIFY(!names.contains("title"));
    QVERIFY(!names.contains("playback_url"));
    QVERIFY(!names.contains("duration"));

    db.close();
}

void TestSchemaV2::playerState_exists()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    auto rows = db.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='player_state'");
    QCOMPARE(rows.size(), 1);

    auto cols = db.exec("PRAGMA table_info(player_state)");
    QCOMPARE(cols.size(), 9);

    QStringList names;
    for (const auto &row : cols) {
        names << row[1].toString();
    }
    QVERIFY(names.contains("id"));
    QVERIFY(names.contains("playlist_json"));
    QVERIFY(names.contains("current_index"));
    QVERIFY(names.contains("media_url"));
    QVERIFY(names.contains("position_ms"));
    QVERIFY(names.contains("should_resume"));
    QVERIFY(names.contains("repeat_mode"));
    QVERIFY(names.contains("shuffle_enabled"));
    QVERIFY(names.contains("updated_at"));

    db.close();
}

void TestSchemaV2::playerState_singletonConstraint()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    // Insert row with id=1 should work
    db.exec("INSERT INTO player_state (id, position_ms) VALUES (1, 1000)");
    auto rows = db.exec("SELECT id FROM player_state");
    QCOMPARE(rows.size(), 1);

    // Insert row with id=2 should fail due to CHECK constraint
    QVERIFY_EXCEPTION_THROWN(db.exec("INSERT INTO player_state (id, position_ms) VALUES (2, 2000)"), DatabaseError);

    rows = db.exec("SELECT id FROM player_state");
    QCOMPARE(rows.size(), 1);

    db.close();
}

void TestSchemaV2::playlists_hasNewColumns()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    auto rows = db.exec("PRAGMA table_info(playlists)");
    QStringList names;
    for (const auto &row : rows) {
        names << row[1].toString();
    }
    QVERIFY(names.contains("custom_cover_url"));
    QVERIFY(names.contains("modified_at"));

    db.close();
}

void TestSchemaV2::allTables_created()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    auto rows = db.exec("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name");
    QStringList tables;
    for (const auto &row : rows) {
        tables << row[0].toString();
    }

    QVERIFY(tables.contains("songs_cache"));
    QVERIFY(tables.contains("playlists"));
    QVERIFY(tables.contains("playlist_songs"));
    QVERIFY(tables.contains("settings"));
    QVERIFY(tables.contains("play_history"));
    QVERIFY(tables.contains("player_state"));
    QVERIFY(tables.contains("schema_version"));

    db.close();
}

QTEST_MAIN(TestSchemaV2)
#include "TestSchemaV2.moc"
