# NetEase Playback URL Cache Incident

## Summary

NetEase tracks could fail after being played two or more times. The UI reported one of the following errors:

```text
Failed to resolve playback URL for: <song>
```

```text
Platform returned an invalid playback URL for: <song>
```

In some cases no NetEase API request appeared in debug logging when playback failed. This indicated that `PlaybackController` was returning a cached URL instead of calling `NeteaseClient::getSongUrl()`.

The investigation found that this was not a single malformed-URL problem. Playlist loading generated a large number of duplicate background URL resolutions, while the playback URL cache had weak lifetime and recovery semantics.

## User-visible symptoms

- A NetEase song played successfully once or twice and then failed.
- Replaying a song sometimes produced no NetEase API request.
- Cached URLs were reported as invalid or expired shortly after they had worked.
- Loading a playlist generated many URL requests for only a few song IDs.
- Restarting the application could temporarily clear the problem because the playback URL cache was in memory.

## Evidence

Debug logs showed dozens of requests for the same three song IDs within approximately two milliseconds while one playlist was being loaded:

```text
10:05:32.223 EAPI request ... ids=[2660502243]
10:05:32.223 EAPI request ... ids=[1840235143]
10:05:32.223 EAPI request ... ids=[3378813781]
10:05:32.224 EAPI request ... ids=[2660502243]
10:05:32.224 EAPI request ... ids=[1840235143]
10:05:32.224 EAPI request ... ids=[3378813781]
...
```

The NetEase response included:

```json
{
  "expi": 1200
}
```

`expi` is the URL lifetime in seconds. In this example, NetEase advertised a 20-minute lifetime. The old cache ignored this value and used a fixed 30-minute TTL.

The logs also showed this sequence:

```text
Using cached playback URL for <song>
Discarding invalid or expired cached playback URL for <song>
```

The discard happened only seconds after the same entry had been accepted. It was therefore not normal expiration according to either the server-provided lifetime or the old fixed TTL.

The old log message combined the invalid and expired cases, so historical logs cannot determine which branch caused each discard. The new implementation logs these reasons separately.

## Root causes

### 1. Queue construction caused a pre-resolution request storm

`PlayerViewModel::loadQueueAndPlay()` previously built a queue like this:

```cpp
queue->clear();
for (const Song &song : songs) {
    queue->addSong(song);
}
```

Both `clear()` and every `addSong()` emit `PlayQueue::queueChanged()`.

`PlaybackController` listens to `queueChanged()` and pre-resolves the next three tracks. While the queue was being assembled, the same tracks were selected repeatedly. Earlier network requests had not completed yet, so their results were not in the cache and did not prevent duplicate requests.

For a large playlist, this created many concurrent requests for a small number of song IDs.

### 2. No per-song in-flight deduplication

The old pre-resolution path checked only whether a completed cache entry existed. It did not track resolutions that were already running.

Repeated `queueChanged()` signals could therefore start another request for the same platform and song ID before the first request completed.

### 3. URL and expiry were stored separately

The cache used two independent hashes:

```cpp
QHash<QString, QString> m_urlCache;
QHash<QString, qint64> m_urlCacheExpiry;
```

A valid cache entry depended on matching values being present in both containers. The duplicate completion storm repeatedly overwrote both structures. This representation made inconsistent URL/expiry state possible and made diagnosis difficult.

The cache now stores the URL and expiry atomically in one entry.

### 4. Server-provided expiration was ignored

NetEase supplies an `expi` field in its playback URL response. The old parser discarded it, and `PlaybackController` always cached URLs for 30 minutes.

A URL could therefore be reused after NetEase intended it to expire. The new cache uses `expi`, converted to milliseconds, and subtracts a safety margin before considering the URL reusable.

### 5. Backend rejection did not invalidate the cache

A URL can remain syntactically valid while its CDN token is no longer accepted. Previously, if Qt Multimedia rejected a cached URL, playback surfaced the backend error without removing and refreshing that URL.

The same rejected URL could consequently be selected again on the next playback attempt.

## Resolution

### Atomic queue replacement

`PlayerViewModel::loadQueueAndPlay()` now calls:

```cpp
queue->setSongs(songs);
```

This emits one queue change instead of one change per song.

### In-flight pre-resolution guard

`PlaybackController` tracks platform-qualified song keys currently being resolved:

```text
<platform>:<song-id>
```

If a key is already in flight, another background request is not started.

Platform qualification prevents IDs from different music services from sharing a cache entry.

### Atomic cache entries

The cache now stores both values together:

```cpp
struct CachedUrl {
    QString url;
    qint64 expiresAtMs;
};
```

A cache entry cannot have a URL without its corresponding expiry timestamp.

### Server-derived TTL

`NeteaseParser::parseSongUrl()` maps `expi` to `SongUrlResult::expiresInMs`.

The usable cache lifetime is:

```text
server lifetime - 30-second safety margin
```

If a platform does not provide a lifetime, the player uses a conservative 10-minute default.

### Cached URL validation

Before insertion or reuse, a remote playback URL must be:

- non-empty;
- valid according to `QUrl::StrictMode`;
- absolute;
- HTTP or HTTPS;
- associated with a non-empty host.

Malformed values such as quoted URLs are not cached.

### Refresh after backend rejection

If loading a remote URL fails, `PlaybackController` now:

1. logs the initial load failure;
2. evicts the cached URL;
3. calls the platform plugin with a forced refresh;
4. loads the refreshed URL;
5. retries playback once.

Only one retry is allowed to avoid a failure loop.

## Logging and diagnosis

Enable debug logging with:

```bash
QERIPLAYER_LOG_LEVEL=debug ./QeriPlayerQt
```

Relevant messages now include:

```text
Using cached playback URL for <song> (remainingMs=<milliseconds>)
Discarding expired cached playback URL for <song>
Discarding invalid cached playback URL for <song>
Playback URL failed for <song>, refreshing once: <backend error>
```

Expected behavior when loading a queue:

- one URL request for the current track when playback begins;
- at most one background request for each of the next three tracks;
- no repeated burst of requests for the same song ID.

Expected behavior when a cached CDN URL is rejected:

- one cache-hit message;
- one backend-failure warning;
- one new NetEase API request;
- playback continues with the refreshed URL if the retry succeeds.

## Regression coverage

Tests cover the following behavior:

- NetEase `expi: 1200` becomes `expiresInMs == 1,200,000`.
- A malformed pre-resolved URL is not cached.
- Loading a queue resolves each current/near-future song at most once.
- A backend-rejected cached URL is evicted and refreshed exactly once.
- A `RequiresLogin` result is surfaced as a specific playback error.

The complete test suite passes after the changes.

## Related files

- `src/api/netease/NeteaseParser.cpp`
- `src/domain/SongUrlResult.h`
- `src/player/PlaybackController.h`
- `src/player/PlaybackController.cpp`
- `src/viewmodel/PlayerViewModel.cpp`
- `tests/api/TestNeteaseParser.cpp`
- `tests/player/TestPlaybackController.cpp`
- `tests/viewmodel/TestPlayerViewModel.cpp`
