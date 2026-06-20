/// @file NeteaseClientAuth.cpp
/// @brief NeteaseClient — authentication, captcha, session management

#include "api/netease/NeteaseClient.h"

#include "api/common/ApiError.h"
#include "api/netease/NeteaseCrypto.h"
#include "api/netease/NeteaseParser.h"
#include "core/network/HttpClient.h"

#include <QJsonObject>
#include <QNetworkRequest>

namespace QeriPlayerQt {

// ─── Login ──────────────────────────────────────────────────────────────────

QCoro::Task<ApiResult<LoginResult>> NeteaseClient::login(const QString &phone, const QString &password, int ctcode)
{
    QJsonObject params;
    params[QLatin1String("phone")] = phone;
    params[QLatin1String("password")] = NeteaseCrypto::md5Hex(password);
    params[QLatin1String("countrycode")] = QString::number(ctcode);
    params[QLatin1String("rememberLogin")] = true;

    auto result = co_await makeEapiRequest(QStringLiteral("/w/login/cellphone"), params);
    if (result.isError()) {
        co_return ApiResult<LoginResult>(result.error());
    }

    LoginResult loginResult = NeteaseParser::parseLoginResult(result.data());
    persistCookies(loginResult.cookie);
    // Ensure cookies from Set-Cookie headers are captured (JSON body may not have them)
    co_await ensureWeapiSession();
    co_return ApiResult<LoginResult>(loginResult);
}

QCoro::Task<ApiResult<LoginResult>> NeteaseClient::loginByEmail(const QString &email, const QString &password)
{
    QJsonObject params;
    params[QLatin1String("email")] = email;
    params[QLatin1String("password")] = NeteaseCrypto::md5Hex(password);
    params[QLatin1String("rememberLogin")] = true;

    auto result = co_await makeEapiRequest(QStringLiteral("/w/login"), params);
    if (result.isError()) {
        co_return ApiResult<LoginResult>(result.error());
    }

    LoginResult loginResult = NeteaseParser::parseLoginResult(result.data());
    persistCookies(loginResult.cookie);
    co_await ensureWeapiSession();
    co_return ApiResult<LoginResult>(loginResult);
}

QCoro::Task<ApiResult<VoidResult>> NeteaseClient::logout()
{
    auto result = co_await makeRequest(QStringLiteral("/weapi/logout"));
    if (result.isError()) {
        co_return ApiResult<VoidResult>(result.error());
    }

    clearCookies();
    co_return ApiResult<VoidResult>(VoidResult {});
}

// ─── Cookie Management ─────────────────────────────────────────────────────

void NeteaseClient::setCookies(const QString &cookieString)
{
    persistCookies(cookieString);
}

QCoro::Task<void> NeteaseClient::ensureWeapiSession()
{
    // Visit homepage to get __csrf cookie (matches Kotlin ensureWeapiSession)
    QUrl url(QStringLiteral("https://music.163.com/"));
    QNetworkRequest request(url);
    request.setRawHeader("Referer", "https://music.163.com");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like "
                                       "Gecko) Chrome/120.0.0.0 Safari/537.36");
    injectCookies(request);

    auto response = co_await m_httpClient->get(request);

    // Extract Set-Cookie headers and merge into current cookie string
    QStringList newCookies;
    for (const auto &header : response.headers) {
        if (QString::fromUtf8(header.first).compare(QStringLiteral("Set-Cookie"), Qt::CaseInsensitive) == 0) {
            // Extract name=value from "name=value; Path=/; ..."
            QString nameValue = QString::fromUtf8(header.second).split(QLatin1Char(';')).first().trimmed();
            newCookies.append(nameValue);
        }
    }
    if (!newCookies.isEmpty()) {
        // Merge with existing cookies by rebuilding the cookie map
        QHash<QString, QString> cookieMap;
        // Parse existing cookies
        for (const QString &part : m_cookie.split(QStringLiteral("; "))) {
            QString name = part.section(QLatin1Char('='), 0, 0);
            QString value = part.section(QLatin1Char('='), 1);
            if (!name.isEmpty()) {
                cookieMap.insert(name, value);
            }
        }
        // Override with new cookies
        for (const QString &cookie : newCookies) {
            QString name = cookie.section(QLatin1Char('='), 0, 0);
            QString value = cookie.section(QLatin1Char('='), 1);
            cookieMap.insert(name, value);
        }
        // Rebuild cookie string
        QStringList parts;
        for (auto it = cookieMap.constBegin(); it != cookieMap.constEnd(); ++it) {
            parts.append(it.key() + QLatin1Char('=') + it.value());
        }
        persistCookies(parts.join(QStringLiteral("; ")));
    }
}

// ─── Account ────────────────────────────────────────────────────────────────

QCoro::Task<ApiResult<QJsonObject>> NeteaseClient::getCurrentUserAccount()
{
    auto result = co_await makeRequest(QStringLiteral("/weapi/w/nuser/account/get"));
    if (result.isError()) {
        co_return ApiResult<QJsonObject>(result.error());
    }

    co_return ApiResult<QJsonObject>(result.data());
}

QCoro::Task<ApiResult<long long>> NeteaseClient::getCurrentUserId()
{
    auto accountResult = co_await getCurrentUserAccount();
    if (accountResult.isError()) {
        co_return ApiResult<long long>(accountResult.error());
    }

    QJsonObject profile = accountResult.data()[QLatin1String("profile")].toObject();
    long long userId = profile[QLatin1String("userId")].toVariant().toLongLong();
    if (userId == 0) {
        co_return ApiResult<long long>(ApiError(-1, QStringLiteral("userId not found in profile")));
    }

    co_return ApiResult<long long>(userId);
}

// ─── Captcha Auth ───────────────────────────────────────────────────────────

QCoro::Task<ApiResult<LoginResult>> NeteaseClient::loginByCaptcha(const QString &phone, const QString &captcha,
                                                                  int ctcode)
{
    QJsonObject params;
    params[QLatin1String("phone")] = phone;
    params[QLatin1String("countrycode")] = QString::number(ctcode);
    params[QLatin1String("rememberLogin")] = QStringLiteral("true");
    params[QLatin1String("type")] = QStringLiteral("1");
    params[QLatin1String("captcha")] = captcha;

    auto result = co_await makeEapiRequest(QStringLiteral("/w/login/cellphone"), params);
    if (result.isError()) {
        co_return ApiResult<LoginResult>(result.error());
    }

    LoginResult loginResult = NeteaseParser::parseLoginResult(result.data());
    persistCookies(loginResult.cookie);
    co_await ensureWeapiSession();
    co_return ApiResult<LoginResult>(loginResult);
}

QCoro::Task<ApiResult<VoidResult>> NeteaseClient::sendCaptcha(const QString &phone, int ctcode)
{
    QJsonObject params;
    params[QLatin1String("cellphone")] = phone;
    params[QLatin1String("ctcode")] = QString::number(ctcode);

    auto result = co_await makeRequest(QStringLiteral("/weapi/sms/captcha/sent"), params);
    if (result.isError()) {
        co_return ApiResult<VoidResult>(result.error());
    }

    co_return ApiResult<VoidResult>(VoidResult {});
}

QCoro::Task<ApiResult<VoidResult>> NeteaseClient::verifyCaptcha(const QString &phone, const QString &captcha,
                                                                int ctcode)
{
    QJsonObject params;
    params[QLatin1String("cellphone")] = phone;
    params[QLatin1String("captcha")] = captcha;
    params[QLatin1String("ctcode")] = QString::number(ctcode);

    auto result = co_await makeRequest(QStringLiteral("/weapi/sms/captcha/verify"), params);
    if (result.isError()) {
        co_return ApiResult<VoidResult>(result.error());
    }

    co_return ApiResult<VoidResult>(VoidResult {});
}

// ─── QR Code Login ─────────────────────────────────────────────────────────

QCoro::Task<ApiResult<QrCodeData>> NeteaseClient::generateQrKey()
{
    // Get a unique key for QR login
    QJsonObject keyParams;
    keyParams[QLatin1String("type")] = 1;

    auto keyResult = co_await makeRequest(QStringLiteral("/weapi/login/qrcode/unikey"), keyParams);
    if (keyResult.isError()) {
        co_return ApiResult<QrCodeData>(keyResult.error());
    }

    QString unikey = keyResult.data()[QLatin1String("unikey")].toString();
    if (unikey.isEmpty()) {
        co_return ApiResult<QrCodeData>(ApiError(-1, QStringLiteral("Failed to get QR key")));
    }

    // The QR code URL for NetEase scan login
    QString qrData = QStringLiteral("https://music.163.com/st/platform/scanlogin?codekey=") + unikey
                     + QStringLiteral("&hdw_device=web&hdw_appid=web");
    QString qrImageUrl = QStringLiteral("https://api.qrserver.com/v1/create-qr-code/?size=200x200&data=")
                         + QUrl::toPercentEncoding(qrData);

    QrCodeData result;
    result.key = unikey;
    result.qrUrl = QUrl(qrImageUrl);
    result.expiresInSeconds = 300; // 5 minutes

    co_return ApiResult<QrCodeData>(result);
}

QCoro::Task<ApiResult<LoginResult>> NeteaseClient::pollQrStatus(const QString &key)
{
    QJsonObject params;
    params[QLatin1String("key")] = key;
    params[QLatin1String("type")] = 1;

    auto result = co_await makeRequest(QStringLiteral("/weapi/login/qrcode/client/login"), params);
    if (result.isError()) {
        co_return ApiResult<LoginResult>(result.error());
    }

    QJsonObject data = result.data();
    int code = data[QLatin1String("code")].toInt();

    // Status codes: 800=expired, 801=waiting, 802=scanned, 803=confirmed
    switch (code) {
        case 800: // Expired
            co_return ApiResult<LoginResult>(ApiError(800, QStringLiteral("QR code expired")));
        case 801: // Waiting for scan
            co_return ApiResult<LoginResult>(ApiError(801, QStringLiteral("Waiting for scan")));
        case 802: { // Scanned, waiting for confirm
            // Extract nickname from response if available
            LoginResult loginResult;
            loginResult.nickname = data[QLatin1String("nickname")].toString();
            loginResult.avatarUrl = QUrl(data[QLatin1String("avatarUrl")].toString());
            co_return ApiResult<LoginResult>(loginResult);
        }
        case 803: { // Confirmed — login success
            // Cookies are in HTTP response headers, extracted by makeRequest
            // Mark as authenticated
            m_authenticated = true;
            co_await ensureWeapiSession();

            // Get user info
            LoginResult loginResult;
            auto accountResult = co_await getCurrentUserAccount();
            if (accountResult.isSuccess()) {
                QJsonObject profile = accountResult.data()[QLatin1String("profile")].toObject();
                if (!profile.isEmpty()) {
                    loginResult.userId = QString::number(profile[QLatin1String("userId")].toVariant().toLongLong());
                    loginResult.nickname = profile[QLatin1String("nickname")].toString();
                    loginResult.avatarUrl = QUrl(profile[QLatin1String("avatarUrl")].toString());
                }
            }

            co_return ApiResult<LoginResult>(loginResult);
        }
        default:
            co_return ApiResult<LoginResult>(ApiError(code, QStringLiteral("Unknown QR status code: %1").arg(code)));
    }
}

} // namespace QeriPlayerQt
