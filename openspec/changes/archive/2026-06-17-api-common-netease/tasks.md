## 1. API Common Types

- [x] 1.1 Create `src/api/common/` directory structure
- [x] 1.2 Define `ApiError` class with code, message, details, and classification methods (`isNetworkError`, `isAuthError`, `isRateLimitError`, `isNotFoundError`, `userMessage`)
- [x] 1.3 Define `ApiResult<T>` template with `isSuccess()`, `isError()`, `data()`, `error()`, and implicit bool conversion
- [x] 1.4 Define `VoidResult` as empty struct (`struct VoidResult {};`) for no-payload operations
- [x] 1.5 Define `LoginResult` struct (userId, nickname, avatarUrl, cookie) — cookie is semicolon-delimited key=value string
- [x] 1.7 Define `QrCodeData` struct (key, qrUrl, expiresInSeconds)
- [x] 1.8 Define `PlayHistory` struct (song, playedAt, playCount)
- [x] 1.9 Define `IMusicPlatformPlugin` abstract interface with search, getSongDetail, getSongUrl (returns `SongUrlResult`), getLyrics, isAuthenticated, platformName
- [x] 1.10 Register all API common types with `Q_DECLARE_METATYPE` (SongUrlResult/AudioInfo already registered in domain)

## 2. NeteaseCrypto

- [x] 2.1 Create `src/api/netease/` directory structure
- [x] 2.2 Define `NeteaseCrypto` class with static encrypt/decrypt method signatures
- [x] 2.3 Implement WeAPI AES-128-CBC encryption with hardcoded key and random IV
- [x] 2.4 Implement WeAPI RSA encryption for the AES key
- [x] 2.5 Implement `weapiEncrypt()` combining AES + RSA per the NetEase web protocol (one-way client→server encryption)
- [x] 2.6 Write unit tests verifying `weapiEncrypt` output against known test vectors (with fixed IV for determinism)

## 3. NeteaseParser

- [x] 3.1 Create `NeteaseParser` header with static method declarations for all parse targets
- [x] 3.2 Implement `parseSong()` — JSON to `Song` with all fields mapped
- [x] 3.3 Implement `parseAlbum()` — JSON to `Album`
- [x] 3.4 Implement `parseArtist()` — JSON to `Artist`
- [x] 3.5 Implement `parsePlaylist()` — JSON to `Playlist` (without songs)
- [x] 3.6 Implement `parsePlaylistDetail()` — JSON to `Playlist` with songs populated
- [x] 3.7 Implement `parseLyrics()` — JSON to `Lyrics` with timed lines
- [x] 3.8 Implement `parseSearchResult()` — JSON to `SearchResult` dispatching by search type
- [x] 3.9 Implement `parseLoginResult()` — JSON to `LoginResult`
- [x] 3.10 Implement `parseSongUrl()` — JSON to `SongUrlResult` (existing domain type)
- [x] 3.11 Implement `parseHotSearches()` — JSON to `QStringList`
- [x] 3.12 Implement `parsePlayHistory()` — JSON to `QVector<PlayHistory>`
- [x] 3.13 Add logging for malformed JSON with `Logger::get("api")` in each parser method
- [x] 3.14 Create recorded JSON fixture files under `tests/fixtures/netease/` for each endpoint
- [x] 3.15 Write unit tests for all parser methods using fixture files

## 4. NeteaseClient — Core

- [x] 4.1 Create `NeteaseClient` header implementing `IMusicPlatformPlugin`
- [x] 4.2 Implement constructor taking `HttpClient*` and optional `SecureStorage*`
- [x] 4.3 Implement `setBaseUrl()` and default base URL (`https://music.163.com/api`)
- [x] 4.4 Implement private `makeRequest()` helper — POST with WeAPI encryption, cookie injection, JSON response parsing
- [x] 4.5 Implement cookie/CSRF token injection into request headers
- [x] 4.6 Implement cookie persistence — load from `SecureStorage` on construction, save on login, clear on logout

## 5. NeteaseClient — Authentication

- [x] 5.1 Implement `login(phone, password)` — call `/login/cellphone`, parse result, persist cookies
- [x] 5.2 Implement `loginByEmail(email, password)` — call `/login`, parse result, persist cookies
- [x] 5.3 Implement `generateQrCode()` — call `/login/qr/key` and `/login/qr/create`, return `QrCodeData`
- [x] 5.4 Implement `checkQrCodeStatus(key)` — call `/login/qr/check`, return `LoginResult` on success
- [x] 5.5 Implement `logout()` — call `/logout`, clear cookies from `SecureStorage`
- [x] 5.6 Implement `isAuthenticated()` — return current auth state
- [x] 5.7 Implement `platformName()` — return `"NetEase"`

## 6. NeteaseClient — Search

- [x] 6.1 Implement `search(keyword, type, limit, offset)` — dispatch to `/cloudsearch/pc` with type parameter
- [x] 6.2 Implement `searchSongs()` — convenience wrapper calling `search` with `SearchType::Song`
- [x] 6.3 Implement `searchPlaylists()` — convenience wrapper calling `search` with `SearchType::Playlist`
- [x] 6.4 Implement `searchAlbums()` — convenience wrapper calling `search` with `SearchType::Album`
- [x] 6.5 Implement `searchArtists()` — convenience wrapper calling `search` with `SearchType::Artist`
- [x] 6.6 Implement `getHotSearches()` — call `/search/hot/detail`, parse to `QStringList`

## 7. NeteaseClient — Songs

- [x] 7.1 Implement `getSongDetail(songId)` — call `/song/detail`, parse to `Song`
- [x] 7.2 Implement `getSongUrl(songId, quality)` — call `/song/url`, parse to `SongUrlResult`
- [x] 7.3 Implement `getLyrics(songId)` — call `/lyric`, parse to `Lyrics`
- [x] 7.4 Implement `getSimilarSongs(songId)` — call `/simi/song`, parse to `QVector<Song>`

## 8. NeteaseClient — Playlists

- [x] 8.1 Implement `getPlaylistDetail(playlistId)` — call `/playlist/detail`, parse to `Playlist` with songs
- [x] 8.2 Implement `getUserPlaylists(userId)` — call `/user/playlist`, parse to `QVector<Playlist>`
- [x] 8.3 Implement `getRecommendedPlaylists()` — call `/personalized`, parse to `QVector<Playlist>`
- [x] 8.4 Implement `getHighQualityPlaylists(category, limit)` — call `/top/playlist`, parse to `QVector<Playlist>`
- [x] 8.5 Implement `createPlaylist(name, description)` — POST to `/playlist/create`, parse to `Playlist`
- [x] 8.6 Implement `deletePlaylist(playlistId)` — POST to `/playlist/delete`, return `VoidResult`
- [x] 8.7 Implement `addSongToPlaylist(playlistId, songId)` — POST to `/playlist/tracks` with op=add
- [x] 8.8 Implement `removeSongFromPlaylist(playlistId, songId)` — POST to `/playlist/tracks` with op=del

## 9. NeteaseClient — Albums, Artists, User, Recommendations

- [x] 9.1 Implement `getAlbumDetail(albumId)` — call `/album`, parse to `Album` with songs
- [x] 9.2 Implement `getNewAlbums(limit)` — call `/album/newest`, parse to `QVector<Album>`
- [x] 9.3 Implement `getArtistDetail(artistId)` — call `/artists`, parse to `Artist` with songs
- [x] 9.4 Implement `getArtistSongs(artistId, limit, offset)` — call `/artist/songs`, parse to `QVector<Song>`
- [x] 9.5 Implement `getArtistAlbums(artistId, limit, offset)` — call `/artist/album`, parse to `QVector<Album>`
- [x] 9.6 Implement `getTopArtists(limit)` — call `/toplist/artist`, parse to `QVector<Artist>`
- [x] 9.7 Implement `getLikedSongs(userId)` — call `/likelist`, parse to `QVector<Song>`
- [x] 9.8 Implement `likeSong(songId)` / `unlikeSong(songId)` — POST to `/like`
- [x] 9.9 Implement `getPlayHistory()` — call `/user/record`, parse to `QVector<PlayHistory>`
- [x] 9.10 Implement `getDailyRecommendations()` — call `/recommend/songs`, parse to `QVector<Song>`
- [x] 9.11 Implement `getNewSongs()` — call `/personalized/newsong`, parse to `QVector<Song>`
- [x] 9.12 Implement `getPersonalFM()` — call `/personal_fm`, parse to `QVector<Song>`

## 10. Service Registration & Build

- [x] 10.1 Register `NeteaseClient` in `NeriPlayerApplication::initializeCoreServices()` after NetworkManager and SecureStorage
- [x] 10.2 Update `CMakeLists.txt` with all new source files (`src/api/common/`, `src/api/netease/`) and test targets (`TestNeteaseCrypto`, `TestNeteaseParser`)
- [x] 10.3 Verify full build compiles without errors
- [x] 10.4 Run all existing tests to verify no regressions
