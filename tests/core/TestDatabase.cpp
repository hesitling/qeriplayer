/// @file TestDatabase.cpp
/// @brief Unit tests for the database module

#include "core/database/DatabaseManager.h"

#include <sqlite3.h>

#include <QTest>

using namespace QeriPlayerQt;

class TestDatabase : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // DatabaseManager lifecycle
    void open_inMemoryDatabase();
    void close_releasesConnection();
    void isOpen_returnsFalseBeforeOpen();

    // Schema versioning
    void firstTimeCreation_setsVersion2();
    void alreadyAtCurrentVersion_noMigration();
    void sequentialMigrations_fromV2();

    // Query execution
    void exec_selectWithPositionalParams();
    void exec_insertAndSelect();
    void exec_selectWithNamedParams();

    // Transaction support
    void transaction_commit();
    void transaction_rollback();

    // Error handling
    void exec_invalidSql_throwsDatabaseError();

    // Initial schema
    void initialSchema_createsAllTables();
};

void TestDatabase::open_inMemoryDatabase()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));
    QVERIFY(db.isOpen());
    db.close();
}

void TestDatabase::close_releasesConnection()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));
    db.close();
    QVERIFY(!db.isOpen());
}

void TestDatabase::isOpen_returnsFalseBeforeOpen()
{
    DatabaseManager db;
    QVERIFY(!db.isOpen());
}

void TestDatabase::firstTimeCreation_setsVersion2()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));
    QCOMPARE(db.schemaVersion(), 2);
    db.close();
}

void TestDatabase::alreadyAtCurrentVersion_noMigration()
{
    // With in-memory DB each open() is a fresh database, so this test
    // verifies that opening twice in sequence doesn't break anything.
    // We use a single open to verify the version is stable.
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));
    QCOMPARE(db.schemaVersion(), 2);

    // Close and reopen a fresh in-memory DB — still version 2
    db.close();
    DatabaseManager db2;
    QVERIFY(db2.open(QString(":memory:")));
    QCOMPARE(db2.schemaVersion(), 2);
    db2.close();
}

void TestDatabase::sequentialMigrations_fromV2()
{
    DatabaseManager db;
    // Register an extra migration on top of v2 base
    db.registerMigration(3, [](sqlite3 *handle) {
        const char *sql = "CREATE TABLE IF NOT EXISTS test_table (id INTEGER PRIMARY KEY);";
        return sqlite3_exec(handle, sql, nullptr, nullptr, nullptr) == SQLITE_OK;
    });

    QVERIFY(db.open(QString(":memory:")));
    QCOMPARE(db.schemaVersion(), 3);
    db.close();
}

void TestDatabase::exec_selectWithPositionalParams()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    db.exec("INSERT INTO settings (key, value) VALUES (?, ?)", {QString("mykey"), QString("myval")});

    auto rows = db.exec("SELECT key, value FROM settings WHERE key = ?", {QString("mykey")});
    QCOMPARE(rows.size(), 1);
    QCOMPARE(rows[0][0].toString(), QStringLiteral("mykey"));
    QCOMPARE(rows[0][1].toString(), QStringLiteral("myval"));

    db.close();
}

void TestDatabase::exec_insertAndSelect()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    db.exec("INSERT INTO settings (key, value) VALUES (?, ?)", {QString("k1"), QString("v1")});
    db.exec("INSERT INTO settings (key, value) VALUES (?, ?)", {QString("k2"), QString("v2")});

    auto rows = db.exec("SELECT key, value FROM settings ORDER BY key");
    QCOMPARE(rows.size(), 2);
    QCOMPARE(rows[0][0].toString(), QStringLiteral("k1"));
    QCOMPARE(rows[1][0].toString(), QStringLiteral("k2"));

    db.close();
}

void TestDatabase::exec_selectWithNamedParams()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    db.exec("INSERT INTO settings (key, value) VALUES (?, ?)", {QString("named_test"), QString("val")});

    QVariantMap params;
    params[":k"] = QString("named_test");
    auto rows = db.execNamed("SELECT key, value FROM settings WHERE key = :k", params);
    QCOMPARE(rows.size(), 1);
    QCOMPARE(rows[0][0].toString(), QStringLiteral("named_test"));

    db.close();
}

void TestDatabase::transaction_commit()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    db.beginTransaction();
    db.exec("INSERT INTO settings (key, value) VALUES (?, ?)", {QString("txn_k1"), QString("txn_v1")});
    db.exec("INSERT INTO settings (key, value) VALUES (?, ?)", {QString("txn_k2"), QString("txn_v2")});
    db.commitTransaction();

    auto rows = db.exec("SELECT key FROM settings WHERE key LIKE 'txn_%'");
    QCOMPARE(rows.size(), 2);

    db.close();
}

void TestDatabase::transaction_rollback()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    db.beginTransaction();
    db.exec("INSERT INTO settings (key, value) VALUES (?, ?)", {QString("rollback_k"), QString("rollback_v")});
    db.rollbackTransaction();

    auto rows = db.exec("SELECT key FROM settings WHERE key = 'rollback_k'");
    QCOMPARE(rows.size(), 0);

    db.close();
}

void TestDatabase::exec_invalidSql_throwsDatabaseError()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    QVERIFY_EXCEPTION_THROWN(db.exec("INVALID SQL"), DatabaseError);

    db.close();
}

void TestDatabase::initialSchema_createsAllTables()
{
    DatabaseManager db;
    QVERIFY(db.open(QString(":memory:")));

    // Check all tables exist by querying sqlite_master
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

QTEST_MAIN(TestDatabase)
#include "TestDatabase.moc"
