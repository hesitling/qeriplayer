## Context

Phase 4 ViewModels expose async operations as `void` Q_INVOKABLE methods. QML consumers must use `Connections` + signal handlers to detect completion — verbose and error-prone. QCoro's `QmlTask` type (in `lib/qcoro/qcoro/qml/`) allows `Q_INVOKABLE` methods to return `QCoro::Task<T>` directly to QML, enabling `.then()` callbacks and `.await()` property binding.

Current state:
- `PlayerViewModel::play()` already returns `QCoro::Task<void>`
- `SettingsViewModel::loginNetease()` and `logoutNetease()` already return `QCoro::Task<void>`
- All other async VM methods return `void` and emit completion signals
- `QCoro::Qml` module exists in `lib/qcoro/` but is disabled in CMake (`QCORO_WITH_QML OFF`)
- All domain types already have `Q_DECLARE_METATYPE` (required for QmlTask conversion)

## Goals / Non-Goals

**Goals:**
- Enable `QCoro::Qml` in the build system
- Convert void async VM methods to return `QCoro::QmlTask`
- Preserve existing `Q_PROPERTY` pattern for continuous state
- Maintain backward compatibility — completion signals remain for C++ consumers

**Non-Goals:**
- No QML UI implementation (Phase 5)
- No new ViewModels or properties
- No removal of completion signals (they stay for C++ test consumers)
- No `qmlRegisterType` calls (that's Phase 5 context property registration)

## Decisions

### D1: Return QCoro::QmlTask, not QCoro::Task<T>

**Choice:** Methods return `QCoro::QmlTask` (the QML-compatible wrapper), not raw `QCoro::Task<T>`.

**Why:** `QmlTask` is the Q_GADGET that QML can interact with (`.then()`, `.await()`). `QCoro::Task<T>` is a C++ coroutine type that QML doesn't understand. The `QmlTask` constructor auto-converts `Task<T>` → `QmlTask` with `QVariant` wrapping.

**Pattern:**
```cpp
// Before
Q_INVOKABLE void search();

// After
Q_INVOKABLE QCoro::QmlTask search();
```

Implementation stays the same internally — the method is still a coroutine, just now returns `QmlTask` instead of `void`:
```cpp
QCoro::QmlTask SearchViewModel::search() {
    m_isLoading = true;
    emit isLoadingChanged();
    // ... co_await API call ...
    m_isLoading = false;
    emit isLoadingChanged();
    co_return;  // QmlTask wraps Task<void> as QVariant()
}
```

### D2: Keep completion signals alongside QmlTask

**Choice:** Existing signals like `searchCompleted()`, `playlistDeleted()` remain. QML can use either `.then()` OR signals.

**Why:**
- C++ tests use `QSignalSpy` on these signals — removing them breaks tests
- Some QML patterns prefer signals (e.g., `Connections` for side effects)
- `QmlTask` is additive, not replacing

**Tradeoff:** Two ways to do the same thing in QML. Acceptable — `.then()` is preferred for new code, signals remain for backward compat.

### D3: Convert only async methods, not fire-and-forget

**Choice:** Only methods that perform async work (co_await) get converted. Instant methods like `clearResults()`, `setTheme()`, `seek()` stay `void`.

**Why:** `QmlTask` adds overhead (QVariant wrapping, JS callback registration). For instant operations, it's unnecessary. The litmus test: does the method contain `co_await`? If no, keep `void`.

### D4: Error propagation via existing ViewModelError pattern

**Choice:** When a QmlTask-returning method encounters an error, it still sets `hasError`/`error` properties and emits `errorChanged`. The `.then()` callback receives a QVariant (null for void tasks, or the result value).

**Why:** `QmlTask` doesn't have built-in error propagation to QML. The existing property-based error pattern works — QML checks `hasError` after `.then()` fires.

**Pattern in QML:**
```qml
searchVm.search().then(() => {
    if (searchVm.hasError) {
        errorLabel.text = searchVm.error.message
    } else {
        // results populated
    }
})
```

### D5: Build system — enable QCoro::Qml as optional component

**Choice:** Add `QCoro6 COMPONENTS Qml` to `find_package` and link `QCoro::Qml` to the viewmodel library only.

**Why:** The main application target doesn't need QmlTask directly — only the viewmodel library exposes it. Keeping it scoped minimizes link dependencies.

**CMake change:**
```cmake
# In QCoro setup block
set(QCORO_WITH_QML ON CACHE BOOL "Build QCoro Qml support" FORCE)

# In viewmodel library
target_link_libraries(qeriplayer_viewmodel PUBLIC
    Qt6::Core
    ${QERIPLAYER_QCORO_LIBRARIES}
    QCoro6::Qml  # NEW
)
```

## Risks / Trade-offs

**[Risk] QVariant conversion for domain types** → `QmlTask` wraps results in `QVariant`. Domain types (`Song`, `Playlist`, etc.) must be registered with `Q_DECLARE_METATYPE`. Already done per AGENTS.md conventions. Verify with a quick test.

**[Risk] Void tasks produce null QVariant** → `QCoro::Task<void>` → `QmlTask` → `QVariant()`. In QML `.then((result) => ...)`, result is `undefined`. This is fine — void tasks don't return meaningful values. Document this for QML consumers.

**[Risk] Exception handling in QmlTask** → If a coroutine throws, `QmlTask` doesn't propagate the exception to QML. The VM must catch and set `hasError`/`error` properties before co_return. This matches the existing pattern — VMs already catch and map errors.

**[Tradeoff] Two completion mechanisms** → `.then()` and signals both work. Slightly confusing for QML developers. Mitigate by documenting `.then()` as preferred pattern in Phase 5 docs.

**[Tradeoff] QCoro::Qml adds binary size** → ~50KB for the QmlTask Q_GADGET and registration. Negligible for a desktop app.

## Open Questions

None — the QmlTask API is well-defined in `lib/qcoro/qcoro/qml/qcoroqmltask.h` and the doc at `lib/qcoro/docs/reference/qml/qmltask.md`.
