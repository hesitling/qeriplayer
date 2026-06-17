/// @file NeteaseClientCore.cpp
/// @brief NeteaseClient — constructor, configuration, request helpers

#include "api/netease/NeteaseClient.h"

#include "api/common/ApiError.h"
#include "api/netease/NeteaseCrypto.h"
#include "core/crypto/SecureStorage.h"
#include "core/logger/Logger.h"
#include "core/network/HttpClient.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrlQuery>

namespace NeriPlayerQt {

static const QUrl DEFAULT_BASE_URL(QStringLiteral("https://music.163.com"));
static const QString COOKIE_STORAGE_KEY = QStringLiteral("netease_cookie");

NeteaseClient::NeteaseClient(HttpClient *httpClient, SecureStorage *storage, QObject *parent)
    : QObject(parent)
    , m_httpClient(httpClient)
    , m_storage(storage)
    , m_baseUrl(DEFAULT_BASE_URL)
{
    Q_ASSERT(m_httpClient);

    // Restore cookies from secure storage
    if (m_storage) {
        auto cookieOpt = m_storage->get(COOKIE_STORAGE_KEY);
        if (cookieOpt.has_value()) {
            m_cookie = cookieOpt.value();
            // Extract CSRF token from cookie
            const QStringList parts = m_cookie.split(QLatin1String("; "));
            for (const QString &part : parts) {
                if (part.startsWith(QLatin1String("__csrf="))) {
                    m_csrfToken = part.mid(7);
                    break;
                }
            }
            m_authenticated = !m_csrfToken.isEmpty();
            if (m_authenticated) {
                Logger::get("api")->info("NeteaseClient: restored session from storage");
            }
        }
    }
}

// ─── Configuration ──────────────────────────────────────────────────────────

void NeteaseClient::setBaseUrl(const QUrl &url)
{
    m_baseUrl = url;
}

// ─── Request Helpers ────────────────────────────────────────────────────────

QCoro::Task<ApiResult<QJsonObject>> NeteaseClient::makeRequest(const QString &path, const QJsonObject &params,
                                                               const QString &host, bool retried)
{
    // Build the encrypted payload
    QJsonObject weapiParams = params;
    if (!m_csrfToken.isEmpty()) {
        weapiParams[QLatin1String("csrf_token")] = m_csrfToken;
    }

    QByteArray payload = QJsonDocument(weapiParams).toJson(QJsonDocument::Compact);
    auto encrypted = NeteaseCrypto::weapiEncrypt(QString::fromUtf8(payload));

    // Build URL-encoded form body manually to ensure + is encoded as %2B
    // QUrlQuery doesn't encode + to %2B, but in form encoding + means space
    QByteArray postData;
    postData += "params=";
    postData += QUrl::toPercentEncoding(encrypted.params);
    postData += "&encSecKey=";
    postData += QUrl::toPercentEncoding(encrypted.encSecKey);

    // Build URL — use explicit host if provided, otherwise resolve against base
    QUrl url = host.isEmpty() ? m_baseUrl.resolved(QUrl(path)) : QUrl(host + path);
    if (!m_csrfToken.isEmpty()) {
        QUrlQuery query(url.query());
        query.addQueryItem(QStringLiteral("csrf_token"), m_csrfToken);
        url.setQuery(query);
    }

    // Build request with headers
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    request.setRawHeader("Referer", "https://music.163.com");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like "
                                       "Gecko) Chrome/120.0.0.0 Safari/537.36");
    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Accept-Language", "zh-CN,zh-Hans;q=0.9");
    // Note: do NOT set Accept-Encoding — Qt 6 QNetworkAccessManager doesn't
    // decompress gzip transparently, so we let the server return plain JSON.

    // Inject cookies
    injectCookies(request);

    // Send request with all headers preserved
    auto response = co_await m_httpClient->post(request, postData);

    if (!response.isSuccess()) {
        Logger::get("api")->warn("NeteaseClient: HTTP error at path {}: {} ({})", path.toStdString(),
                                 response.errorString.toStdString(), response.statusCode);
        co_return ApiResult<QJsonObject>(ApiError(response.statusCode, response.errorString));
    }

    // Parse JSON response
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(response.body, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        Logger::get("api")->warn("NeteaseClient: JSON parse error at path {}: {} (body: {})", path.toStdString(),
                                 parseError.errorString().toStdString(), response.body.left(200).toStdString());
        co_return ApiResult<QJsonObject>(
            ApiError(-1, QStringLiteral("Invalid JSON response"), parseError.errorString()));
    }

    QJsonObject json = doc.object();

    // Check for API-level errors
    int code = json[QLatin1String("code")].toInt();
    if (code != 200) {
        // Auto-retry 301 (session expired) once, like Kotlin
        if (code == 301 && !retried && isAuthenticated()) {
            Logger::get("api")->info("NeteaseClient: 301 at {}, refreshing session", path.toStdString());
            co_await ensureWeapiSession();
            co_return co_await makeRequest(path, params, host, true);
        }
        QString msg = json[QLatin1String("msg")].toString();
        if (msg.isEmpty()) {
            msg = json[QLatin1String("message")].toString();
        }
        Logger::get("api")->warn("NeteaseClient: API error {} at {}: {}", code, path.toStdString(), msg.toStdString());
        co_return ApiResult<QJsonObject>(ApiError(code, msg));
    }

    co_return ApiResult<QJsonObject>(json);
}

QCoro::Task<ApiResult<QJsonObject>> NeteaseClient::makeUnencryptedRequest(const QString &path,
                                                                          const QJsonObject &params)
{
    QUrl url = m_baseUrl.resolved(QUrl(path));

    // Inject CSRF token into URL query (like makeRequest)
    if (!m_csrfToken.isEmpty()) {
        QUrlQuery csrfQuery(url.query());
        csrfQuery.addQueryItem(QStringLiteral("csrf_token"), m_csrfToken);
        url.setQuery(csrfQuery);
    }

    QUrlQuery query;
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        query.addQueryItem(it.key(), it.value().toVariant().toString());
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    request.setRawHeader("Referer", "https://music.163.com");
    injectCookies(request);

    auto response = co_await m_httpClient->post(request, query.toString(QUrl::FullyEncoded).toUtf8());

    if (!response.isSuccess()) {
        co_return ApiResult<QJsonObject>(ApiError(response.statusCode, response.errorString));
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(response.body, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        co_return ApiResult<QJsonObject>(
            ApiError(-1, QStringLiteral("Invalid JSON response"), parseError.errorString()));
    }

    QJsonObject json = doc.object();
    int code = json[QLatin1String("code")].toInt();
    if (code != 200) {
        QString msg = json[QLatin1String("msg")].toString();
        if (msg.isEmpty()) {
            msg = json[QLatin1String("message")].toString();
        }
        co_return ApiResult<QJsonObject>(ApiError(code, msg));
    }

    co_return ApiResult<QJsonObject>(json);
}

QCoro::Task<ApiResult<QJsonObject>> NeteaseClient::makeEapiRequest(const QString &path, const QJsonObject &params,
                                                                   const QString &host, bool returnRawOnNon200,
                                                                   bool retried)
{
    // Build JSON payload
    QByteArray payload = QJsonDocument(params).toJson(QJsonDocument::Compact);

    // EAPI encrypt
    QString eapiPath = QStringLiteral("/eapi") + path;
    QString encryptedParams = NeteaseCrypto::eapiEncrypt(eapiPath, QString::fromUtf8(payload));

    // Build form body
    QUrlQuery formBody;
    formBody.addQueryItem(QStringLiteral("params"), encryptedParams);
    QByteArray postData = formBody.toString(QUrl::FullyEncoded).toUtf8();

    // Build URL using the specified host
    QUrl url(host + eapiPath);

    // Build request
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    request.setRawHeader("Referer", "https://music.163.com");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like "
                                       "Gecko) Chrome/120.0.0.0 Safari/537.36");
    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Accept-Language", "zh-CN,zh-Hans;q=0.9");

    // Inject cookies
    injectCookies(request);

    // Send request with all headers preserved
    auto response = co_await m_httpClient->post(request, postData);

    if (!response.isSuccess()) {
        Logger::get("api")->warn("NeteaseClient: EAPI HTTP error at {}: {} ({})", path.toStdString(),
                                 response.errorString.toStdString(), response.statusCode);
        co_return ApiResult<QJsonObject>(ApiError(response.statusCode, response.errorString));
    }

    // Parse JSON response
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(response.body, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        Logger::get("api")->warn("NeteaseClient: EAPI JSON parse error: {} (body: {})",
                                 parseError.errorString().toStdString(), response.body.left(200).toStdString());
        co_return ApiResult<QJsonObject>(
            ApiError(-1, QStringLiteral("Invalid JSON response"), parseError.errorString()));
    }

    QJsonObject json = doc.object();

    // Check for API-level errors
    int code = json[QLatin1String("code")].toInt();
    if (code != 200) {
        if (returnRawOnNon200) {
            co_return ApiResult<QJsonObject>(json);
        }
        // Auto-retry 301 (session expired) once, like Kotlin
        if (code == 301 && !retried && isAuthenticated()) {
            Logger::get("api")->info("NeteaseClient: 301 at {}, refreshing session", path.toStdString());
            co_await ensureWeapiSession();
            co_return co_await makeEapiRequest(path, params, host, returnRawOnNon200, true);
        }
        QString msg = json[QLatin1String("msg")].toString();
        if (msg.isEmpty()) {
            msg = json[QLatin1String("message")].toString();
        }
        Logger::get("api")->warn("NeteaseClient: EAPI error {} at {}: {}", code, path.toStdString(), msg.toStdString());
        co_return ApiResult<QJsonObject>(ApiError(code, msg));
    }

    co_return ApiResult<QJsonObject>(json);
}

void NeteaseClient::injectCookies(QNetworkRequest &request)
{
    QString cookie = m_cookie;
    // Add base cookies if not present
    if (!cookie.contains("os=")) {
        if (cookie.isEmpty()) {
            cookie = QStringLiteral("os=pc; appver=8.10.35");
        } else {
            cookie += QStringLiteral("; os=pc; appver=8.10.35");
        }
    }
    if (!cookie.isEmpty()) {
        request.setRawHeader("Cookie", cookie.toUtf8());
    }
}

void NeteaseClient::persistCookies(const QString &cookieString)
{
    m_cookie = cookieString;
    m_authenticated = true;

    // Extract CSRF token
    const QStringList parts = cookieString.split(QLatin1String("; "));
    for (const QString &part : parts) {
        if (part.startsWith(QLatin1String("__csrf="))) {
            m_csrfToken = part.mid(7);
            break;
        }
    }

    if (m_storage) {
        m_storage->set(COOKIE_STORAGE_KEY, cookieString);
    }
}

void NeteaseClient::clearCookies()
{
    m_cookie.clear();
    m_csrfToken.clear();
    m_authenticated = false;

    if (m_storage) {
        m_storage->remove(COOKIE_STORAGE_KEY);
    }
}

} // namespace NeriPlayerQt
