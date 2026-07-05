/// @file TestSettingsViewModel.cpp
/// @brief Unit tests for SettingsViewModel

#include "api/common/ApiError.h"
#include "core/network/HttpClient.h"
#include "repo/IPlayHistoryRepository.h"
#include "repo/ISettingsRepository.h"
#include "viewmodel/SettingsViewModel.h"

#include <QSignalSpy>
#include <QTest>

using namespace QeriPlayerQt;

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

class FakeNeteaseClient : public NeteaseClient {
public:
    explicit FakeNeteaseClient(HttpClient *httpClient)
        : NeteaseClient(httpClient)
    {
    }

    bool isAuthenticated() const override
    {
        return m_authenticatedState;
    }

    QCoro::Task<ApiResult<LoginResult>> importCookies(const QString &cookieString) override
    {
        m_lastImportedCookie = cookieString;
        if (m_importError.has_value()) {
            m_authenticatedState = false;
            co_return ApiResult<LoginResult>(m_importError.value());
        }

        m_authenticatedState = true;
        co_return ApiResult<LoginResult>(m_importResult);
    }

    QCoro::Task<ApiResult<QJsonObject>> getCurrentUserAccount() override
    {
        if (m_accountError.has_value()) {
            co_return ApiResult<QJsonObject>(m_accountError.value());
        }
        co_return ApiResult<QJsonObject>(m_accountResponse);
    }

    void clearLocalSession() override
    {
        m_clearLocalSessionCount++;
        m_authenticatedState = false;
    }

    QCoro::Task<void> ensureWeapiSession() override
    {
        m_ensureWeapiSessionCount++;
        co_return;
    }

    QString m_lastImportedCookie;
    int m_clearLocalSessionCount = 0;
    int m_ensureWeapiSessionCount = 0;
    bool m_authenticatedState = false;
    LoginResult m_importResult;
    std::optional<ApiError> m_importError;
    QJsonObject m_accountResponse;
    std::optional<ApiError> m_accountError;
};

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
    void importCookie_success_setsAuthState();
    void importCookie_failure_clearsSessionAndSetsError();
    void logout_localOnly_clearsSession();
    void loadSettings_hydratesRestoredProfile();
    void loadSettings_refreshesWeapiSessionBeforeHydration();
    void loadSettings_authFailureClearsRestoredSession();
};

void TestSettingsViewModel::initialState()
{
    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    QCOMPARE(vm.theme(), QStringLiteral("light"));
    QCOMPARE(vm.audioQuality(), AudioQuality::High);
    QVERIFY(vm.downloadPath().isEmpty());
    QVERIFY(!vm.hasError());
    QVERIFY(vm.neteaseUsername().isEmpty());
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

    vm.setTheme("light");
    QSignalSpy spy(&vm, &SettingsViewModel::themeChanged);
    vm.setTheme("light");

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

    vm.clearError();
    QVERIFY(!vm.hasError());

    QSignalSpy spy(&vm, &SettingsViewModel::errorChanged);
    vm.clearError();
    QCOMPARE(spy.count(), 1);
}

void TestSettingsViewModel::isNeteaseLoggedIn_nullClient()
{
    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    QVERIFY(!vm.isNeteaseLoggedIn());
}

void TestSettingsViewModel::setTheme_invalidValue()
{
    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, nullptr, &historyRepo);

    QSignalSpy spy(&vm, &SettingsViewModel::themeChanged);
    vm.setTheme("invalid");

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
    vm.clearPlayHistory();
}

void TestSettingsViewModel::importCookie_success_setsAuthState()
{
    HttpClient http;
    FakeNeteaseClient client(&http);
    client.m_importResult.nickname = QStringLiteral("TestUser");
    client.m_importResult.userId = QStringLiteral("42");

    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, &client, &historyRepo);

    QSignalSpy authSpy(&vm, &SettingsViewModel::neteaseAuthChanged);
    auto importTask = vm.importNeteaseCookie(QStringLiteral("MUSIC_U=abc; __csrf=xyz"));
    Q_UNUSED(importTask);

    QTRY_VERIFY(vm.isNeteaseLoggedIn());
    QCOMPARE(vm.neteaseUsername(), QStringLiteral("TestUser"));
    QCOMPARE(client.m_lastImportedCookie, QStringLiteral("MUSIC_U=abc; __csrf=xyz"));
    QVERIFY(!vm.hasError());
    QCOMPARE(authSpy.count(), 1);
}

void TestSettingsViewModel::importCookie_failure_clearsSessionAndSetsError()
{
    HttpClient http;
    FakeNeteaseClient client(&http);
    client.m_importError = ApiError(401, QStringLiteral("Cookie is invalid or expired"));

    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, &client, &historyRepo);

    auto importTask = vm.importNeteaseCookie(QStringLiteral("MUSIC_U=bad"));
    Q_UNUSED(importTask);

    QTRY_VERIFY(vm.hasError());
    QVERIFY(!vm.isNeteaseLoggedIn());
    QCOMPARE(vm.error().type(), ViewModelError::ErrorType::Auth);
    QCOMPARE(client.m_clearLocalSessionCount, 1);
}

void TestSettingsViewModel::logout_localOnly_clearsSession()
{
    HttpClient http;
    FakeNeteaseClient client(&http);
    client.m_authenticatedState = true;

    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, &client, &historyRepo);

    auto importTask = vm.importNeteaseCookie(QStringLiteral("MUSIC_U=abc; __csrf=xyz"));
    Q_UNUSED(importTask);
    QTRY_VERIFY(vm.isNeteaseLoggedIn());
    client.m_clearLocalSessionCount = 0;

    auto logoutTask = vm.logoutNetease();
    Q_UNUSED(logoutTask);

    QTRY_VERIFY(!vm.isNeteaseLoggedIn());
    QVERIFY(vm.neteaseUsername().isEmpty());
    QCOMPARE(client.m_clearLocalSessionCount, 1);
}

void TestSettingsViewModel::loadSettings_hydratesRestoredProfile()
{
    HttpClient http;
    FakeNeteaseClient client(&http);
    client.m_authenticatedState = true;

    QJsonObject profile;
    profile[QLatin1String("nickname")] = QStringLiteral("RestoredUser");
    profile[QLatin1String("userId")] = 7;
    QJsonObject account;
    account[QLatin1String("profile")] = profile;
    client.m_accountResponse = account;

    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, &client, &historyRepo);

    vm.loadSettings();

    QTRY_VERIFY(vm.isNeteaseLoggedIn());
    QTRY_COMPARE(vm.neteaseUsername(), QStringLiteral("RestoredUser"));
}

void TestSettingsViewModel::loadSettings_refreshesWeapiSessionBeforeHydration()
{
    HttpClient http;
    FakeNeteaseClient client(&http);
    client.m_authenticatedState = true;

    QJsonObject profile;
    profile[QLatin1String("nickname")] = QStringLiteral("RestoredUser");
    profile[QLatin1String("userId")] = 7;
    QJsonObject account;
    account[QLatin1String("profile")] = profile;
    client.m_accountResponse = account;

    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, &client, &historyRepo);

    vm.loadSettings();

    QTRY_COMPARE(client.m_ensureWeapiSessionCount, 1);
    QTRY_COMPARE(vm.neteaseUsername(), QStringLiteral("RestoredUser"));
}

void TestSettingsViewModel::loadSettings_authFailureClearsRestoredSession()
{
    HttpClient http;
    FakeNeteaseClient client(&http);
    client.m_authenticatedState = true;
    client.m_accountError = ApiError(401, QStringLiteral("Expired"));

    MockSettingsRepo settingsRepo;
    MockPlayHistoryRepo historyRepo;
    SettingsViewModel vm(&settingsRepo, &client, &historyRepo);

    vm.loadSettings();

    QTRY_VERIFY(!vm.isNeteaseLoggedIn());
    QVERIFY(vm.neteaseUsername().isEmpty());
    QTRY_COMPARE(client.m_clearLocalSessionCount, 1);
}

QTEST_MAIN(TestSettingsViewModel)
#include "TestSettingsViewModel.moc"
