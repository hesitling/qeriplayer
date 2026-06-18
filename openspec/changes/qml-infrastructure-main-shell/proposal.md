## Why

The ViewModels (Phase 4) are QML-ready with Q_PROPERTY/Q_INVOKABLE and QmlTask returns, but there's no QML engine to load them. The app still uses a placeholder QWidget MainWindow. This change bootstraps the QML infrastructure — engine setup, VM context property registration, and a navigation shell — so subsequent PRs can build real views on top.

## What Changes

- **Enable Qt Quick in CMake** — add `Qt6::Quick`, `Qt6::Qml`, `Qt6::QuickControls2` to the main target's `find_package` and `target_link_libraries`.
- **Create QML resource infrastructure** — new `src/qml/` directory with `qml.qrc` resource file.
- **Create main QML shell** — `main.qml` with `ApplicationWindow`, sidebar navigation, `StackView` content area, and placeholder player bar. Material Dark theme.
- **Create Sidebar component** — `Sidebar.qml` with Home/Search/Library/Settings navigation items.
- **Wire ViewModels to QML** — rewrite `QeriPlayerApplication::initializeUi()` to create all VMs, inject dependencies from `ServiceLocator`, and register them as QML context properties.
- **Fix MainViewModel constructor** — add `ISongRepository*`, `IPlaylistRepository*`, `NeteaseClient*` params so detail VMs get real repos instead of `nullptr`.
- **Remove MainWindow** — delete `MainWindow` class and `.ui` file (replaced by QML engine).

## Non-goals

- **No real view content** — SearchView, PlaylistView, SettingsView, PlayerBar are placeholder rectangles. Real implementations are PRs 2-6.
- **No new ViewModels** — only modifying MainViewModel constructor.
- **No dialog or theme system** — Phase 7.
- **No keyboard shortcuts or system tray** — Phase 7.

## Capabilities

### New Capabilities

- `ui-qml-shell`: QML engine bootstrap, VM context property registration, ApplicationWindow with sidebar + StackView navigation.

### Modified Capabilities

- `vm-main`: MainViewModel constructor gains `ISongRepository*`, `IPlaylistRepository*`, `NeteaseClient*` params for wiring detail VMs.

## Impact

- **Affected layers**: `app/` (engine setup), `viewmodel/` (MainViewModel constructor), `CMakeLists.txt` (Qt Quick deps), new `qml/` directory
- **Dependencies**: `Qt6::Quick`, `Qt6::Qml`, `Qt6::QuickControls2` (all available on system)
- **Files deleted**: `mainwindow.h`, `mainwindow.cpp`, `mainwindow.ui`
- **Files modified**: `CMakeLists.txt`, `QeriPlayerApplication.h/cpp`, `MainViewModel.h/cpp`, `tests/viewmodel/TestMainViewModel.cpp`
- **Files created**: `src/qml/qml.qrc`, `src/qml/main.qml`, `src/qml/Sidebar.qml`
