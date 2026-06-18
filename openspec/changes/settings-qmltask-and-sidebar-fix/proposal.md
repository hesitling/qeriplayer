## Why

The SettingsViewModel's `loginNetease()` and `logoutNetease()` methods currently return `QCoro::Task<void>` instead of `QCoro::QmlTask`, creating an inconsistency with other ViewModels that have already been converted. This prevents clean `.then()` chaining in QML and requires workarounds for async error handling.

Additionally, the Sidebar navigation has a UI positioning issue where the "Settings" text appears slightly lower than other items, requiring vertical alignment adjustment.

## What Changes

- Convert `SettingsViewModel::loginNetease()` and `SettingsViewModel::logoutNetease()` from `QCoro::Task<void>` to `QCoro::QmlTask`
- Update related tests to work with new return types
- Fix Sidebar vertical alignment to ensure consistent item positioning
- Expand Sidebar layout to properly distribute space across all navigation items
- Verify complete QML UI integration (window, sidebar, navigation, placeholder pages)

## Capabilities

### New Capabilities

_None — this change modifies existing capabilities._

### Modified Capabilities

- `vm-settings`: Requirements changing for `loginNetease()` and `logoutNetease()` return types (QCoro::QmlTask instead of QCoro::Task<void>)
- `ui-qml-shell`: Requirements changing for Sidebar vertical alignment and item positioning

## Impact

- **Affected layers**: `ui` (QML), `viewmodel` (SettingsViewModel)
- **Affected files**:
  - `src/viewmodel/SettingsViewModel.h` — method signatures
  - `src/viewmodel/SettingsViewModel.cpp` — implementations
  - `src/qml/Sidebar.qml` — vertical alignment fix
  - `tests/viewmodel/TestSettingsViewModel.cpp` — test updates
- **Dependencies**: QCoro QML module (already enabled in CMake)
- **Breaking changes**: None — QML binding interface remains compatible
