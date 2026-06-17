## ADDED Requirements

### Requirement: SearchViewModel dispatches to IMusicPlatformPlugin
The system SHALL define `SearchViewModel` as a `QObject` that searches songs via `IMusicPlatformPlugin::search()`, dispatching to the plugin matching `selectedPlatform`.

#### Scenario: Search dispatched to NetEase
- **WHEN** `selectedPlatform == NetEase` and `search()` is called with query "hello"
- **THEN** the NetEase `IMusicPlatformPlugin` SHALL be called with `search("hello", SearchType::Song, 30, 0)`

### Requirement: Query property with debounce
`SearchViewModel` SHALL expose `query` (QString, read/write, notify: `queryChanged`). Setting `query` SHALL start a 300ms single-shot `QTimer`. When the timer fires, `search()` SHALL be called automatically.

#### Scenario: Debounce triggers search
- **WHEN** `query` is set to "hello"
- **THEN** a 300ms timer SHALL start. After 300ms, `search()` SHALL be called with the current query

#### Scenario: Rapid typing resets debounce
- **WHEN** `query` is set to "h", then "he", then "hel" within 300ms
- **THEN** only one `search()` call SHALL occur (for "hel"), 300ms after the last set

### Requirement: Platform selection
`SearchViewModel` SHALL expose `selectedPlatform` (MusicPlatform, read/write, notify: `selectedPlatformChanged`). Changing the platform SHALL clear results and re-search if query is non-empty. `availablePlatforms` (QVariantList, read-only) SHALL list registered plugin platforms.

#### Scenario: Platform change clears and re-searches
- **WHEN** `selectedPlatform` is changed from `NetEase` to `Bilibili` and `query` is "rock"
- **THEN** results SHALL be cleared and a new search SHALL be dispatched to the Bilibili plugin

### Requirement: Request versioning for race safety
`SearchViewModel` SHALL maintain a `quint64 m_searchRequestVersion` incremented on each `search()` call. When the async result arrives, if the version no longer matches, the result SHALL be discarded.

#### Scenario: Stale search result discarded
- **WHEN** search v1 is dispatched for "rock", then search v2 is dispatched for "pop", then v1 result arrives
- **THEN** the v1 result SHALL be discarded and the UI SHALL NOT be updated

### Requirement: Results as SongListModel
`SearchViewModel` SHALL expose `results` as a `SongListModel*` (read-only, notify: `resultsChanged`). On successful search, `results->setSongs(songs)` SHALL be called.

#### Scenario: Search results populated
- **WHEN** a search returns 20 songs successfully
- **THEN** `results->count()` SHALL be 20

### Requirement: Pagination via loadMore
`Q_INVOKABLE loadMore()` SHALL fetch the next page of results (incrementing offset by 30) and append via `results->appendSongs()`. `hasMore` (bool, read-only, notify: `hasMoreChanged`) SHALL indicate if more results exist.

#### Scenario: loadMore appends results
- **WHEN** `loadMore()` is called after an initial search with 30 results
- **THEN** the next page SHALL be fetched with offset 30 and appended to the model

### Requirement: Search result caching
On successful search, `SearchViewModel` SHALL call `ISongRepository::saveBatch(songs)` to cache results locally.

#### Scenario: Results cached in repository
- **WHEN** a search returns 20 songs
- **THEN** `ISongRepository::saveBatch()` SHALL be called with those 20 songs

### Requirement: Loading state
`SearchViewModel` SHALL expose `isLoading` (bool, read-only, notify: `isLoadingChanged`). It SHALL be `true` while a search is in flight and `false` when it completes or errors.

#### Scenario: Loading state during search
- **WHEN** `search()` is called
- **THEN** `isLoading` SHALL become `true`. When the result arrives, `isLoading` SHALL become `false`

### Requirement: Error state
`SearchViewModel` SHALL expose `hasError` (bool, notify: `errorChanged`) and `error` (ViewModelError, notify: `errorChanged`). API errors SHALL be mapped via `ViewModelError::fromApiError()`.

#### Scenario: API error surfaced
- **WHEN** `IMusicPlatformPlugin::search()` returns an `ApiError` with `isNetworkError() == true`
- **THEN** `hasError` SHALL be `true`, `error.type` SHALL be `Network`, `error.canRetry` SHALL be `true`

### Requirement: clearResults and clearError
`Q_INVOKABLE clearResults()` SHALL empty the results model and reset pagination. `Q_INVOKABLE clearError()` SHALL clear the error state.

#### Scenario: clearResults empties model
- **WHEN** `clearResults()` is called on a model with 50 results
- **THEN** `results->count()` SHALL be 0 and `hasMore` SHALL be `false`

### Requirement: requestPlay signal
`SearchViewModel` SHALL emit `requestPlay(const Song &song)` when the user selects a search result to play. `MainViewModel` wires this to `PlayerViewModel::play()`.

#### Scenario: Play signal emitted
- **WHEN** the user clicks a search result at index 3
- **THEN** `requestPlay` SHALL be emitted with the song at index 3 from `results`
