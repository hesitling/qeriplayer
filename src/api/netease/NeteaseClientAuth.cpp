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

} // namespace QeriPlayerQt
