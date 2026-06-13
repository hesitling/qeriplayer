# API Module (api/)

## 1. Overview

The API module encapsulates API clients for various music platforms, providing a unified interface to access online resources from NetEase Cloud Music, Bilibili, YouTube Music, and other platforms.

## 2. Module Composition

```
src/api/
├── netease/              # NetEase Cloud Music
├── bilibili/             # Bilibili
├── youtube/              # YouTube Music
├── qqmusic/              # QQ Music
└── common/               # Common types
```

## 3. Submodule Documents

- [Common Types](common.md) - Music types, search types, API errors
- [NetEase Cloud Music](netease.md) - NetEase Cloud Music API client
- [Bilibili](bilibili.md) - Bilibili API client
- [YouTube Music](youtube.md) - YouTube Music API client
- [QQ Music](qqmusic.md) - QQ Music API client

## 4. Module Dependencies

```
┌─────────────────────────────────────────────────────────────┐
│                      Search Module (search/)                 │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                       API Module (api/)                      │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐        │
│  │ Netease  │ │ Bilibili │ │ YouTube  │ │ QQMusic  │        │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘        │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                       Core Module (core/)                    │
│                      Network (HttpClient)                    │
└─────────────────────────────────────────────────────────────┘
```

## 5. Design Principles

- **Unified Interface**: All platform clients implement the same interface
- **Async Operations**: All API calls return QFuture
- **Error Handling**: Unified ApiError error handling
- **Extensible**: Support for adding new platform plugins

## 6. Supported Platforms

| Platform | Search | Playback | Lyrics | Playlists | Download |
|----------|--------|----------|--------|-----------|----------|
| NetEase Cloud Music | ✅ | ✅ | ✅ | ✅ | ✅ |
| Bilibili | ✅ | ✅ | ❌ | ✅ | ✅ |
| YouTube Music | ✅ | ✅ | ✅ | ✅ | ✅ |
| QQ Music | ✅ | ❌ | ✅ | ❌ | ❌ |
