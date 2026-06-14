/// @file TestDatabase.cpp
/// @brief Unit tests for the database module
/// @date 2024-01-15

#include "core/database/DatabaseManager.h"

#include <sqlite3.h>

#include <QTemporaryDir>
#include <QTest>

using namespace NeriPlayerQt;

class TestDatabase : public QObject {
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    // DatabaseManager lifecycle
    void open_createsNewDatabase();
    void open_existingDatabase();
    void close_releasesFileHandle();

    // Schema versioning
    void firstTimeCreation_setsVersion1();
    void alreadyAtCurrentVersion_noMigration();
    void sequentialMigrations();

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

void TestDatabase::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
}

void TestDatabase::cleanupTestCase()
{
    // temp dir auto-cleans
}

void TestDatabase::open_createsNewDatabase()
{
    QString path = m_tempDir.filePath("test_new.db");
    QVERIFY(!QFile::exists(path));

    DatabaseManager db;
    bool ok = db.open(path);
    QVERIFY(ok);
    QVERIFY(QFile::exists(path));
    db.close();
}

void TestDatabase::open_existingDatabase()
{
    QString path = m_tempDir.filePath("test_existing.db");

    DatabaseManager db1;
    QVERIFY(db1.open(path));
    db1.close();

    DatabaseManager db2;
    QVERIFY(db2.open(path));
    db2.close();
}

void TestDatabase::close_releasesFileHandle()
{
    QString path = m_tempDir.filePath("test_close.db");

    DatabaseManager db;
    QVERIFY(db.open(path));
    db.close();

    // Should be able to open again
    DatabaseManager db2;
    QVERIFY(db2.open(path));
    db2.close();
}

void TestDatabase::firstTimeCreation_setsVersion1()
{
    QString path = m_tempDir.filePath("test_version1.db");

    DatabaseManager db;
    QVERIFY(db.open(path));
    QCOMPARE(db.schemaVersion(), 1);
    db.close();
}

void TestDatabase::alreadyAtCurrentVersion_noMigration()
{
    QString path = m_tempDir.filePath("test_no_migration.db");

    DatabaseManager db;
    QVERIFY(db.open(path));
    QCOMPARE(db.schemaVersion(), 1);

    // Open again — should stay at version 1
    db.close();
    DatabaseManager db2;
    QVERIFY(db2.open(path));
    QCOMPARE(db2.schemaVersion(), 1);
    db2.close();
}

void TestDatabase::sequentialMigrations()
{
    QString path = m_tempDir.filePath("test_migrations.db");

    DatabaseManager db;
    // Register an extra migration before opening
    db.registerMigration(2, [](sqlite3 *handle) {
        const char *sql = "CREATE TABLE IF NOT EXISTS test_table (id INTEGER PRIMARY KEY);";
        return sqlite3_exec(handle, sql, nullptr, nullptr, nullptr) == SQLITE_OK;
    });

    QVERIFY(db.open(path));
    QCOMPARE(db.schemaVersion(), 2);
    db.close();
}

void TestDatabase::exec_selectWithPositionalParams()
{
    QString path = m_tempDir.filePath("test_select_params.db");

    DatabaseManager db;
    QVERIFY(db.open(path));

    db.exec("INSERT INTO settings (key, value) VALUES (?, ?)", { QString("mykey"), QString("myval") });

    auto rows = db.exec("SELECT key, value FROM settings WHERE key = ?", { QString("mykey") });
    QCOMPARE(rows.size(), 1);
    QCOMPARE(rows[0][0].toString(), QStringLiteral("mykey"));
    QCOMPARE(rows[0][1].toString(), QStringLiteral("myval"));

    db.close();
}

void TestDatabase::exec_insertAndSelect()
{
    QString path = m_tempDir.filePath("test_insert_select.db");

    DatabaseManager db;
    QVERIFY(db.open(path));

    db.exec("INSERT INTO settings (key, value) VALUES (?, ?)", { QString("k1"), QString("v1") });
    db.exec("INSERT INTO settings (key, value) VALUES (?, ?)", { QString("k2"), QString("v2") });

    auto rows = db.exec("SELECT key, value FROM settings ORDER BY key");
    QCOMPARE(rows.size(), 2);
    QCOMPARE(rows[0][0].toString(), QStringLiteral("k1"));
    QCOMPARE(rows[1][0].toString(), QStringLiteral("k2"));

    db.close();
}

void TestDatabase::exec_selectWithNamedParams()
{
    QString path = m_tempDir.filePath("test_named_params.db");

    DatabaseManager db;
    QVERIFY(db.open(path));

    db.exec("INSERT INTO settings (key, value) VALUES (?, ?)", { QString("named_test"), QString("val") });

    QVariantMap params;
    params[":k"] = QString("named_test");
    auto rows = db.execNamed("SELECT key, value FROM settings WHERE key = :k", params);
    QCOMPARE(rows.size(), 1);
    QCOMPARE(rows[0][0].toString(), QStringLiteral("named_test"));

    db.close();
}

void TestDatabase::transaction_commit()
{
    QString path = m_tempDir.filePath("test_txn_commit.db");

    DatabaseManager db;
    QVERIFY(db.open(path));

    db.beginTransaction();
    db.exec("INSERT INTO settings (key, value) VALUES (?, ?)", { QString("txn_k1"), QString("txn_v1") });
    db.exec("INSERT INTO settings (key, value) VALUES (?, ?)", { QString("txn_k2"), QString("txn_v2") });
    db.commitTransaction();

    auto rows = db.exec("SELECT key FROM settings WHERE key LIKE 'txn_%'");
    QCOMPARE(rows.size(), 2);

    db.close();
}

void TestDatabase::transaction_rollback()
{
    QString path = m_tempDir.filePath("test_txn_rollback.db");

    DatabaseManager db;
    QVERIFY(db.open(path));

    db.beginTransaction();
    db.exec("INSERT INTO settings (key, value) VALUES (?, ?)", { QString("rollback_k"), QString("rollback_v") });
    db.rollbackTransaction();

    auto rows = db.exec("SELECT key FROM settings WHERE key = 'rollback_k'");
    QCOMPARE(rows.size(), 0);

    db.close();
}

void TestDatabase::exec_invalidSql_throwsDatabaseError()
{
    QString path = m_tempDir.filePath("test_error.db");

    DatabaseManager db;
    QVERIFY(db.open(path));

    QVERIFY_EXCEPTION_THROWN(db.exec("INVALID SQL"), DatabaseError);

    db.close();
}

void TestDatabase::initialSchema_createsAllTables()
{
    QString path = m_tempDir.filePath("test_schema.db");

    DatabaseManager db;
    QVERIFY(db.open(path));

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
    QVERIFY(tables.contains("schema_version"));

    db.close();
}

QTEST_MAIN(TestDatabase)
#include "TestDatabase.moc"
