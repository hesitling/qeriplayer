#ifndef NERIPLAYERQT_HTTPCLIENT_H
#define NERIPLAYERQT_HTTPCLIENT_H

#include <QCoroTask>

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QUrl>

namespace NeriPlayerQt {

struct HttpResponse {
    int statusCode = 0;
    QByteArray body;
    QString errorString;

    bool isSuccess() const;
};

class HttpClient : public QObject {
    Q_OBJECT

public:
    explicit HttpClient(QObject *parent = nullptr);

    QCoro::Task<HttpResponse> get(const QUrl &url);
    QCoro::Task<HttpResponse> post(const QUrl &url, const QByteArray &body);

private:
    QCoro::Task<HttpResponse> send(QNetworkReply *reply);

    QNetworkAccessManager m_networkAccess;
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_HTTPCLIENT_H
