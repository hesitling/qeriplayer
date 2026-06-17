# API Module (api/)

## Overview

The API module contains platform-specific API clients that implement `IMusicPlatformPlugin`. Each client handles authentication, content browsing, playback URL resolution, and platform-specific features.

## Module Structure

```
src/api/
├── common/                  # Shared types and interfaces
│   ├── ApiError.h           # Error classification
│   ├── ApiResult.h          # Result<T> template
│   ├── VoidResult.h         # Empty success type
│   ├── LoginResult.h        # Login result
│   ├── QrCodeData.h         # QR code login data
│   ├── PlayHistory.h        # Play history entry
│   └── IMusicPlatformPlugin.h  # Abstract interface
├── netease/                 # NetEase Cloud Music
│   ├── NeteaseClient.h
│   ├── NeteaseCrypto.h
│   └── NeteaseParser.h
├── bilibili/                # (planned)
├── youtube/                 # (planned)
└── qqmusic/                 # (planned)
```

## Submodule Documents

- [Common Types](common.md) — `ApiError`, `ApiResult<T>`, `VoidResult`, `LoginResult`, `QrCodeData`, `PlayHistory`, `IMusicPlatformPlugin`
- [NetEase Cloud Music](netease.md) — `NeteaseClient`, `NeteaseCrypto`, `NeteaseParser`

## Plugin Interface

All platform clients implement `IMusicPlatformPlugin`:

```cpp
class IMusicPlatformPlugin {
public:
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

Platform-specific operations (playlist CRUD, user data, recommendations) live on the concrete client class, not on the interface.

## Implementation Status

| Platform | Status | Notes |
|----------|--------|-------|
| NetEase | ✅ Implemented | Full auth, search, playback, playlists, user data |
| Bilibili | 📋 Planned | |
| YouTube Music | 📋 Planned | |
| QQ Music | 📋 Planned | |
