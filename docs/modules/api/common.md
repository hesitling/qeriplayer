# API Common Types (api/common/)

## Overview

Common types shared across all platform API integrations: error handling, result types, login data, and the abstract plugin interface.

## Source Files

```
src/api/common/
├── ApiError.h / .cpp         # Error with code classification
├── ApiResult.h               # Result<T> template (value or error)
├── VoidResult.h              # Empty success type
├── LoginResult.h             # Login success data
├── QrCodeData.h              # QR code login data
├── PlayHistory.h             # Play history entry
└── IMusicPlatformPlugin.h    # Abstract platform interface
```

## ApiError

Stores an integer error code, message, and optional details. Provides classification methods.

```cpp
class ApiError {
public:
    ApiError(int code, const QString &message, const QString &details = {});

    int code() const;
    const QString &message() const;
    const QString &details() const;

    bool isNetworkError() const;   // code == -1
    bool isAuthError() const;      // HTTP 401/403, NetEase -10, -460
    bool isRateLimitError() const; // HTTP 429, NetEase -429
    bool isNotFoundError() const;  // HTTP 404

    QString userMessage() const;   // User-facing description
};
```

### NetEase Error Code Mapping

| Method | HTTP Codes | NetEase Body Codes |
|--------|-----------|-------------------|
| `isNetworkError()` | -1 (connection failure) | — |
| `isAuthError()` | 401, 403 | -10 (auth expired), -460 (cheating) |
| `isRateLimitError()` | 429 | -429 |
| `isNotFoundError()` | 404 | — |

## ApiResult\<T\>

Holds either a value of type `T` or an `ApiError`. Used as the return type for all API operations.

```cpp
template <typename T>
class ApiResult {
public:
    explicit ApiResult(T value);
    explicit ApiResult(ApiError error);

    bool isSuccess() const;
    bool isError() const;
    explicit operator bool() const;  // true if success

    const T &data() const;     // Asserts if error
    const ApiError &error() const; // Asserts if success
};
```

## VoidResult

Empty struct for operations that return no data on success. Use `ApiResult<VoidResult>`.

```cpp
struct VoidResult {};
```

## LoginResult

Result of a successful login.

```cpp
struct LoginResult {
    QString userId;
    QString nickname;
    QUrl avatarUrl;
    QString cookie; // Semicolon-delimited "key=val; key=val"
};
```

## QrCodeData

Data for QR code-based login flow.

```cpp
struct QrCodeData {
    QString key;              // Polling key
    QUrl qrUrl;               // QR code image URL
    int expiresInSeconds = 0;
};
```

## PlayHistory

A record of a song play event.

```cpp
struct PlayHistory {
    Song song;
    qint64 playedAt = 0; // Epoch ms
    int playCount = 0;
};
```

## IMusicPlatformPlugin

Abstract interface that all platform clients implement. Defines the cross-platform contract for search, song details, playback URLs, lyrics, and auth status.

```cpp
class IMusicPlatformPlugin {
public:
    virtual ~IMusicPlatformPlugin() = default;

    virtual QCoro::Task<ApiResult<SearchResult>> search(
        const QString &keyword, SearchType type, int limit, int offset) = 0;
    virtual QCoro::Task<ApiResult<Song>> getSongDetail(const QString &songId) = 0;
    virtual QCoro::Task<ApiResult<SongUrlResult>> getSongUrl(
        const QString &songId, AudioQuality quality = AudioQuality::High) = 0;
    virtual QCoro::Task<ApiResult<Lyrics>> getLyrics(const QString &songId) = 0;
    virtual bool isAuthenticated() const = 0;
    virtual QString platformName() const = 0;
};
```

### Design Rationale

- **Minimal interface** — only operations that make sense across all platforms. Platform-specific operations (playlist CRUD, user data, recommendations) live on the concrete client class.
- **`isAuthenticated()` is synchronous** — it's an in-memory check (cookie/token presence), not a network call.
- **All methods return `ApiResult<T>`** — callers must check `isSuccess()` before accessing `data()`.

## Q_DECLARE_METATYPE Registration

All common types are registered with `Q_DECLARE_METATYPE` for QVariant interop: `ApiError`, `VoidResult`, `LoginResult`, `QrCodeData`, `PlayHistory`.
