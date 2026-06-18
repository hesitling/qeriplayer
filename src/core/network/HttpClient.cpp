#include "core/network/HttpClient.h"

#include <QCoroNetworkReply>

#include <QNetworkReply>

namespace QeriPlayerQt {

bool HttpResponse::isSuccess() const
{
    return statusCode >= 200 && statusCode < 300 && errorString.isEmpty();
}

HttpClient::HttpClient(QObject *parent)
    : QObject(parent)
{
    // Follow all redirects including HTTPS upgrades
    m_networkAccess.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

QCoro::Task<HttpResponse> HttpClient::get(const QUrl &url)
{
    co_return co_await send(m_networkAccess.get(QNetworkRequest(url)));
}

QCoro::Task<HttpResponse> HttpClient::get(const QNetworkRequest &request)
{
    co_return co_await send(m_networkAccess.get(request));
}

QCoro::Task<HttpResponse> HttpClient::post(const QUrl &url, const QByteArray &body)
{
    co_return co_await send(m_networkAccess.post(QNetworkRequest(url), body));
}

QCoro::Task<HttpResponse> HttpClient::post(const QNetworkRequest &request, const QByteArray &body)
{
    co_return co_await send(m_networkAccess.post(request, body));
}

QCoro::Task<HttpResponse> HttpClient::send(QNetworkReply *reply)
{
    co_await reply;

    HttpResponse response;
    response.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    response.body = reply->readAll();
    response.headers = reply->rawHeaderPairs();
    if (reply->error() != QNetworkReply::NoError) {
        response.errorString = reply->errorString();
    }

    reply->deleteLater();
    co_return response;
}

} // namespace QeriPlayerQt
