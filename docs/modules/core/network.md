# Network Module (network/)

## 1. Overview

The network module handles all network communication for the application, using C++20 coroutines and QCoro to implement asynchronous HTTP requests and WebSocket connections.

## 2. Directory Structure

```
src/core/network/
├── HttpClient.h/.cpp          # HTTP client (coroutine version)
├── WebSocketClient.h/.cpp     # WebSocket client
├── NetworkManager.h/.cpp      # Network manager
└── NetworkMonitor.h/.cpp      # Network status monitor
```

## 3. Main Class Design

### 3.1 NetworkManager

Network manager that centrally manages network resources.

```cpp
class NetworkManager : public QObject {
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);
    
    HttpClient* httpClient();
    WebSocketClient* webSocketClient();
    
    bool isOnline() const;
    NetworkType networkType() const;
    
    void setProxy(const QNetworkProxy &proxy);
    
signals:
    void networkStatusChanged(bool online);
    void networkTypeChanged(NetworkType type);
    
private:
    std::unique_ptr<HttpClient> m_httpClient;
    std::unique_ptr<WebSocketClient> m_wsClient;
    std::unique_ptr<NetworkMonitor> m_monitor;
};
```

### 3.2 HttpClient

HTTP client that implements asynchronous requests using coroutines.

```cpp
class HttpClient : public QObject {
    Q_OBJECT
public:
    explicit HttpClient(QObject *parent = nullptr);
    
    // ==================== Coroutine API ====================
    
    // GET request
    QCoro::Task<HttpResponse> get(const QUrl &url,
                                  const HttpHeaders &headers = {});
    
    // POST request
    QCoro::Task<HttpResponse> post(const QUrl &url,
                                   const QByteArray &data,
                                   const HttpHeaders &headers = {});
    
    // PUT request
    QCoro::Task<HttpResponse> put(const QUrl &url,
                                  const QByteArray &data,
                                  const HttpHeaders &headers = {});
    
    // DELETE request
    QCoro::Task<HttpResponse> deleteResource(const QUrl &url,
                                             const HttpHeaders &headers = {});
    
    // PATCH request
    QCoro::Task<HttpResponse> patch(const QUrl &url,
                                    const QByteArray &data,
                                    const HttpHeaders &headers = {});
    
    // File download
    QCoro::Task<bool> download(const QUrl &url,
                               const QString &savePath,
                               ProgressCallback progress = nullptr);
    
    // File upload
    QCoro::Task<HttpResponse> upload(const QUrl &url,
                                     const QString &filePath,
                                     const HttpHeaders &headers = {});
    
    // Request with timeout
    QCoro::Task<HttpResponse> getWithTimeout(const QUrl &url,
                                              std::chrono::milliseconds timeout,
                                              const HttpHeaders &headers = {});
    
    // ==================== Configuration ====================
    
    void setTimeout(int timeoutMs);
    int timeout() const;
    
    void setMaxRetries(int retries);
    void setRetryDelay(int delayMs);
    
signals:
    void requestStarted(const QUrl &url);
    void requestFinished(const QUrl &url, int statusCode);
    void requestError(const QUrl &url, const QString &error);
    void downloadProgress(const QUrl &url, qint64 bytesReceived,
                          qint64 bytesTotal);
    
private:
    // Internal methods
    QCoro::Task<HttpResponse> executeWithRetry(
        std::function<QCoro::Task<QNetworkReply*()> > requestFactory);
    
    QNetworkAccessManager *m_manager;
    int m_timeoutMs = 30000;
    int m_maxRetries = 3;
    int m_retryDelayMs = 1000;
};
```

### 3.3 HttpResponse

HTTP response structure.

```cpp
struct HttpResponse {
    int statusCode;
    QByteArray data;
    HttpHeaders headers;
    QString error;
    qint64 responseTime;
    
    bool isSuccess() const { return statusCode >= 200 && statusCode < 300; }
    QJsonObject toJson() const;
    QString toString() const;
    QJsonDocument toJsonDocument() const;
};

using HttpHeaders = QHash<QString, QString>;
using ProgressCallback = std::function<void(qint64 bytesReceived, 
                                            qint64 bytesTotal)>;
```

### 3.4 WebSocketClient

WebSocket client.

```cpp
class WebSocketClient : public QObject {
    Q_OBJECT
public:
    explicit WebSocketClient(QObject *parent = nullptr);
    ~WebSocketClient();
    
    // Connect (coroutine)
    QCoro::Task<bool> connectToServer(const QUrl &url,
                                       std::chrono::milliseconds timeout = std::chrono::seconds{10});
    void disconnectFromServer();
    
    // Send messages
    void sendTextMessage(const QString &message);
    void sendBinaryMessage(const QByteArray &data);
    void sendJsonMessage(const QJsonObject &json);
    
    // Wait for messages (coroutine)
    QCoro::Task<QString> waitForTextMessage(
        std::chrono::milliseconds timeout = std::chrono::seconds{30});
    QCoro::Task<QByteArray> waitForBinaryMessage(
        std::chrono::milliseconds timeout = std::chrono::seconds{30});
    QCoro::Task<QJsonObject> waitForJsonMessage(
        std::chrono::milliseconds timeout = std::chrono::seconds{30});
    
    // Status
    bool isConnected() const;
    QUrl serverUrl() const;
    
signals:
    void connected();
    void disconnected();
    void textMessageReceived(const QString &message);
    void binaryMessageReceived(const QByteArray &data);
    void jsonMessageReceived(const QJsonObject &json);
    void errorOccurred(const QString &error);
    
private:
    QWebSocket *m_webSocket;
    QUrl m_serverUrl;
};
```

## 4. Implementation Examples

### 4.1 HttpClient Implementation

```cpp
#include "HttpClient.h"
#include <QCoroNetworkReply>
#include <QCoroTimer>

HttpClient::HttpClient(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
}

QCoro::Task<HttpResponse> HttpClient::get(const QUrl &url,
                                           const HttpHeaders &headers)
{
    emit requestStarted(url);
    
    QNetworkRequest request(url);
    for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }
    
    auto *reply = co_await m_manager->get(request);
    
    HttpResponse response;
    response.statusCode = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
    response.data = reply->readAll();
    response.error = reply->errorString();
    
    // Collect response headers
    for (const auto &header : reply->rawHeaderPairs()) {
        response.headers[QString::fromUtf8(header.first)] = 
            QString::fromUtf8(header.second);
    }
    
    reply->deleteLater();
    
    emit requestFinished(url, response.statusCode);
    
    co_return response;
}

QCoro::Task<HttpResponse> HttpClient::post(const QUrl &url,
                                            const QByteArray &data,
                                            const HttpHeaders &headers)
{
    emit requestStarted(url);
    
    QNetworkRequest request(url);
    for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }
    
    auto *reply = co_await m_manager->post(request, data);
    
    HttpResponse response;
    response.statusCode = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
    response.data = reply->readAll();
    response.error = reply->errorString();
    
    reply->deleteLater();
    
    emit requestFinished(url, response.statusCode);
    
    co_return response;
}

QCoro::Task<HttpResponse> HttpClient::getWithTimeout(
    const QUrl &url,
    std::chrono::milliseconds timeout,
    const HttpHeaders &headers)
{
    QNetworkRequest request(url);
    for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }
    
    auto *reply = m_manager->get(request);
    
    // Use QCoro's waitForFinished for timeout
    bool finished = co_await qCoro(reply)->waitForFinished(timeout);
    
    HttpResponse response;
    if (!finished) {
        reply->abort();
        response.statusCode = -1;
        response.error = "Request timed out";
    } else {
        response.statusCode = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt();
        response.data = reply->readAll();
        response.error = reply->errorString();
    }
    
    reply->deleteLater();
    co_return response;
}

QCoro::Task<bool> HttpClient::download(const QUrl &url,
                                        const QString &savePath,
                                        ProgressCallback progress)
{
    QNetworkRequest request(url);
    auto *reply = m_manager->get(request);
    
    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        reply->abort();
        reply->deleteLater();
        co_return false;
    }
    
    // Connect progress signal
    if (progress) {
        connect(reply, &QNetworkReply::downloadProgress,
                this, [this, url, progress](qint64 received, qint64 total) {
            progress(received, total);
            emit downloadProgress(url, received, total);
        });
    }
    
    // Wait for completion
    co_await qCoro(reply)->waitForFinished();
    
    if (reply->error() != QNetworkReply::NoError) {
        file.close();
        QFile::remove(savePath);
        reply->deleteLater();
        co_return false;
    }
    
    file.write(reply->readAll());
    file.close();
    reply->deleteLater();
    
    co_return true;
}
```

### 4.2 Usage Example

```cpp
// Usage in ViewModel
class SearchViewModel : public QObject {
    Q_OBJECT
public:
    QCoro::Task<void> search(const QString &query) {
        emit searchStarted();
        
        try {
            auto *network = ServiceLocator::instance()->service<NetworkManager>();
            auto *httpClient = network->httpClient();
            
            QUrl url("https://api.example.com/search");
            QUrlQuery params;
            params.addQueryItem("q", query);
            url.setQuery(params);
            
            auto response = co_await httpClient->get(url);
            
            if (response.isSuccess()) {
                auto results = parseResults(response.toJson());
                emit searchFinished(results);
            } else {
                emit searchError(response.error);
            }
        } catch (const std::exception &e) {
            emit searchError(QString::fromStdString(e.what()));
        }
    }
};
```

### 4.3 Parallel Requests

```cpp
QCoro::Task<void> fetchMultipleSources(const QString &query) {
    auto *httpClient = ServiceLocator::instance()->service<NetworkManager>()->httpClient();
    
    // Launch multiple requests in parallel
    auto neteaseTask = httpClient->get(QUrl("https://netease.api/search?q=" + query));
    auto bilibiliTask = httpClient->get(QUrl("https://bilibili.api/search?q=" + query));
    auto youtubeTask = httpClient->get(QUrl("https://youtube.api/search?q=" + query));
    
    // Wait for all to complete
    auto neteaseResult = co_await neteaseTask;
    auto bilibiliResult = co_await bilibiliTask;
    auto youtubeResult = co_await youtubeTask;
    
    // Merge results
    SearchResult result;
    result.songs.append(parseNeteaseResults(neteaseResult));
    result.songs.append(parseBilibiliResults(bilibiliResult));
    result.songs.append(parseYoutubeResults(youtubeResult));
    
    co_return result;
}
```

## 5. Error Handling

```cpp
QCoro::Task<void> safeRequest() {
    try {
        auto response = co_await httpClient->get(url);
        
        if (!response.isSuccess()) {
            // HTTP error
            LOG_ERROR("HTTP error:" << response.statusCode << response.error);
            co_return;
        }
        
        // Process response
    } catch (const QNetworkReply::NetworkError &e) {
        // Network error
        LOG_ERROR("Network error:" << e);
    } catch (const std::exception &e) {
        // Other error
        LOG_ERROR("Error:" << e.what());
    }
}
```

## 6. Testing

```cpp
class HttpClientTest : public QObject {
    Q_OBJECT
private slots:
    void testGet() {
        QCoro::waitFor([]() -> QCoro::Task<void> {
            HttpClient client;
            auto response = co_await client.get(QUrl("https://httpbin.org/get"));
            QVERIFY(response.isSuccess());
        }());
    }
    
    void testTimeout() {
        QCoro::waitFor([]() -> QCoro::Task<void> {
            HttpClient client;
            auto response = co_await client.getWithTimeout(
                QUrl("https://httpbin.org/delay/10"),
                std::chrono::seconds{1});
            QCOMPARE(response.statusCode, -1);
        }());
    }
};
```

## 7. Summary

The network module implemented with C++20 coroutines and QCoro:
- **Clean Code**: Async code written in synchronous style
- **Error Handling**: Exception handling with try-catch
- **Timeout Support**: Built-in timeout mechanism
- **Parallel Requests**: Support for launching multiple requests in parallel
- **Type Safety**: Compile-time type checking
