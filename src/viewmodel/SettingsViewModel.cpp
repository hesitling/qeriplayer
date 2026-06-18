/// @file SettingsViewModel.cpp
/// @brief Implementation of SettingsViewModel

#include "viewmodel/SettingsViewModel.h"

#include "core/logger/Logger.h"

namespace QeriPlayerQt {

SettingsViewModel::SettingsViewModel(ISettingsRepository *settingsRepo, NeteaseClient *neteaseClient,
                                     IPlayHistoryRepository *historyRepo, QObject *parent)
    : QObject(parent)
    , m_settingsRepo(settingsRepo)
    , m_neteaseClient(neteaseClient)
    , m_historyRepo(historyRepo)
{
}

SettingsViewModel::~SettingsViewModel() = default;

// --- Getters ---

QString SettingsViewModel::theme() const
{
    return m_theme;
}

AudioQuality SettingsViewModel::audioQuality() const
{
    return m_audioQuality;
}

QString SettingsViewModel::downloadPath() const
{
    return m_downloadPath;
}

bool SettingsViewModel::isNeteaseLoggedIn() const
{
    if (!m_neteaseClient) {
        return false;
    }
    return m_neteaseClient->isAuthenticated();
}

QString SettingsViewModel::neteaseUsername() const
{
    return m_neteaseUsername;
}

bool SettingsViewModel::hasError() const
{
    return m_hasError;
}

ViewModelError SettingsViewModel::error() const
{
    return m_error;
}

// --- Settings ---

void SettingsViewModel::loadSettings()
{
    try {
        auto themeVal = m_settingsRepo->get(QStringLiteral("theme"));
        if (themeVal.has_value()) {
            m_theme = themeVal.value();
            Q_EMIT themeChanged();
        }
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to load theme setting: {}", ex.what());
    }

    try {
        auto qualityVal = m_settingsRepo->get(QStringLiteral("audioQuality"));
        if (qualityVal.has_value()) {
            const QString &val = qualityVal.value();
            if (val == QStringLiteral("Low")) {
                m_audioQuality = AudioQuality::Low;
            } else if (val == QStringLiteral("Standard")) {
                m_audioQuality = AudioQuality::Standard;
            } else if (val == QStringLiteral("Lossless")) {
                m_audioQuality = AudioQuality::Lossless;
            } else {
                m_audioQuality = AudioQuality::High;
            }
            Q_EMIT audioQualityChanged();
        }
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to load audioQuality setting: {}", ex.what());
    }

    try {
        auto pathVal = m_settingsRepo->get(QStringLiteral("downloadPath"));
        if (pathVal.has_value()) {
            m_downloadPath = pathVal.value();
            Q_EMIT downloadPathChanged();
        }
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to load downloadPath setting: {}", ex.what());
    }
}

void SettingsViewModel::setTheme(const QString &theme)
{
    if (m_theme == theme) {
        return;
    }

    // Validate supported themes
    if (theme != QStringLiteral("light") && theme != QStringLiteral("dark")) {
        return;
    }

    m_theme = theme;
    try {
        m_settingsRepo->set(QStringLiteral("theme"), theme);
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to save theme setting: {}", ex.what());
    }
    Q_EMIT themeChanged();
}

void SettingsViewModel::setAudioQuality(AudioQuality quality)
{
    if (m_audioQuality == quality) {
        return;
    }
    m_audioQuality = quality;

    QString qualityStr;
    switch (quality) {
        case AudioQuality::Low:
            qualityStr = QStringLiteral("Low");
            break;
        case AudioQuality::Standard:
            qualityStr = QStringLiteral("Standard");
            break;
        case AudioQuality::High:
            qualityStr = QStringLiteral("High");
            break;
        case AudioQuality::Lossless:
            qualityStr = QStringLiteral("Lossless");
            break;
    }
    try {
        m_settingsRepo->set(QStringLiteral("audioQuality"), qualityStr);
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to save audioQuality setting: {}", ex.what());
    }
    Q_EMIT audioQualityChanged();
}

void SettingsViewModel::setDownloadPath(const QString &path)
{
    if (m_downloadPath == path) {
        return;
    }
    m_downloadPath = path;
    try {
        m_settingsRepo->set(QStringLiteral("downloadPath"), path);
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to save downloadPath setting: {}", ex.what());
    }
    Q_EMIT downloadPathChanged();
}

// --- Auth ---

QCoro::Task<void> SettingsViewModel::loginNetease(const QString &phone, const QString &password)
{
    if (!m_neteaseClient) {
        m_error = ViewModelError(ViewModelError::ErrorType::Api, "NetEase client not available");
        m_hasError = true;
        Q_EMIT errorChanged();
        co_return;
    }

    auto result = co_await m_neteaseClient->login(phone, password);
    if (result.isError()) {
        m_error = ViewModelError::fromApiError(result.error());
        m_hasError = true;
        Q_EMIT errorChanged();
        co_return;
    }

    m_hasError = false;
    m_neteaseUsername = result.data().nickname;
    Q_EMIT neteaseAuthChanged();
    Q_EMIT errorChanged();
}

QCoro::Task<void> SettingsViewModel::logoutNetease()
{
    if (!m_neteaseClient) {
        m_error = ViewModelError(ViewModelError::ErrorType::Api, "NetEase client not available");
        m_hasError = true;
        Q_EMIT errorChanged();
        co_return;
    }

    auto result = co_await m_neteaseClient->logout();
    if (result.isError()) {
        m_error = ViewModelError::fromApiError(result.error());
        m_hasError = true;
        Q_EMIT errorChanged();
        co_return;
    }

    m_hasError = false;
    m_neteaseUsername.clear();
    Q_EMIT neteaseAuthChanged();
    Q_EMIT errorChanged();
}

// --- History ---

void SettingsViewModel::clearPlayHistory()
{
    try {
        m_historyRepo->clear();
    } catch (const std::exception &ex) {
        Logger::get("viewmodel")->warn("Failed to clear play history: {}", ex.what());
    }
}

// --- Error ---

void SettingsViewModel::clearError()
{
    m_hasError = false;
    m_error = ViewModelError();
    Q_EMIT errorChanged();
}

} // namespace QeriPlayerQt
