## 1. Remove Deprecated Methods from NeteaseClient.h

- [x] 1.1 Remove `getHotSearches()` declaration
- [x] 1.2 Remove `getSimilarSongs()` declaration
- [x] 1.3 Remove `createPlaylist()` declaration
- [x] 1.4 Remove `deletePlaylist()` declaration
- [x] 1.5 Remove `addSongToPlaylist()` declaration
- [x] 1.6 Remove `removeSongFromPlaylist()` declaration
- [x] 1.7 Remove `getNewAlbums()` declaration
- [x] 1.8 Remove `getArtistDetail()` declaration
- [x] 1.9 Remove `getArtistSongs()` declaration
- [x] 1.10 Remove `getArtistAlbums()` declaration
- [x] 1.11 Remove `getTopArtists()` declaration
- [x] 1.12 Remove `getLikedSongs()` declaration
- [x] 1.13 Remove `getPlayHistory()` declaration
- [x] 1.14 Remove `getDailyRecommendations()` declaration
- [x] 1.15 Remove `getNewSongs()` declaration
- [x] 1.16 Remove `getPersonalFM()` declaration
- [x] 1.17 Remove `generateQrCode()` declaration
- [x] 1.18 Remove `checkQrCodeStatus()` declaration

## 2. Remove Deprecated Methods from NeteaseClient.cpp

- [x] 2.1 Remove `getHotSearches()` implementation
- [x] 2.2 Remove `getSimilarSongs()` implementation
- [x] 2.3 Remove `createPlaylist()` implementation
- [x] 2.4 Remove `deletePlaylist()` implementation
- [x] 2.5 Remove `addSongToPlaylist()` implementation
- [x] 2.6 Remove `removeSongFromPlaylist()` implementation
- [x] 2.7 Remove `getNewAlbums()` implementation
- [x] 2.8 Remove `getArtistDetail()` implementation
- [x] 2.9 Remove `getArtistSongs()` implementation
- [x] 2.10 Remove `getArtistAlbums()` implementation
- [x] 2.11 Remove `getTopArtists()` implementation
- [x] 2.12 Remove `getLikedSongs()` implementation
- [x] 2.13 Remove `getPlayHistory()` implementation
- [x] 2.14 Remove `getDailyRecommendations()` implementation
- [x] 2.15 Remove `getNewSongs()` implementation
- [x] 2.16 Remove `getPersonalFM()` implementation
- [x] 2.17 Remove `generateQrCode()` implementation
- [x] 2.18 Remove `checkQrCodeStatus()` implementation

## 3. Update Existing Endpoints

- [x] 3.1 Update `getRecommendedPlaylists()` to use `/weapi/personalized/playlist`
- [x] 3.2 Update `getHighQualityPlaylists()` to use `/weapi/playlist/highquality/list` via `callWeApi`

## 4. Add New Methods from Kotlin

- [x] 4.1 Add `getLikedSongIds(userId)` — `/weapi/song/like/get`
- [x] 4.2 Add `getCurrentUserAccount()` — `/weapi/w/nuser/account/get`
- [x] 4.3 Add `getSongDownloadUrl(songId, level)` — EAPI `/song/enhance/player/url/v1`
- [x] 4.4 Add `getHighQualityTags()` — `/api/playlist/highquality/tags`
- [x] 4.5 Add `getUserAlbums(userId, offset, limit)` — `interface3.music.163.com/eapi/mine/rn/resource/list`
- [x] 4.6 Add `getUserDjRadios(userId, offset, limit)` — `/weapi/user/djradio/get/subed`

## 5. Update E2E Tests

- [x] 5.1 Remove `testGetHotSearches()` from TestNeteaseE2E.cpp
- [x] 5.2 Remove `testGetSimilarSongs()` from TestNeteaseE2E.cpp
- [x] 5.3 Remove `testCreatePlaylist()` from TestNeteaseE2E.cpp
- [x] 5.4 Remove `testDeletePlaylist()` from TestNeteaseE2E.cpp
- [x] 5.5 Remove `testAddSongToPlaylist()` from TestNeteaseE2E.cpp
- [x] 5.6 Remove `testRemoveSongFromPlaylist()` from TestNeteaseE2E.cpp
- [x] 5.7 Remove `testGetArtistDetail()` from TestNeteaseE2E.cpp
- [x] 5.8 Remove `testGetDailyRecommendations()` from TestNeteaseE2E.cpp
- [x] 5.9 Update `testGetPlaylistDetail()` if needed for new endpoint pattern
- [x] 5.10 Add tests for new endpoints (getLikedSongIds, getCurrentUserAccount, etc.)

## 6. Update IMusicPlatformPlugin Interface

- [x] 6.1 Check if `IMusicPlatformPlugin` has methods that are being removed
- [x] 6.2 Update interface if needed (may affect other implementations)

## 7. Update NeteaseParser

- [x] 7.1 Remove `parseHotSearches()` if only used by deleted endpoint
- [x] 7.2 Remove `parsePlayHistory()` if only used by deleted endpoint
- [x] 7.3 Add any new parsers needed for new endpoints

## 8. Build and Test Verification

- [x] 8.1 Verify build compiles without errors
- [x] 8.2 Run all existing tests to verify no regressions
- [x] 8.3 Run E2E tests with valid credentials to verify new endpoints work

## 9. Add Missing Kotlin APIs

- [x] 9.1 Add `getDjRadioDetail(radioId)` — `/api/v6/playlist/detail` (same as playlist detail)
- [x] 9.2 Add `getRelatedPlaylists(playlistId)` — HTML scrape from `/playlist?id=xxx`
- [x] 9.3 Add `getUserCreatedPlaylists(userId)` — wrapper around `getUserPlaylists` filtering by creator
- [x] 9.4 Add `getUserStaredAlbums(userId)` — wrapper around `getUserAlbums` filtering subscribed
- [x] 9.5 Add `getUserSubscribedPlaylists(userId)` — wrapper around `getUserPlaylists` filtering subscribed
- [x] 9.6 Add `getLikedPlaylistId(userId)` — wrapper around `getUserPlaylists` finding specialType=5
- [x] 9.7 Add `getCurrentUserId()` — helper using `getCurrentUserAccount()`
- [x] 9.8 Add `loginByCaptcha(phone, captcha, ctcode)` — EAPI `/w/login/cellphone` with captcha
- [x] 9.9 Add `sendCaptcha(phone, ctcode)` — `/weapi/sms/captcha/sent`
- [x] 9.10 Add `verifyCaptcha(phone, captcha, ctcode)` — `/weapi/sms/captcha/verify`

## 10. Fix Parameter Differences

- [x] 10.1 Update `getLyrics()` params to match Kotlin: lv=0, add rv=0, yv=1, ytv=1, yrv=0
- [x] 10.2 Add 301 retry logic to `getLyrics()` like Kotlin `getLyricNew()`

## 11. Remove Qt-Only Methods (Not in Kotlin)

- [x] 11.1 Remove `searchPlaylists()` — Kotlin only has `searchSongs(type=1)`
- [x] 11.2 Remove `searchAlbums()` — Kotlin only has `searchSongs(type=1)`
- [x] 11.3 Remove `searchArtists()` — Kotlin only has `searchSongs(type=1)`
- [x] 11.4 Decide on `loginByEmail()` — keep or remove? **Keep** — useful for desktop users
- [x] 11.5 Decide on `unlikeSong()` — keep as convenience or remove? **Keep** — cleaner API
