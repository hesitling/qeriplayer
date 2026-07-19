/// @file NeteaseClientCore.cpp
/// @brief NeteaseClient — constructor, configuration, request helpers

#include "api/netease/NeteaseClient.h"

#include "api/common/ApiError.h"
#include "api/netease/NeteaseCrypto.h"
#include "core/crypto/SecureStorage.h"
#include "core/logger/Logger.h"
#include "core/network/HttpClient.h"

#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkRequest>
#include <QStringList>
#include <QUrlQuery>

namespace QeriPlayerQt {

static const QUrl DEFAULT_BASE_URL(QStringLiteral("https://music.163.com"));
static const QString COOKIE_STORAGE_KEY = QStringLiteral("netease_cookie");

static bool isSensitiveJsonKey(const QString &key)
{
    const QString lower = key.toLower();
    return lower.contains(QStringLiteral("password")) || lower.contains(QStringLiteral("cookie"))
           || lower.contains(QStringLiteral("token")) || lower.contains(QStringLiteral("csrf"))
           || lower.contains(QStringLiteral("captcha"));
}

static QJsonValue redactJsonValue(const QString &key, const QJsonValue &value)
{
    if (isSensitiveJsonKey(key)) {
        return QStringLiteral("<redacted>");
    }

    if (value.isObject()) {
        QJsonObject object;
        const QJsonObject input = value.toObject();
        for (auto it = input.constBegin(); it != input.constEnd(); ++it) {
            object.insert(it.key(), redactJsonValue(it.key(), it.value()));
        }
        return object;
    }

    if (value.isArray()) {
        QJsonArray array;
        const QJsonArray input = value.toArray();
        for (const QJsonValue &item : input) {
            array.append(redactJsonValue(QString(), item));
        }
        return array;
    }

    return value;
}

static QString compactJsonForLog(const QJsonObject &object)
{
    QJsonObject redacted;
    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        redacted.insert(it.key(), redactJsonValue(it.key(), it.value()));
    }
    return QString::fromUtf8(QJsonDocument(redacted).toJson(QJsonDocument::Compact));
}

static QHash<QString, QString> parseCookieString(const QString &cookieString)
{
    QHash<QString, QString> cookieMap;
    const QStringList cookies = cookieString.split(QLatin1Char(';'), Qt::SkipEmptyParts);
    for (const QString &cookie : cookies) {
        const QString trimmed = cookie.trimmed();
        const qsizetype equals = trimmed.indexOf(QLatin1Char('='));
        if (equals <= 0) {
            continue;
        }

        const QString name = trimmed.left(equals).trimmed();
        const QString value = trimmed.mid(equals + 1).trimmed();
        if (!name.isEmpty()) {
            cookieMap.insert(name, value);
        }
    }
    return cookieMap;
}

static QString formatCookieString(const QHash<QString, QString> &cookieMap)
{
    QStringList parts;
    for (auto it = cookieMap.constBegin(); it != cookieMap.constEnd(); ++it) {
        if (!it.key().isEmpty()) {
            QString value = it.value();
            value.replace(QLatin1Char('\\'), QStringLiteral("%5C"));
            parts.append(it.key() + QLatin1Char('=') + value);
        }
    }
    return parts.join(QStringLiteral("; "));
}

static QString redactCookieHeaderForLog(const QByteArray &cookieHeader)
{
    const QHash<QString, QString> cookieMap = parseCookieString(QString::fromUtf8(cookieHeader));
    if (cookieMap.isEmpty()) {
        return QStringLiteral("<none>");
    }

    QStringList parts;
    for (auto it = cookieMap.constBegin(); it != cookieMap.constEnd(); ++it) {
        parts.append(it.key() + QStringLiteral("=<redacted>"));
    }
    return parts.join(QStringLiteral("; "));
}

static QString responseBodyForLog(const QByteArray &body)
{
    constexpr qsizetype MAX_LOG_BODY_BYTES = 500;

    QByteArray toLog = body;
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error == QJsonParseError::NoError) {
        if (doc.isObject()) {
            toLog = compactJsonForLog(doc.object()).toUtf8();
        } else if (doc.isArray()) {
            QJsonArray redactedArray;
            const QJsonArray input = doc.array();
            for (const QJsonValue &item : input) {
                redactedArray.append(redactJsonValue(QString(), item));
            }
            toLog = QJsonDocument(redactedArray).toJson(QJsonDocument::Compact);
        }
    }

    QString text = QString::fromUtf8(toLog.left(MAX_LOG_BODY_BYTES));
    if (toLog.size() > MAX_LOG_BODY_BYTES) {
        text += QStringLiteral("...");
    }
    return text;
}

static void useManualCookieHandling(QNetworkRequest &request)
{
    request.setAttribute(QNetworkRequest::CookieLoadControlAttribute, QNetworkRequest::Manual);
    request.setAttribute(QNetworkRequest::CookieSaveControlAttribute, QNetworkRequest::Manual);
}

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
            const QHash<QString, QString> cookieMap = parseCookieString(m_cookie);
            m_csrfToken = cookieMap.value(QStringLiteral("__csrf"));
            m_authenticated = !m_csrfToken.isEmpty();
        }
    }
    Logger::get("api")->info("NeteaseClient: restored login status authenticated={} cookie={}", m_authenticated,
                             m_cookie.toStdString());
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
    useManualCookieHandling(request);
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

    auto log = Logger::get("api");
    log->debug("NeteaseClient: WEAPI request path={} url={} params={} cookie={} bodyBytes={}", path.toStdString(),
               url.toString(QUrl::RemovePassword).toStdString(), compactJsonForLog(weapiParams).toStdString(),
               redactCookieHeaderForLog(request.rawHeader("Cookie")).toStdString(), postData.size());

    // Send request with all headers preserved
    auto response = co_await m_httpClient->post(request, postData);

    log->debug("NeteaseClient: WEAPI response path={} status={} success={} bytes={} body={}", path.toStdString(),
               response.statusCode, response.isSuccess(), response.body.size(),
               responseBodyForLog(response.body).toStdString());

    // Extract cookies from response headers and merge
    extractResponseCookies(response);

    if (!response.isSuccess()) {
        log->warn("NeteaseClient: HTTP error at path {}: {} ({})", path.toStdString(),
                  response.errorString.toStdString(), response.statusCode);
        co_return ApiResult<QJsonObject>(ApiError(response.statusCode, response.errorString));
    }

    // Parse JSON response
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(response.body, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        log->warn("NeteaseClient: JSON parse error at path {}: {} (body: {})", path.toStdString(),
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
            log->info("NeteaseClient: 301 at {}, refreshing session", path.toStdString());
            co_await ensureWeapiSession();
            co_return co_await makeRequest(path, params, host, true);
        }
        QString msg = json[QLatin1String("msg")].toString();
        if (msg.isEmpty()) {
            msg = json[QLatin1String("message")].toString();
        }
        log->warn("NeteaseClient: API error {} at {}: {}", code, path.toStdString(), msg.toStdString());
        co_return ApiResult<QJsonObject>(ApiError(code, msg));
    }

    co_return ApiResult<QJsonObject>(json);
}

QCoro::Task<ApiResult<QJsonObject>> NeteaseClient::makeUnencryptedRequest(const QString &path,
                                                                          const QJsonObject &params, bool useGet)
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
    if (useGet && !query.isEmpty()) {
        QUrlQuery urlQuery(url.query());
        for (const auto &item : query.queryItems()) {
            urlQuery.addQueryItem(item.first, item.second);
        }
        url.setQuery(urlQuery);
    }

    QNetworkRequest request(url);
    useManualCookieHandling(request);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    request.setRawHeader("Referer", "https://music.163.com");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like "
                                       "Gecko) Chrome/120.0.0.0 Safari/537.36");
    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Accept-Language", "zh-CN,zh-Hans;q=0.9");
    injectCookies(request);

    QByteArray postData = useGet ? QByteArray() : query.toString(QUrl::FullyEncoded).toUtf8();
    auto log = Logger::get("api");
    log->debug("NeteaseClient: raw {} request path={} url={} params={} cookie={} bodyBytes={}", useGet ? "GET" : "POST",
               path.toStdString(), url.toString(QUrl::RemovePassword).toStdString(),
               compactJsonForLog(params).toStdString(),
               redactCookieHeaderForLog(request.rawHeader("Cookie")).toStdString(), postData.size());

    auto response = useGet ? co_await m_httpClient->get(request) : co_await m_httpClient->post(request, postData);

    log->debug("NeteaseClient: raw response path={} status={} success={} bytes={} body={}", path.toStdString(),
               response.statusCode, response.isSuccess(), response.body.size(),
               responseBodyForLog(response.body).toStdString());

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
    useManualCookieHandling(request);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    request.setRawHeader("Referer", "https://music.163.com");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like "
                                       "Gecko) Chrome/120.0.0.0 Safari/537.36");
    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Accept-Language", "zh-CN,zh-Hans;q=0.9");

    // Inject cookies
    injectCookies(request);

    auto log = Logger::get("api");
    log->debug("NeteaseClient: EAPI request path={} url={} params={} cookie={} bodyBytes={}", path.toStdString(),
               url.toString(QUrl::RemovePassword).toStdString(), compactJsonForLog(params).toStdString(),
               redactCookieHeaderForLog(request.rawHeader("Cookie")).toStdString(), postData.size());

    // Send request with all headers preserved
    auto response = co_await m_httpClient->post(request, postData);

    log->debug("NeteaseClient: EAPI response path={} status={} success={} bytes={} body={}", path.toStdString(),
               response.statusCode, response.isSuccess(), response.body.size(),
               responseBodyForLog(response.body).toStdString());

    if (!response.isSuccess()) {
        log->warn("NeteaseClient: EAPI HTTP error at {}: {} ({})", path.toStdString(),
                  response.errorString.toStdString(), response.statusCode);
        co_return ApiResult<QJsonObject>(ApiError(response.statusCode, response.errorString));
    }

    // Parse JSON response
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(response.body, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        log->warn("NeteaseClient: EAPI JSON parse error: {} (body: {})", parseError.errorString().toStdString(),
                  response.body.left(200).toStdString());
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
            log->info("NeteaseClient: 301 at {}, refreshing session", path.toStdString());
            co_await ensureWeapiSession();
            co_return co_await makeEapiRequest(path, params, host, returnRawOnNon200, true);
        }
        QString msg = json[QLatin1String("msg")].toString();
        if (msg.isEmpty()) {
            msg = json[QLatin1String("message")].toString();
        }
        log->warn("NeteaseClient: EAPI error {} at {}: {}", code, path.toStdString(), msg.toStdString());
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

void NeteaseClient::persistCookies(const QString &cookieString, bool authenticated, bool persistToStorage)
{
    m_cookie = cookieString;
    m_authenticated = authenticated;
    m_csrfToken.clear();

    // Extract CSRF token
    const QHash<QString, QString> cookieMap = parseCookieString(cookieString);
    m_csrfToken = cookieMap.value(QStringLiteral("__csrf"));

    Logger::get("api")->info("NeteaseClient: login status authenticated={} cookie={}", m_authenticated,
                             m_cookie.toStdString());

    if (m_storage && persistToStorage) {
        m_storage->set(COOKIE_STORAGE_KEY, cookieString);
    }
}

void NeteaseClient::extractResponseCookies(const HttpResponse &response)
{
    QStringList newCookies;
    for (const auto &header : response.headers) {
        if (QString::fromUtf8(header.first).compare(QStringLiteral("Set-Cookie"), Qt::CaseInsensitive) == 0) {
            const QString setCookie = QString::fromUtf8(header.second);
            const QString nameValue = setCookie.split(QLatin1Char(';')).first().trimmed();
            const QString value = nameValue.section(QLatin1Char('='), 1);
            const QString lowerSetCookie = setCookie.toLower();
            if (value.isEmpty() || value == QStringLiteral("deleted")
                || lowerSetCookie.contains(QStringLiteral("max-age=0"))) {
                continue;
            }
            newCookies.append(nameValue);
        }
    }
    if (!newCookies.isEmpty()) {
        QHash<QString, QString> cookieMap = parseCookieString(m_cookie);
        for (const QString &cookie : newCookies) {
            const QHash<QString, QString> responseCookie = parseCookieString(cookie);
            for (auto it = responseCookie.constBegin(); it != responseCookie.constEnd(); ++it) {
                cookieMap.insert(it.key(), it.value());
            }
        }
        persistCookies(formatCookieString(cookieMap), m_authenticated, m_authenticated);
    }
}

void NeteaseClient::clearCookies()
{
    Logger::get("api")->info("NeteaseClient: login status authenticated=false cookie={}", m_cookie.toStdString());

    m_cookie.clear();
    m_csrfToken.clear();
    m_authenticated = false;

    if (m_storage) {
        m_storage->remove(COOKIE_STORAGE_KEY);
    }
}

} // namespace QeriPlayerQt
