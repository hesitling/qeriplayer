# AGENTS.md

Guidelines for AI agents working on NeriPlayer Qt.

## Tech Stack

| Category | Technology | Version |
|----------|-----------|---------|
| Language | C++ | 20 |
| Coroutine | QCoro | 0.10+ |
| UI Framework | Qt | 6.5+ |
| Build System | CMake | 3.16+ |
| Database | SQLite | 3.35+ |
| JSON | nlohmann/json | 3.11+ |
| Logging | spdlog | 1.12+ |

## Project Structure

```
src/
├── app/                    # Application entry
├── core/                   # Infrastructure (network, database, crypto, logger)
├── api/                    # Platform APIs (netease, bilibili, youtube)
├── player/                 # Audio playback
├── playlist/               # Playlist management
├── download/               # Download manager
├── search/                 # Search aggregation
├── sync/                   # Cloud sync (GitHub/WebDAV)
├── settings/               # Settings management
├── ui/                     # UI components (widgets, dialogs, views)
└── viewmodel/              # ViewModels (MVVM)
```

## Code Conventions

### Naming

| Type | Convention | Example |
|------|-----------|---------|
| Files | `PascalCase.h/cpp` | `HttpClient.h` |
| Classes | `PascalCase` | `PlayerManager` |
| Interfaces | `IPascalCase` | `IMusicPlatformPlugin` |
| Member vars | `m_camelCase` | `m_networkManager` |
| Local vars | `camelCase` | `statusCode` |
| Constants | `UPPER_SNAKE_CASE` | `MAX_RETRY_COUNT` |
| Functions | `camelCase` | `getSongDetail()` |
| Signals | `camelCase` | `stateChanged()` |

### Coroutine Style

```cpp
QCoro::Task<SearchResult> search(const QString &query) {
    auto *reply = co_await m_manager->get(request);
    if (reply->error() != QNetworkReply::NoError) {
        throw NetworkError(reply->errorString());
    }
    co_return parseResult(reply->readAll());
}
```

### Include Order

1. Corresponding header
2. QCoro headers
3. Qt headers
4. Standard library
5. Third-party libraries

## Length Guidelines

| Type | Max | Recommended |
|------|-----|-------------|
| Header file | 200 lines | 100-150 |
| Source file | 500 lines | 200-400 |
| Function | 50 lines | 20-30 |
| Coroutine | 80 lines | 30-50 |
| Line width | 120 chars | 80-100 |

## Commit Conventions

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <subject>
```

### Types

| Type | Usage |
|------|-------|
| `feat` | New feature |
| `fix` | Bug fix |
| `docs` | Documentation |
| `refactor` | Code refactoring |
| `test` | Add/update tests |
| `chore` | Other changes |

### Scopes

`app`, `core`, `api`, `player`, `ui`, `docs`, `build`, `test`

### Examples

```
feat(api): add YouTube Music search support
fix(network): handle connection timeout
refactor(core): use QCoro for async operations
docs: update architecture documentation
```

### Rules

- Use imperative mood ("add" not "added")
- No capitalization on first letter
- No period at end
- Max 50 characters for subject
- Reference issues: `Fixes #123`

## Doxygen Documentation

Use `///` for brief comments, `/** */` for detailed documentation.

### File Header

```cpp
/// @file HttpClient.h
/// @brief HTTP client with coroutine support
/// @author NeriPlayer Team
/// @date 2024-01-15
```

### Class Documentation

```cpp
/**
 * @brief HTTP client for async network requests
 *
 * Provides coroutine-based HTTP methods using QCoro.
 * Supports GET, POST, PUT, DELETE with timeout and retry.
 *
 * @code
 * HttpClient client;
 * auto response = co_await client.get(QUrl("https://api.example.com"));
 * @endcode
 *
 * @see WebSocketClient, NetworkManager
 */
class HttpClient : public QObject {
    Q_OBJECT
```

### Function Documentation

```cpp
/**
 * @brief Send GET request
 * @param url Request URL
 * @param headers Optional HTTP headers
 * @return Response data
 * @throws NetworkError on request failure
 * @throws TimeoutError if request times out
 *
 * @code
 * auto response = co_await client.get(QUrl("https://example.com"));
 * @endcode
 */
QCoro::Task<HttpResponse> get(const QUrl &url,
                              const HttpHeaders &headers = {});
```

### Property Documentation

```cpp
/**
 * @brief Current playback state
 * @see PlaybackState
 */
Q_PROPERTY(PlaybackState state READ state NOTIFY stateChanged)
```

### Enum Documentation

```cpp
/**
 * @brief Playback state
 */
enum class PlaybackState {
    Stopped,  ///< Playback stopped
    Playing,  ///< Currently playing
    Paused,   ///< Playback paused
    Loading   ///< Loading media
};
```

### Member Variable Documentation

```cpp
private:
    QNetworkAccessManager *m_manager; ///< Network manager instance
    int m_timeoutMs = 30000;          ///< Request timeout in milliseconds
    int m_maxRetries = 3;             ///< Maximum retry attempts
```

### Grouping

```cpp
/**
 * @name Authentication
 * @brief User authentication methods
 * @{
 */
QCoro::Task<LoginResult> login(const QString &user, const QString &pass);
QCoro::Task<VoidResult> logout();
bool isAuthenticated() const;
/** @} */
```

## Testing

```cpp
class HttpClientTest : public QObject {
    Q_OBJECT
private slots:
    void testGet_ValidUrl_ReturnsSuccess();
    void testGet_Timeout_ReturnsError();
};

void HttpClientTest::testGet_ValidUrl_ReturnsSuccess() {
    QCoro::waitFor([]() -> QCoro::Task<void> {
        HttpClient client;
        auto response = co_await client.get(QUrl("https://example.com"));
        QVERIFY(response.isSuccess());
    }());
}
```

## References

- [Qt 6 Docs](https://doc.qt.io/qt-6/)
- [QCoro Docs](https://qcoro.dev/)
- [Doxygen Manual](https://www.doxygen.nl/manual/)
- [Conventional Commits](https://www.conventionalcommits.org/)
