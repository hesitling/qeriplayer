# C++20 Coroutines & QCoro

## 1. Overview

QeriPlayer Qt uses C++20 coroutines and the [QCoro](https://github.com/qcoro/qcoro) library to simplify asynchronous programming, particularly in scenarios involving network requests, file I/O, and database operations.

## 2. Why Use Coroutines

### 2.1 Problems with Traditional Async Programming

Traditional Qt asynchronous programming uses the signal-slot mechanism, which has the following issues:

```cpp
// Traditional approach: callback hell
void MainWindow::fetchData() {
    auto *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            auto data = reply->readAll();
            auto *reply2 = m_networkManager->get(request2);
            connect(reply2, &QNetworkReply::finished, this, [this, reply2]() {
                // Nesting gets deeper and deeper...
            });
        }
    });
}
```

### 2.2 Advantages of Coroutines

Using coroutines allows writing async code in synchronous style:

```cpp
// Coroutine approach: clear linear code
QCoro::Task<void> MainWindow::fetchData() {
    auto *reply = co_await m_networkManager->get(request);
    if (reply->error() != QNetworkReply::NoError) {
        co_return;
    }
    
    auto data = reply->readAll();
    auto *reply2 = co_await m_networkManager->get(request2);
    // Continue processing...
}
```

## 3. QCoro Introduction

QCoro is a library that integrates Qt's asynchronous APIs with C++20 coroutines, providing:

- `QCoro::Task<T>`: Coroutine return type
- Coroutine support for Qt types: `QNetworkReply`, `QTimer`, `QFuture`, etc.
- `.then()` continuation support
- Async generators

### 3.1 Supported Qt Types

| Type | Description | Usage |
|------|-------------|-------|
| `QNetworkReply` | Network request | `co_await reply` |
| `QTimer` | Timer | `co_await timer` |
| `QFuture<T>` | Async task | `co_await future` |
| `QIODevice` | I/O device | `co_await device` |
| `QWebSocket` | WebSocket | `co_await socket` |
| `QDBusPendingCall` | D-Bus call | `co_await call` |

## 4. Integration Setup

### 4.1 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(QeriPlayerQt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find QCoro
find_package(QCoro6 REQUIRED COMPONENTS Core Network WebSockets)

# Enable coroutine support
qcoro_enable_coroutines()

# Find Qt
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network WebSockets)

add_executable(QeriPlayerQt
    src/main.cpp
    src/mainwindow.cpp
    # ...
)

target_link_libraries(QeriPlayerQt PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
    QCoro::Core
    QCoro::Network
    QCoro::WebSockets
)
```

### 4.2 Compiler Requirements

| Compiler | Minimum Version |
|----------|-----------------|
| GCC | 10+ |
| Clang | 14+ |
| MSVC | 19.29+ |

## 5. Basic Usage

### 5.1 Task<T>

The coroutine return type is `QCoro::Task<T>`:

```cpp
// Returns Task<QString>
QCoro::Task<QString> fetchUserName(int userId) {
    auto *reply = co_await m_networkManager->get(QNetworkRequest{url});
    QString name = parseUserName(reply->readAll());
    co_return name;
}

// Returns Task<void>
QCoro::Task<void> updateUI() {
    QString name = co_await fetchUserName(123);
    m_nameLabel->setText(name);
}
```

### 5.2 Network Requests

```cpp
QCoro::Task<QJsonObject> HttpClient::get(const QUrl &url) {
    QNetworkRequest request(url);
    auto *reply = co_await m_manager->get(request);
    
    if (reply->error() != QNetworkReply::NoError) {
        throw NetworkError(reply->errorString());
    }
    
    auto data = reply->readAll();
    reply->deleteLater();
    
    co_return QJsonDocument::fromJson(data).object();
}
```

### 5.3 Timers

```cpp
QCoro::Task<void> delayedTask() {
    co_await QCoro::sleepFor(std::chrono::seconds(5));
    qDebug() << "5 seconds later...";
}
```

### 5.4 Error Handling

```cpp
QCoro::Task<void> riskyOperation() {
    try {
        auto *reply = co_await m_manager->get(request);
        // Process response
    } catch (const NetworkError &e) {
        LOG_ERROR("Network", e.what());
    } catch (const std::exception &e) {
        LOG_ERROR("Unknown", e.what());
    }
}
```

## 6. Advanced Usage

### 6.1 Parallel Execution

```cpp
QCoro::Task<void> fetchMultiple() {
    // Launch multiple requests in parallel
    auto task1 = fetchUserData(1);
    auto task2 = fetchUserData(2);
    auto task3 = fetchUserData(3);
    
    // Wait for all to complete
    auto data1 = co_await task1;
    auto data2 = co_await task2;
    auto data3 = co_await task3;
}
```

### 6.2 Timeout Handling

```cpp
QCoro::Task<QByteArray> fetchWithTimeout(const QUrl &url, 
                                          std::chrono::milliseconds timeout) {
    QNetworkRequest request(url);
    auto *reply = m_manager->get(request);
    
    // Use waitForFinished for timeout
    bool finished = co_await qCoro(reply)->waitForFinished(timeout);
    
    if (!finished) {
        reply->abort();
        throw TimeoutError("Request timed out");
    }
    
    co_return reply->readAll();
}
```

### 6.3 .then() Continuations

When co_await cannot be used, .then() can be used instead:

```cpp
void MainWindow::onButtonClicked() {
    fetchData().then([this](const QString &data) {
        m_label->setText(data);
    }, [](const std::exception &e) {
        qWarning() << "Error:" << e.what();
    });
}
```

### 6.4 QCoro::connect

Safe connection similar to QObject::connect:

```cpp
void Model::update() {
    auto task = fetchEntries();
    QCoro::connect(std::move(task), this, [this](auto &&entries) {
        beginResetModel();
        m_entries = std::move(entries);
        endResetModel();
    });
}
```

## 7. Application in the Project

### 7.1 Network Layer

```cpp
// HttpClient uses coroutines
class HttpClient : public QObject {
    Q_OBJECT
public:
    // Returns Task instead of QFuture
    QCoro::Task<HttpResponse> get(const QUrl &url,
                                  const HttpHeaders &headers = {});
    
    QCoro::Task<HttpResponse> post(const QUrl &url,
                                   const QByteArray &data,
                                   const HttpHeaders &headers = {});
    
    QCoro::Task<bool> download(const QUrl &url,
                               const QString &savePath,
                               ProgressCallback progress = nullptr);
};
```

### 7.2 API Clients

```cpp
class NeteaseClient : public QObject {
    Q_OBJECT
public:
    QCoro::Task<ApiResult<SearchResult>> searchSongs(const QString &keyword,
                                                      int limit = 30);
    
    QCoro::Task<ApiResult<SongDetail>> getSongDetail(const QString &songId);
    
    QCoro::Task<ApiResult<Lyrics>> getLyrics(const QString &songId);
};
```

### 7.3 Business Layer

```cpp
// PlaybackController lives in player/ and acts as a de facto service for playback orchestration
class PlaybackController : public QObject {
    Q_OBJECT
public:
    QCoro::Task<void> play(const Song &song);
    
private:
    QCoro::Task<QString> resolveUrl(const Song &song);
};

// ViewModels access repos and API clients directly (no dedicated service layer)
class SearchViewModel : public QObject {
    Q_OBJECT
public:
    QCoro::Task<void> search(const QString &query);
    
private:
    IMusicPlatformPlugin *currentPlugin() const;
    QVector<IMusicPlatformPlugin *> m_plugins;
};
```

## 8. Best Practices

### 8.1 Lifetime Management

```cpp
// Use QPointer to prevent dangling pointers
QCoro::Task<void> MainWindow::fetchData() {
    QPointer<MainWindow> self = this;
    
    auto *reply = co_await m_manager->get(request);
    
    // Check if object is still valid
    if (!self) {
        co_return;
    }
    
    // Continue processing...
}
```

### 8.2 Cancellation Support

```cpp
class CancellableTask {
public:
    void cancel() { m_cancelled = true; }
    
    QCoro::Task<void> run() {
        for (int i = 0; i < 100; ++i) {
            if (m_cancelled) {
                co_return;
            }
            co_await processItem(i);
        }
    }
    
private:
    std::atomic<bool> m_cancelled{false};
};
```

### 8.3 Exception Safety

```cpp
QCoro::Task<Result> safeOperation() {
    try {
        auto data = co_await fetchData();
        co_return process(data);
    } catch (const std::exception &e) {
        LOG_ERROR("Operation failed:" << e.what());
        co_return Result::error(e.what());
    }
}
```

## 9. Summary

Using C++20 coroutines and QCoro can:
- Simplify async code and avoid callback hell
- Improve code readability and maintainability
- Keep Qt's event loop running properly
- Support timeout, cancellation, and error handling
