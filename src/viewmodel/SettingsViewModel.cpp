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
    , m_captchaCooldownTimer(new QTimer(this))
{
    m_captchaCooldownTimer->setInterval(1000);
    connect(m_captchaCooldownTimer, &QTimer::timeout, this, [this]() {
        if (m_captchaCooldown > 0) {
            m_captchaCooldown--;
            Q_EMIT captchaCooldownChanged();
            if (m_captchaCooldown == 0) {
                m_captchaCooldownTimer->stop();
                Q_EMIT canSendCaptchaChanged();
            }
        }
    });
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

int SettingsViewModel::captchaCooldown() const
{
    return m_captchaCooldown;
}

bool SettingsViewModel::canSendCaptcha() const
{
    return m_captchaCooldown == 0;
}

QrLoginStatus SettingsViewModel::qrLoginStatus() const
{
    return m_qrLoginStatus;
}

QUrl SettingsViewModel::qrImageUrl() const
{
    return m_qrImageUrl;
}

QString SettingsViewModel::qrKey() const
{
    return m_qrKey;
}

bool SettingsViewModel::qrPollingActive() const
{
    return m_qrPollingActive;
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

// --- Auth: Password ---

QCoro::QmlTask SettingsViewModel::loginByPassword(const QString &phone, const QString &password)
{
    return QCoro::QmlTask(loginByPasswordImpl(phone, password));
}

QCoro::Task<void> SettingsViewModel::loginByPasswordImpl(const QString &phone, const QString &password)
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

// --- Auth: Captcha ---

QCoro::QmlTask SettingsViewModel::sendCaptcha(const QString &phone)
{
    return QCoro::QmlTask(sendCaptchaImpl(phone));
}

QCoro::Task<void> SettingsViewModel::sendCaptchaImpl(const QString &phone)
{
    if (!m_neteaseClient) {
        m_error = ViewModelError(ViewModelError::ErrorType::Api, "NetEase client not available");
        m_hasError = true;
        Q_EMIT errorChanged();
        co_return;
    }

    auto result = co_await m_neteaseClient->sendCaptcha(phone);
    if (result.isError()) {
        m_error = ViewModelError::fromApiError(result.error());
        m_hasError = true;
        Q_EMIT errorChanged();
        co_return;
    }

    m_hasError = false;
    m_captchaCooldown = 60;
    m_captchaCooldownTimer->start();
    Q_EMIT captchaCooldownChanged();
    Q_EMIT canSendCaptchaChanged();
    Q_EMIT errorChanged();
}

QCoro::QmlTask SettingsViewModel::loginByCaptcha(const QString &phone, const QString &captcha)
{
    return QCoro::QmlTask(loginByCaptchaImpl(phone, captcha));
}

QCoro::Task<void> SettingsViewModel::loginByCaptchaImpl(const QString &phone, const QString &captcha)
{
    if (!m_neteaseClient) {
        m_error = ViewModelError(ViewModelError::ErrorType::Api, "NetEase client not available");
        m_hasError = true;
        Q_EMIT errorChanged();
        co_return;
    }

    auto result = co_await m_neteaseClient->loginByCaptcha(phone, captcha);
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

// --- Auth: QR Code ---

QCoro::QmlTask SettingsViewModel::generateQrLogin()
{
    return QCoro::QmlTask(generateQrLoginImpl());
}

QCoro::Task<void> SettingsViewModel::generateQrLoginImpl()
{
    if (!m_neteaseClient) {
        m_error = ViewModelError(ViewModelError::ErrorType::Api, "NetEase client not available");
        m_hasError = true;
        Q_EMIT errorChanged();
        co_return;
    }

    m_hasError = false;
    Q_EMIT errorChanged();

    auto result = co_await m_neteaseClient->generateQrKey();
    if (result.isError()) {
        m_error = ViewModelError::fromApiError(result.error());
        m_hasError = true;
        Q_EMIT errorChanged();
        co_return;
    }

    m_qrKey = result.data().key;
    m_qrImageUrl = result.data().qrUrl;
    m_qrLoginStatus = QrLoginStatus::Waiting;
    m_qrPollingActive = true;
    Q_EMIT qrKeyChanged();
    Q_EMIT qrImageUrlChanged();
    Q_EMIT qrLoginStatusChanged();
    Q_EMIT qrPollingActiveChanged();
}

QCoro::QmlTask SettingsViewModel::pollQrLogin()
{
    return QCoro::QmlTask(pollQrLoginImpl());
}

QCoro::Task<void> SettingsViewModel::pollQrLoginImpl()
{
    if (!m_qrPollingActive || !m_neteaseClient) {
        co_return;
    }

    auto result = co_await m_neteaseClient->pollQrStatus(m_qrKey);
    if (!m_qrPollingActive) {
        // Cancelled while awaiting
        co_return;
    }

    if (result.isError()) {
        int code = result.error().code();
        switch (code) {
            case 800: // Expired
                m_qrLoginStatus = QrLoginStatus::Expired;
                m_qrPollingActive = false;
                Q_EMIT qrLoginStatusChanged();
                Q_EMIT qrPollingActiveChanged();
                break;
            case 801: // Waiting
                m_qrLoginStatus = QrLoginStatus::Waiting;
                Q_EMIT qrLoginStatusChanged();
                break;
            default:
                // Other errors — keep polling
                break;
        }
        co_return;
    }

    // Success — either 802 (scanned) or 803 (confirmed)
    if (!result.data().nickname.isEmpty()) {
        m_neteaseUsername = result.data().nickname;
    }

    if (m_neteaseClient->isAuthenticated()) {
        // 803 — Confirmed, login success
        m_qrLoginStatus = QrLoginStatus::Confirmed;
        m_qrPollingActive = false;
        m_hasError = false;
        Q_EMIT qrLoginStatusChanged();
        Q_EMIT qrPollingActiveChanged();
        Q_EMIT neteaseAuthChanged();
        Q_EMIT errorChanged();
    } else {
        // 802 — Scanned, waiting for confirmation
        m_qrLoginStatus = QrLoginStatus::Scanned;
        Q_EMIT qrLoginStatusChanged();
    }
}

void SettingsViewModel::cancelQrLogin()
{
    m_qrPollingActive = false;
    Q_EMIT qrPollingActiveChanged();
}

// --- Auth: Logout ---

QCoro::QmlTask SettingsViewModel::logoutNetease()
{
    return QCoro::QmlTask(logoutNeteaseImpl());
}

QCoro::Task<void> SettingsViewModel::logoutNeteaseImpl()
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
