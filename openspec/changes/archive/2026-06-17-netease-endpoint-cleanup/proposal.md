## Why

The Qt NeteaseClient was built by porting endpoints from various sources (web browser network inspector, old documentation), but many of these endpoints are now deprecated or broken (returning 404). Meanwhile, the Kotlin QeriPlayer has a smaller, validated set of endpoints that actually work. This change removes broken/deprecated endpoints and aligns the Qt client with the Kotlin version's proven API surface.

## What Changes

### Endpoints to REMOVE (returning 404 or not in Kotlin):

| Endpoint | Method | Status |
|----------|--------|--------|
| `/weapi/search/hot/detail` | `getHotSearches()` | 404 |
| `/weapi/artists` | `getArtistDetail()` | 404 |
| `/weapi/simi/song` | `getSimilarSongs()` | 404 |
| `/weapi/recommend/songs` | `getDailyRecommendations()` | 404 |
| `/weapi/album/newest` | `getNewAlbums()` | Not in Kotlin |
| `/weapi/artist/album` | `getArtistAlbums()` | Not in Kotlin |
| `/weapi/artist/songs` | `getArtistSongs()` | Not in Kotlin |
| `/weapi/likelist` | `getLikedSongs()` | Not in Kotlin |
| `/weapi/personal_fm` | `getPersonalFM()` | Not in Kotlin |
| `/weapi/personalized` | `getRecommendedPlaylists()` | Wrong pattern |
| `/weapi/top/playlist` | `getHighQualityPlaylists()` | Not in Kotlin |
| `/weapi/toplist/artist` | `getTopArtists()` | Not in Kotlin |
| `/weapi/user/record` | `getPlayHistory()` | Not in Kotlin |
| `/weapi/playlist/create` | `createPlaylist()` | 301 session issue |
| `/weapi/playlist/delete` | `deletePlaylist()` | Not in Kotlin |
| `/weapi/playlist/tracks` | `addSongToPlaylist()` / `removeSongFromPlaylist()` | Not in Kotlin |
| `/weapi/login/qr/*` | QR login methods | Not in Kotlin |

### Endpoints to UPDATE (wrong pattern):

| Method | Current Endpoint | Correct Endpoint (from Kotlin) |
|--------|------------------|-------------------------------|
| `getRecommendedPlaylists()` | `/weapi/personalized` | `/weapi/personalized/playlist` |
| `getHighQualityPlaylists()` | `/weapi/top/playlist` | `/weapi/playlist/highquality/list` via `callWeApi` |

### New endpoints to ADD (from Kotlin):

| Method | Endpoint | Notes |
|--------|----------|-------|
| `getLikedSongIds()` | `/weapi/song/like/get` | Get liked song IDs (not full song details) |
| `getCurrentUserAccount()` | `/weapi/w/nuser/account/get` | Get current user info |
| `getSongDownloadUrl()` | EAPI `/song/enhance/player/url/v1` | Download URL with quality levels |
| `getHighQualityTags()` | `/api/playlist/highquality/tags` | HQ playlist category tags |
| `getUserAlbums()` | `interface3.music.163.com/eapi/mine/rn/resource/list` | User's collected albums |
| `getUserDjRadios()` | `/weapi/user/djradio/get/subed` | User's subscribed DJ radios |

## Capabilities

### Modified Capabilities

- `netease-client`: Remove broken endpoints, fix endpoint patterns, add missing Kotlin-validated endpoints.

## Impact

- Breaking changes: Public API methods will be removed from `NeteaseClient`.
- Callers of removed methods must be updated or removed.
- E2E tests must be updated to skip/remove tests for deprecated endpoints.
- Header file `NeteaseClient.h` will shrink significantly.
