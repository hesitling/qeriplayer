## MODIFIED Requirements

### Requirement: NetEase login
`Q_INVOKABLE QCoro::QmlTask loginNetease(const QString &phone, const QString &password)` SHALL call `NeteaseClient::login()`. On success, `isNeteaseLoggedIn` SHALL become `true` and `neteaseAuthChanged` SHALL be emitted. The method SHALL return a `QCoro::QmlTask` that completes when the login operation finishes, enabling `.then()` chaining in QML.

#### Scenario: Successful login with QmlTask
- **WHEN** QML calls `settingsVm.loginNetease("13800138000", "password").then(callback)`
- **THEN** the callback SHALL fire after `isNeteaseLoggedIn` becomes `true` and `neteaseUsername` is populated

#### Scenario: Failed login with QmlTask
- **WHEN** QML calls `settingsVm.loginNetease("13800138000", "wrong").then(callback)` and login fails
- **THEN** the callback SHALL fire after `hasError` becomes `true` and `error.type` is `Auth`

### Requirement: NetEase logout
`Q_INVOKABLE QCoro::QmlTask logoutNetease()` SHALL call `NeteaseClient::logout()`. On completion, `isNeteaseLoggedIn` SHALL become `false`. The method SHALL return a `QCoro::QmlTask` that completes when the logout operation finishes, enabling `.then()` chaining in QML.

#### Scenario: Logout with QmlTask
- **WHEN** QML calls `settingsVm.logoutNetease().then(callback)`
- **THEN** the callback SHALL fire after `isNeteaseLoggedIn` becomes `false` and `neteaseAuthChanged` is emitted
