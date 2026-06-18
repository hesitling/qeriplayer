## 1. CMake — Add Qt Quick Dependencies

- [x] 1.1 Add `Qt6::Quick`, `Qt6::Qml`, `Qt6::QuickControls2` to `find_package(Qt6 ...)` in root `CMakeLists.txt`
- [x] 1.2 Add `Qt6::Quick`, `Qt6::Qml`, `Qt6::QuickControls2` to `target_link_libraries` for the main `QeriPlayerQt` target
- [x] 1.3 Verify build compiles with new Qt deps (no new code yet, just linkage)

## 2. QML Files — Shell and Sidebar

- [x] 2.1 Create `src/qml/qml.qrc` resource file listing QML files
- [x] 2.2 Create `src/qml/main.qml` — ApplicationWindow (1000×700, Material Dark, ColumnLayout with RowLayout(Sidebar + StackView) + placeholder PlayerBar area)
- [x] 2.3 Create `src/qml/Sidebar.qml` — ListView with Home/Search/Library/Settings items, emits `navigateTo(view)` via mainVm

## 3. MainViewModel — Expand Constructor

- [x] 3.1 Add `ISongRepository*`, `IPlaylistRepository*`, `NeteaseClient*` members and constructor params to `MainViewModel.h`
- [x] 3.2 Update `MainViewModel.cpp` — store new params, fix `openLocalPlaylist()` to pass real repos to `LocalPlaylistDetailViewModel`, fix `openNeteasePlaylist()` and `openNeteaseAlbum()` to pass real repos to `NeteasePlaylistDetailViewModel`
- [x] 3.3 Update `tests/viewmodel/TestMainViewModel.cpp` — adjust constructor calls to pass 7 args (nullptr for new params)

## 4. QeriPlayerApplication — Wire QML Engine

- [x] 4.1 Add `#include <QQmlApplicationEngine>` and `#include <QQmlContext>` to `QeriPlayerApplication.cpp`
- [x] 4.2 Rewrite `initializeUi()` to: create VMs from ServiceLocator deps, register as context properties, create QQmlApplicationEngine, load `qrc:/qml/main.qml`, connect engine warnings to Logger
- [x] 4.3 Replace `m_mainWindow` member with `std::unique_ptr<QQmlApplicationEngine>` in `QeriPlayerApplication.h`
- [x] 4.4 Remove `showMainWindow()` and `mainWindow()` methods (or make them no-ops)

## 5. Remove MainWindow

- [x] 5.1 Delete `src/mainwindow.h`, `src/mainwindow.cpp`, `src/mainwindow.ui`
- [x] 5.2 Remove `mainwindow.h/cpp/ui` from `SOURCES`/`HEADERS`/`UI_FILES` in `CMakeLists.txt`
- [x] 5.3 Remove `#include "mainwindow.h"` from `QeriPlayerApplication.cpp`

## 6. Build, Test, Format

- [x] 6.1 Run `just build_dir=/tmp/qeriplayer-build build` — verify compilation succeeds
- [x] 6.2 Run `just build_dir=/tmp/qeriplayer-build test` — verify all existing tests pass
- [x] 6.3 Run `just format` — format changed files
- [ ] 6.4 Launch app manually and verify: window appears, sidebar visible, navigation switches placeholder pages
