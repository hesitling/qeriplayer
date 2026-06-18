#ifndef QERIPLAYERQT_HTTPCLIENT_H
#define QERIPLAYERQT_HTTPCLIENT_H

#include <QCoroTask>

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QUrl>

namespace QeriPlayerQt {

struct HttpResponse {
    int statusCode = 0;
    QByteArray body;
    QString errorString;
    QList<QNetworkReply::RawHeaderPair> headers;

    bool isSuccess() const;
};

class HttpClient : public QObject {
    Q_OBJECT

public:
    explicit HttpClient(QObject *parent = nullptr);

    QCoro::Task<HttpResponse> get(const QUrl &url);
    QCoro::Task<HttpResponse> get(const QNetworkRequest &request);
    QCoro::Task<HttpResponse> post(const QUrl &url, const QByteArray &body);
    QCoro::Task<HttpResponse> post(const QNetworkRequest &request, const QByteArray &body);

private:
    QCoro::Task<HttpResponse> send(QNetworkReply *reply);

    QNetworkAccessManager m_networkAccess;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_HTTPCLIENT_H
