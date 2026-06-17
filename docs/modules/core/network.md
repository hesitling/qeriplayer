# Network Module (core/network/)

## Overview

The network module provides HTTP client, WebSocket client, and network status monitoring. The `HttpClient` uses QCoro for coroutine-based async requests.

## Source Files

```
src/core/network/
├── HttpClient.h / .cpp        # Coroutine-based HTTP client
├── WebSocketClient.h / .cpp   # WebSocket client
├── NetworkManager.h / .cpp    # Owns HttpClient, WebSocketClient, NetworkMonitor
└── NetworkMonitor.h / .cpp    # Online/offline status
```

## NetworkManager

Owns and provides access to the HTTP client, WebSocket client, and network monitor.

```cpp
class NetworkManager : public QObject {
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    HttpClient *httpClient();
    WebSocketClient *webSocketClient();
    NetworkMonitor *networkMonitor();
};
```

## HttpClient

Coroutine-based HTTP client wrapping `QNetworkAccessManager`.

```cpp
class HttpClient : public QObject {
    Q_OBJECT
public:
    explicit HttpClient(QObject *parent = nullptr);

    QCoro::Task<HttpResponse> get(const QUrl &url);
    QCoro::Task<HttpResponse> get(const QNetworkRequest &request);
    QCoro::Task<HttpResponse> post(const QUrl &url, const QByteArray &body);
    QCoro::Task<HttpResponse> post(const QNetworkRequest &request, const QByteArray &body);
};
```

### HttpResponse

```cpp
struct HttpResponse {
    int statusCode = 0;
    QByteArray body;
    QString errorString;
    QList<QNetworkReply::RawHeaderPair> headers;

    bool isSuccess() const; // statusCode 200-299
};
```

### Usage

```cpp
auto *http = ServiceLocator::service<NetworkManager>()->httpClient();

auto response = co_await http->get(QUrl("https://api.example.com/data"));
if (response.isSuccess()) {
    auto json = QJsonDocument::fromJson(response.body);
    // process json...
}
```

## WebSocketClient

Simple WebSocket client wrapping `QWebSocket`.

```cpp
class WebSocketClient : public QObject {
    Q_OBJECT
public:
    explicit WebSocketClient(QObject *parent = nullptr);

    void connectTo(const QUrl &url);
    void sendTextMessage(const QString &message);
    void close();
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void textMessageReceived(const QString &message);
    void errorOccurred(const QString &message);
};
```

## NetworkMonitor

Tracks online/offline status.

```cpp
class NetworkMonitor : public QObject {
    Q_OBJECT
public:
    explicit NetworkMonitor(QObject *parent = nullptr);
    bool isOnline() const;
    void setOnline(bool online);

signals:
    void onlineChanged(bool online);
};
```

## Design Decisions

- **QCoro integration** — `HttpClient` returns `QCoro::Task<HttpResponse>` for `co_await`-based usage.
- **No retry logic built in** — retry policies are the responsibility of callers (e.g., `NeteaseClient`).
- **No timeout built in** — callers can implement timeouts externally via `QCoro::Task` cancellation.
