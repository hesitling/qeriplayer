## Purpose

Defines the `QmlTask` return type pattern for ViewModel Q_INVOKABLE methods, enabling QML consumers to use `.then()` callbacks and `.await()` property binding for async operations.

## Requirements

### Requirement: Async VM methods return QCoro::QmlTask
ViewModel methods that perform async work (containing `co_await`) SHALL return `QCoro::QmlTask` instead of `void`. The `QmlTask` type is a Q_GADGET that QML can interact with via `.then()` and `.await()`.

#### Scenario: QML uses .then() on async method
- **WHEN** QML calls `viewModel.asyncMethod().then(callback)`
- **THEN** the callback SHALL execute after the coroutine completes

#### Scenario: QML uses .await() for property binding
- **WHEN** QML uses `viewModel.asyncMethod().await().value` in a binding
- **THEN** the value SHALL be initially null and update to the result when the coroutine completes

#### Scenario: Void coroutine produces null QVariant
- **WHEN** a method returns `QCoro::QmlTask` from a `QCoro::Task<void>` coroutine
- **THEN** the `.then()` callback SHALL receive `undefined` as the result argument

### Requirement: Fire-and-forget methods stay void
ViewModel methods that perform no async work (no `co_await`) SHALL remain `void`. This includes property setters, clear/reset methods, and instant operations.

#### Scenario: Instant method remains void
- **WHEN** `clearResults()` is called (no async work)
- **THEN** it SHALL return `void` and not be callable with `.then()`

### Requirement: Error state via properties, not exceptions
When a QmlTask-returning method encounters an error, it SHALL set `hasError`/`error` properties and emit `errorChanged` before returning. The QmlTask SHALL NOT propagate C++ exceptions to QML.

#### Scenario: Error set before co_return
- **WHEN** an API call fails inside `search()` with a network error
- **THEN** `hasError` SHALL be `true`, `error` SHALL contain the mapped `ViewModelError`, and the `.then()` callback SHALL still fire

### Requirement: Existing completion signals preserved
Methods converted to return QmlTask SHALL retain their existing completion signals (e.g., `searchCompleted()`, `playlistDeleted()`). Both `.then()` and signal-based patterns SHALL work.

#### Scenario: Signal still emitted
- **WHEN** `search()` completes successfully
- **THEN** `searchCompleted()` SHALL be emitted AND `.then()` callbacks SHALL fire

### Requirement: Domain types convertible to QVariant
All domain types (`Song`, `Playlist`, `Album`, `Artist`, `SearchResult`, etc.) used as QmlTask return values SHALL be registered with `Q_DECLARE_METATYPE` to enable `QVariant` conversion.

#### Scenario: Song type convertible
- **WHEN** a method returns `QCoro::Task<Song>` wrapped in QmlTask
- **THEN** the `.then()` callback SHALL receive a QVariant containing the Song
