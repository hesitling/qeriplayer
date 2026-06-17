## Design: Netease Endpoint Cleanup

### Overview

This change removes deprecated/broken endpoints from `NeteaseClient` and aligns with the Kotlin version's validated API surface. The design prioritizes a clean, minimal API over feature completeness.

### Removed Endpoints

#### Category 1: Confirmed 404 (Broken)

These endpoints return HTTP 404 with "接口未找到！":

```
/weapi/search/hot/detail    → getHotSearches()
/weapi/artists               → getArtistDetail()
/weapi/simi/song             → getSimilarSongs()
/weapi/recommend/songs       → getDailyRecommendations()
```

**Action:** Remove methods entirely. Any callers must be updated to remove these features or use alternative approaches.

#### Category 2: Not in Kotlin (Likely Deprecated)

These endpoints exist in Qt but not in the Kotlin reference implementation:

```
/weapi/album/newest          → getNewAlbums()
/weapi/artist/album          → getArtistAlbums()
/weapi/artist/songs          → getArtistSongs()
/weapi/likelist              → getLikedSongs()
/weapi/personal_fm           → getPersonalFM()
/weapi/top/playlist          → getHighQualityPlaylists()
/weapi/toplist/artist        → getTopArtists()
/weapi/user/record           → getPlayHistory()
/weapi/playlist/create       → createPlaylist()
/weapi/playlist/delete       → deletePlaylist()
/weapi/playlist/tracks       → addSongToPlaylist() / removeSongFromPlaylist()
```

**Action:** Remove methods entirely.

#### Category 3: QR Login (Not in Kotlin)

```
/weapi/login/qr/key          → generateQrCode()
/weapi/login/qr/create       → (part of QR flow)
/weapi/login/qr/check        → checkQrCodeStatus()
```

**Action:** Remove QR login methods. Keep phone and email login.

### Updated Endpoints

#### getRecommendedPlaylists()

**Current:** `/weapi/personalized`
**Correct:** `/weapi/personalized/playlist`

The Kotlin version uses `/weapi/personalized/playlist` which returns a different response structure (`result` array with playlist objects including `copywriter` field).

#### getHighQualityPlaylists()

**Current:** `/weapi/top/playlist`
**Correct:** `/weapi/playlist/highquality/list` via `callWeApi`

The Kotlin version calls this through `callWeApi` helper which wraps the path with `/weapi` prefix.

### New Endpoints

#### getLikedSongIds()

```kotlin
// Kotlin reference
fun getUserLikedSongIds(userId: Long): String {
    val url = "https://music.163.com/weapi/song/like/get"
    val params = mapOf("uid" to uid.toString())
    return request(url, params, CryptoMode.WEAPI, "POST", usePersistedCookies = true)
}
```

Returns array of liked song IDs. Can be combined with `getSongDetail()` to get full song info.

#### getCurrentUserAccount()

```kotlin
fun getCurrentUserAccount(): String {
    return callWeApi("/w/nuser/account/get", emptyMap(), usePersistedCookies = true)
}
```

Returns current user profile including `userId`.

#### getSongDownloadUrl()

```kotlin
fun getSongDownloadUrl(songId: Long, level: String = "lossless"): String {
    val params = mutableMapOf<String, Any>(
        "ids" to "[$songId]",
        "level" to level,
        "encodeType" to "flac",
    )
    return callEApi("/song/enhance/player/url/v1", params, usePersistedCookies = usePersistedCookies)
}
```

EAPI endpoint for download URLs with quality levels: `standard`, `exhigh`, `lossless`, `hires`, `jyeffect`, `sky`, `jymaster`.

#### getHighQualityTags()

```kotlin
fun getHighQualityTags(): String {
    val url = "https://music.163.com/api/playlist/highquality/tags"
    return request(url, emptyMap(), CryptoMode.WEAPI, "POST", usePersistedCookies = true)
}
```

Returns category tags for high-quality playlists.

#### getUserAlbums()

```kotlin
fun getUserAlbums(userId: Long, offset: Int = 0, limit: Int = 30): String {
    val url = "https://interface3.music.163.com/eapi/mine/rn/resource/list"
    val params = mutableMapOf<String, Any>(
        "userId" to userId.toString(),
        "offset" to offset.toString(),
        "limit" to limit.toString(),
        "pageType" to "3",
        "needRcmd" to "0",
        "isVistor" to "false",
        "includeStarPodcast" to "true"
    )
    return request(url, params, CryptoMode.EAPI, "POST", usePersistedCookies = true)
}
```

EAPI endpoint on `interface3.music.163.com` for user's collected albums.

#### getUserDjRadios()

```kotlin
fun getUserDjRadios(userId: Long, offset: Int = 0, limit: Int = 30): String {
    val url = "https://music.163.com/weapi/user/djradio/get/subed"
    val params = mutableMapOf<String, Any>(
        "uid" to userId.toString(),
        "offset" to offset.toString(),
        "limit" to limit.toString()
    )
    return request(url, params, CryptoMode.WEAPI, "POST", usePersistedCookies = true)
}
```

### File Changes

#### NeteaseClient.h

Remove method declarations:
- `getHotSearches()`
- `getSimilarSongs()`
- `getRecommendedPlaylists()` (will be re-added with correct endpoint)
- `getHighQualityPlaylists()` (will be re-added with correct endpoint)
- `createPlaylist()`
- `deletePlaylist()`
- `addSongToPlaylist()`
- `removeSongFromPlaylist()`
- `getNewAlbums()`
- `getArtistDetail()`
- `getArtistSongs()`
- `getArtistAlbums()`
- `getTopArtists()`
- `getLikedSongs()`
- `getPlayHistory()`
- `getDailyRecommendations()`
- `getNewSongs()`
- `getPersonalFM()`
- `generateQrCode()`
- `checkQrCodeStatus()`

Add method declarations:
- `getLikedSongIds(userId)`
- `getCurrentUserAccount()`
- `getSongDownloadUrl(songId, level)`
- `getHighQualityTags()`
- `getUserAlbums(userId, offset, limit)`
- `getUserDjRadios(userId, offset, limit)`

#### NeteaseClient.cpp

- Remove implementations of deleted methods
- Update `getRecommendedPlaylists()` to use `/weapi/personalized/playlist`
- Update `getHighQualityPlaylists()` to use `/weapi/playlist/highquality/list`
- Add implementations of new methods

#### NeteaseParser.h / NeteaseParser.cpp

- Remove `parseHotSearches()` if only used by deleted endpoint
- Remove `parsePlayHistory()` if only used by deleted endpoint
- Add any new parsers needed for new endpoints

#### TestNeteaseE2E.cpp

- Remove test methods for deleted endpoints
- Update test methods for changed endpoints
- Add test methods for new endpoints

#### IMusicPlatformPlugin.h

- Check if any removed methods are part of the interface
- If so, update the interface (this may affect other implementations)

### Implementation Order

1. Remove deprecated methods from header and implementation
2. Update endpoint patterns for changed methods
3. Add new methods from Kotlin
4. Update E2E tests
5. Verify build compiles
6. Run tests to verify no regressions
