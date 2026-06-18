/// @file SettingsViewModel.h
/// @brief ViewModel for settings management and platform authentication

#ifndef QERIPLAYERQT_SETTINGSVIEWMODEL_H
#define QERIPLAYERQT_SETTINGSVIEWMODEL_H

#include "api/netease/NeteaseClient.h"
#include "domain/Enums.h"
#include "repo/IPlayHistoryRepository.h"
#include "repo/ISettingsRepository.h"
#include "viewmodel/ViewModelError.h"

#include <QCoroTask>
#include <QObject>

namespace QeriPlayerQt {

/**
 * @brief ViewModel managing settings persistence and platform auth
 *
 * Reads/writes settings via ISettingsRepository.
 * Manages NeteaseClient login/logout/auth status.
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

    // --- Settings ---
    Q_INVOKABLE void loadSettings();
    Q_INVOKABLE void setTheme(const QString &theme);
    Q_INVOKABLE void setAudioQuality(QeriPlayerQt::AudioQuality quality);
    Q_INVOKABLE void setDownloadPath(const QString &path);

    // --- Auth ---
    Q_INVOKABLE QCoro::Task<void> loginNetease(const QString &phone, const QString &password);
    Q_INVOKABLE QCoro::Task<void> logoutNetease();

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
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_SETTINGSVIEWMODEL_H
