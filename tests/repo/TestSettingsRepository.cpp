/// @file TestSettingsRepository.cpp
/// @brief Tests for SettingsRepository

#include "core/database/DatabaseManager.h"
#include "repo/SettingsRepository.h"

#include <QTest>

using namespace QeriPlayerQt;

class TestSettingsRepository : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<DatabaseManager> createDb();

private Q_SLOTS:
    void get_existingKey();
    void get_nonExistent_returnsEmpty();
    void set_newKey();
    void set_upsert();
    void remove_existingKey();
    void remove_nonExistent_noOp();
    void getAll_returnsAll();
    void getBool_trueValue();
    void getBool_falseValue();
    void getBool_nonExistent_returnsDefault();
    void getInt_validValue();
    void getInt_nonExistent_returnsDefault();
    void getBool_invalidValue_returnsDefault();
};

std::unique_ptr<DatabaseManager> TestSettingsRepository::createDb()
{
    auto db = std::make_unique<DatabaseManager>();
    db->open(QString(":memory:"));
    return db;
}

void TestSettingsRepository::get_existingKey()
{
    auto db = createDb();
    SettingsRepository repo(db.get());

    repo.set("audio_quality", "high");
    auto val = repo.get("audio_quality");
    QVERIFY(val.has_value());
    QCOMPARE(*val, QStringLiteral("high"));
}

void TestSettingsRepository::get_nonExistent_returnsEmpty()
{
    auto db = createDb();
    SettingsRepository repo(db.get());

    auto val = repo.get("nonexistent");
    QVERIFY(!val.has_value());
}

void TestSettingsRepository::set_newKey()
{
    auto db = createDb();
    SettingsRepository repo(db.get());

    repo.set("volume", "80");
    QCOMPARE(repo.getInt("volume", 0), 80);
}

void TestSettingsRepository::set_upsert()
{
    auto db = createDb();
    SettingsRepository repo(db.get());

    repo.set("quality", "low");
    QCOMPARE(*repo.get("quality"), QStringLiteral("low"));

    repo.set("quality", "high");
    QCOMPARE(*repo.get("quality"), QStringLiteral("high"));
}

void TestSettingsRepository::remove_existingKey()
{
    auto db = createDb();
    SettingsRepository repo(db.get());

    repo.set("key1", "val1");
    QVERIFY(repo.get("key1").has_value());

    repo.remove("key1");
    QVERIFY(!repo.get("key1").has_value());
}

void TestSettingsRepository::remove_nonExistent_noOp()
{
    auto db = createDb();
    SettingsRepository repo(db.get());

    // Should not throw
    repo.remove("nonexistent");
}

void TestSettingsRepository::getAll_returnsAll()
{
    auto db = createDb();
    SettingsRepository repo(db.get());

    repo.set("k1", "v1");
    repo.set("k2", "v2");
    repo.set("k3", "v3");

    auto all = repo.getAll();
    QCOMPARE(all.size(), 3);
    QCOMPARE(all["k1"].toString(), QStringLiteral("v1"));
    QCOMPARE(all["k2"].toString(), QStringLiteral("v2"));
    QCOMPARE(all["k3"].toString(), QStringLiteral("v3"));
}

void TestSettingsRepository::getBool_trueValue()
{
    auto db = createDb();
    SettingsRepository repo(db.get());

    repo.set("shuffle", "true");
    QVERIFY(repo.getBool("shuffle", false));
}

void TestSettingsRepository::getBool_falseValue()
{
    auto db = createDb();
    SettingsRepository repo(db.get());

    repo.set("shuffle", "false");
    QVERIFY(!repo.getBool("shuffle", true));
}

void TestSettingsRepository::getBool_nonExistent_returnsDefault()
{
    auto db = createDb();
    SettingsRepository repo(db.get());

    QVERIFY(repo.getBool("missing", true));
    QVERIFY(!repo.getBool("missing", false));
}

void TestSettingsRepository::getInt_validValue()
{
    auto db = createDb();
    SettingsRepository repo(db.get());

    repo.set("volume", "42");
    QCOMPARE(repo.getInt("volume", 0), 42);
}

void TestSettingsRepository::getInt_nonExistent_returnsDefault()
{
    auto db = createDb();
    SettingsRepository repo(db.get());

    QCOMPARE(repo.getInt("missing", 99), 99);
}

void TestSettingsRepository::getBool_invalidValue_returnsDefault()
{
    auto db = createDb();
    SettingsRepository repo(db.get());

    repo.set("shuffle", "maybe");
    QVERIFY(repo.getBool("shuffle", true));
    QVERIFY(!repo.getBool("shuffle", false));
}

QTEST_MAIN(TestSettingsRepository)
#include "TestSettingsRepository.moc"
