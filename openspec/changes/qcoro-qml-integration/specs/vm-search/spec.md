## MODIFIED Requirements

### Requirement: Pagination via loadMore
`Q_INVOKABLE QCoro::QmlTask loadMore()` SHALL fetch the next page of results (incrementing offset by 30) and append via `results->appendSongs()`. `hasMore` (bool, read-only, notify: `hasMoreChanged`) SHALL indicate if more results exist.

#### Scenario: loadMore appends results
- **WHEN** `loadMore()` is called after an initial search with 30 results
- **THEN** the next page SHALL be fetched with offset 30 and appended to the model

### Requirement: Loading state
`SearchViewModel` SHALL expose `isLoading` (bool, read-only, notify: `isLoadingChanged`). It SHALL be `true` while a search is in flight and `false` when it completes or errors.

#### Scenario: Loading state during search
- **WHEN** `search()` is called
- **THEN** `isLoading` SHALL become `true`. When the result arrives, `isLoading` SHALL become `false`

## ADDED Requirements

### Requirement: search returns QmlTask
`Q_INVOKABLE QCoro::QmlTask search()` SHALL dispatch the search query and return a QmlTask that completes when the search finishes. QML may use `.then()` or `.await()` to react to completion.

#### Scenario: QML awaits search completion
- **WHEN** QML calls `searchVm.search().then(callback)`
- **THEN** the callback SHALL fire after the search completes (success or error)

#### Scenario: search still sets properties
- **WHEN** `search()` completes successfully with 20 results
- **THEN** `results->count()` SHALL be 20, `isLoading` SHALL be `false`, and `searchCompleted` SHALL be emitted

### Requirement: loadMore returns QmlTask
`Q_INVOKABLE QCoro::QmlTask loadMore()` SHALL fetch the next page and return a QmlTask that completes when the fetch finishes.

#### Scenario: QML awaits loadMore completion
- **WHEN** QML calls `searchVm.loadMore().then(callback)`
- **THEN** the callback SHALL fire after the page is fetched and appended
