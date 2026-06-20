/// @file SettingsViewModel.h
/// @brief ViewModel for settings management and platform authentication

#ifndef QERIPLAYERQT_SETTINGSVIEWMODEL_H
#define QERIPLAYERQT_SETTINGSVIEWMODEL_H

#include "api/netease/NeteaseClient.h"
#include "domain/Enums.h"
#include "repo/IPlayHistoryRepository.h"
#include "repo/ISettingsRepository.h"
#include "viewmodel/ViewModelError.h"

#include <QCoroQmlTask>
#include <QCoroTask>
#include <QObject>
#include <QTimer>
#include <QUrl>

namespace QeriPlayerQt {

/**
 * @brief ViewModel managing settings persistence and platform auth
 *
 * Reads/writes settings via ISettingsRepository.
 * Manages NeteaseClient login/logout/auth status.
 * Supports password, captcha, and QR code login methods.
 */
class SettingsViewModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(AudioQuality audioQuality READ audioQuality WRITE setAudioQuality NOTIFY audioQualityChanged)
    Q_PROPERTY(QString downloadPath READ downloadPath WRITE setDownloadPath NOTIFY downloadPathChanged)
    Q_PROPERTY(bool isNeteaseLoggedIn READ isNeteaseLoggedIn NOTIFY neteaseAuthChanged)
    Q_PROPERTY(QString neteaseUsername READ neteaseUsername NOTIFY neteaseAuthChanged)
    Q_PROPERTY(bool hasError READ hasError NOTIFY errorChanged)
    Q_PROPERTY(ViewModelError error READ error NOTIFY errorChanged)
    Q_PROPERTY(int captchaCooldown READ captchaCooldown NOTIFY captchaCooldownChanged)
    Q_PROPERTY(bool canSendCaptcha READ canSendCaptcha NOTIFY canSendCaptchaChanged)
    Q_PROPERTY(QrLoginStatus qrLoginStatus READ qrLoginStatus NOTIFY qrLoginStatusChanged)
    Q_PROPERTY(QUrl qrImageUrl READ qrImageUrl NOTIFY qrImageUrlChanged)
    Q_PROPERTY(QString qrKey READ qrKey NOTIFY qrKeyChanged)
    Q_PROPERTY(bool qrPollingActive READ qrPollingActive NOTIFY qrPollingActiveChanged)

public:
    explicit SettingsViewModel(ISettingsRepository *settingsRepo, NeteaseClient *neteaseClient,
                               IPlayHistoryRepository *historyRepo, QObject *parent = nullptr);
    ~SettingsViewModel() override;

    // --- Getters ---
    QString theme() const;
    AudioQuality audioQuality() const;
    QString downloadPath() const;
    bool isNeteaseLoggedIn() const;
    QString neteaseUsername() const;
    bool hasError() const;
    ViewModelError error() const;
    int captchaCooldown() const;
    bool canSendCaptcha() const;
    QrLoginStatus qrLoginStatus() const;
    QUrl qrImageUrl() const;
    QString qrKey() const;
    bool qrPollingActive() const;

    // --- Settings ---
    Q_INVOKABLE void loadSettings();
    Q_INVOKABLE void setTheme(const QString &theme);
    Q_INVOKABLE void setAudioQuality(QeriPlayerQt::AudioQuality quality);
    Q_INVOKABLE void setDownloadPath(const QString &path);

    // --- Auth: Password ---
    Q_INVOKABLE QCoro::QmlTask loginByPassword(const QString &phone, const QString &password);

    // --- Auth: Captcha ---
    Q_INVOKABLE QCoro::QmlTask sendCaptcha(const QString &phone);
    Q_INVOKABLE QCoro::QmlTask loginByCaptcha(const QString &phone, const QString &captcha);

    // --- Auth: QR Code ---
    Q_INVOKABLE QCoro::QmlTask generateQrLogin();
    Q_INVOKABLE QCoro::QmlTask pollQrLogin();
    Q_INVOKABLE void cancelQrLogin();

    // --- Auth: Logout ---
    Q_INVOKABLE QCoro::QmlTask logoutNetease();

    // --- History ---
    Q_INVOKABLE void clearPlayHistory();

    // --- Error ---
    Q_INVOKABLE void clearError();

Q_SIGNALS:
    void themeChanged();
    void audioQualityChanged();
    void downloadPathChanged();
    void neteaseAuthChanged();
    void errorChanged();
    void captchaCooldownChanged();
    void canSendCaptchaChanged();
    void qrLoginStatusChanged();
    void qrImageUrlChanged();
    void qrKeyChanged();
    void qrPollingActiveChanged();

private:
    ISettingsRepository *m_settingsRepo;
    NeteaseClient *m_neteaseClient;
    IPlayHistoryRepository *m_historyRepo;

    QString m_theme = QStringLiteral("light");
    AudioQuality m_audioQuality = AudioQuality::High;
    QString m_downloadPath;
    QString m_neteaseUsername;
    ViewModelError m_error;
    bool m_hasError = false;

    // Captcha
    int m_captchaCooldown = 0;
    QTimer *m_captchaCooldownTimer;

    // QR Code
    QrLoginStatus m_qrLoginStatus = QrLoginStatus::Waiting;
    QUrl m_qrImageUrl;
    QString m_qrKey;
    bool m_qrPollingActive = false;

    QCoro::Task<void> loginByPasswordImpl(const QString &phone, const QString &password);
    QCoro::Task<void> sendCaptchaImpl(const QString &phone);
    QCoro::Task<void> loginByCaptchaImpl(const QString &phone, const QString &captcha);
    QCoro::Task<void> generateQrLoginImpl();
    QCoro::Task<void> pollQrLoginImpl();
    QCoro::Task<void> logoutNeteaseImpl();
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_SETTINGSVIEWMODEL_H
