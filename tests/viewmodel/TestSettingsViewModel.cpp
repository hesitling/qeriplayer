/// @file TestSettingsViewModel.cpp
/// @brief Unit tests for SettingsViewModel

#include "domain/Enums.h"
#include "repo/IPlayHistoryRepository.h"
#include "repo/ISettingsRepository.h"
#include "viewmodel/SettingsViewModel.h"

#include <QCoroTask>
#include <QSignalSpy>
#include <QTest>

using namespace QeriPlayerQt;

// --- Mock ISettingsRepository ---

class MockSettingsRepo : public ISettingsRepository {
public:
    std::optional<QString> get(const QString &key) override
    {
        auto it = m_settings.find(key);
        if (it != m_settings.end())
            return it.value().toString();
        return std::nullopt;
    }
    void set(const QString &key, const QString &value) override
    {
        m_settings[key] = value;
    }
    void remove(const QString &key) override
    {
        m_settings.remove(key);
    }
    QVariantMap getAll() override
    {
        return m_settings;
    }
    bool getBool(const QString &key, bool defaultValue) override
    {
        auto it = m_settings.find(key);
        if (it != m_settings.end())
            return it.value() == "true";
        return defaultValue;
    }
    int getInt(const QString &key, int defaultValue) override
    {
        auto it = m_settings.find(key);
        if (it != m_settings.end())
            return it.value().toInt();
        return defaultValue;
    }

    QVariantMap m_settings;
};

// --- Mock IPlayHistoryRepository ---

class MockPlayHistoryRepo : public IPlayHistoryRepository {
public:
    void record(const QString &) override { }
    QVector<Song> recent(int) override
    {
        return {};
    }
    void clear() override
    {
        if (m_throwOnClear)
            throw std::runtime_error("DB error");
        m_cleared = true;
    }
    void remove(const QStringList &) override { }
    int playCount(const QString &) override
    {
        return 0;
    }

    bool m_cleared = false;
    bool m_throwOnClear = false;
};

// --- Test class ---

class TestSettingsViewModel : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initialState();
    void loadSettings_emptyRepo();
    void loadSettings_withTheme();
    void loadSettings_withAudioQuality();
    void loadSettings_withDownloadPath();
    void setTheme_persistsAndEmits();
    void setTheme_sameValue_noSignal();
    void setAudioQuality_persistsAndEmits();
    void setDownloadPath_persistsAndEmits();
    void clearPlayHistory();
    void clearError();
    void isNeteaseLoggedIn_nullClient();
    void setTheme_invalidValue();
    void setTheme_validValues();
    void clearPlayHistory_repoException_doesNotCrash();
};

void TestSettingsViewModel::initialState()
{
    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    // Pass nullptr for NeteaseClient — not needed for basic settings tests
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    QCOMPARE(vm.theme(), QStringLiteral("light"));
    QCOMPARE(vm.audioQuality(), AudioQuality::High);
    QVERIFY(vm.downloadPath().isEmpty());
    QVERIFY(!vm.hasError());
}

void TestSettingsViewModel::loadSettings_emptyRepo()
{
    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    vm.loadSettings();

    QCOMPARE(vm.theme(), QStringLiteral("light"));
    QCOMPARE(vm.audioQuality(), AudioQuality::High);
}

void TestSettingsViewModel::loadSettings_withTheme()
{
    MockSettingsRepo settingsRepo;
    settingsRepo.set("theme", "dark");
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    QSignalSpy spy(&vm, &SettingsViewModel::themeChanged);
    vm.loadSettings();

    QCOMPARE(vm.theme(), QStringLiteral("dark"));
    QCOMPARE(spy.count(), 1);
}

void TestSettingsViewModel::loadSettings_withAudioQuality()
{
    MockSettingsRepo settingsRepo;
    settingsRepo.set("audioQuality", "Lossless");
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    QSignalSpy spy(&vm, &SettingsViewModel::audioQualityChanged);
    vm.loadSettings();

    QCOMPARE(vm.audioQuality(), AudioQuality::Lossless);
    QCOMPARE(spy.count(), 1);
}

void TestSettingsViewModel::loadSettings_withDownloadPath()
{
    MockSettingsRepo settingsRepo;
    settingsRepo.set("downloadPath", "/home/user/Music");
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    QSignalSpy spy(&vm, &SettingsViewModel::downloadPathChanged);
    vm.loadSettings();

    QCOMPARE(vm.downloadPath(), QStringLiteral("/home/user/Music"));
    QCOMPARE(spy.count(), 1);
}

void TestSettingsViewModel::setTheme_persistsAndEmits()
{
    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    QSignalSpy spy(&vm, &SettingsViewModel::themeChanged);
    vm.setTheme("dark");

    QCOMPARE(vm.theme(), QStringLiteral("dark"));
    QCOMPARE(settingsRepo.get("theme"), QStringLiteral("dark"));
    QCOMPARE(spy.count(), 1);
}

void TestSettingsViewModel::setTheme_sameValue_noSignal()
{
    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    vm.setTheme("light"); // Same as default
    QSignalSpy spy(&vm, &SettingsViewModel::themeChanged);
    vm.setTheme("light"); // Same again

    QCOMPARE(spy.count(), 0);
}

void TestSettingsViewModel::setAudioQuality_persistsAndEmits()
{
    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    QSignalSpy spy(&vm, &SettingsViewModel::audioQualityChanged);
    vm.setAudioQuality(AudioQuality::Lossless);

    QCOMPARE(vm.audioQuality(), AudioQuality::Lossless);
    QCOMPARE(settingsRepo.get("audioQuality"), QStringLiteral("Lossless"));
    QCOMPARE(spy.count(), 1);
}

void TestSettingsViewModel::setDownloadPath_persistsAndEmits()
{
    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    QSignalSpy spy(&vm, &SettingsViewModel::downloadPathChanged);
    vm.setDownloadPath("/tmp/downloads");

    QCOMPARE(vm.downloadPath(), QStringLiteral("/tmp/downloads"));
    QCOMPARE(settingsRepo.get("downloadPath"), QStringLiteral("/tmp/downloads"));
    QCOMPARE(spy.count(), 1);
}

void TestSettingsViewModel::clearPlayHistory()
{
    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    vm.clearPlayHistory();
    QVERIFY(historyRepo.m_cleared);
}

void TestSettingsViewModel::clearError()
{
    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    // Set an error first
    vm.clearError(); // No-op when no error
    QVERIFY(!vm.hasError());

    QSignalSpy spy(&vm, &SettingsViewModel::errorChanged);
    vm.clearError();
    QCOMPARE(spy.count(), 1); // Still emits
}

void TestSettingsViewModel::isNeteaseLoggedIn_nullClient()
{
    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    // Should not crash and return false
    QVERIFY(!vm.isNeteaseLoggedIn());
}

void TestSettingsViewModel::setTheme_invalidValue()
{
    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    QSignalSpy spy(&vm, &SettingsViewModel::themeChanged);
    vm.setTheme("invalid");

    // Should not change theme or emit signal
    QCOMPARE(vm.theme(), QStringLiteral("light"));
    QCOMPARE(spy.count(), 0);
}

void TestSettingsViewModel::setTheme_validValues()
{
    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    QSignalSpy spy(&vm, &SettingsViewModel::themeChanged);

    vm.setTheme("dark");
    QCOMPARE(vm.theme(), QStringLiteral("dark"));
    QCOMPARE(spy.count(), 1);

    vm.setTheme("light");
    QCOMPARE(vm.theme(), QStringLiteral("light"));
    QCOMPARE(spy.count(), 2);
}

void TestSettingsViewModel::clearPlayHistory_repoException_doesNotCrash()
{
    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    historyRepo.m_throwOnClear = true;

    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    // Should not crash — exception is caught internally
    vm.clearPlayHistory();
}

QTEST_MAIN(TestSettingsViewModel)
#include "TestSettingsViewModel.moc"
