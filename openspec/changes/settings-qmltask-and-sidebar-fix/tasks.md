## 1. SettingsViewModel QmlTask Conversion

- [x] 1.1 Update `SettingsViewModel.h` method signatures: change `loginNetease()` and `logoutNetease()` from `QCoro::Task<void>` to `QCoro::QmlTask`
- [x] 1.2 Update `SettingsViewModel.cpp` implementations to return `QCoro::QmlTask` using `co_return` pattern
- [x] 1.3 Update `tests/viewmodel/TestSettingsViewModel.cpp` to work with new return types (use `QCoro::waitFor()` pattern)

## 2. Sidebar Vertical Alignment Fix

- [ ] 2.1 Update `src/qml/Sidebar.qml`: add vertical centering to navigation items using `Layout.alignment: Qt.AlignVCenter` or `anchors.verticalCenter: parent.verticalCenter`
- [ ] 2.2 Update `src/qml/Sidebar.qml`: expand sidebar to fill available vertical space with proper distribution
- [ ] 2.3 Verify all navigation items (Home, Search, Library, Settings) have consistent vertical positioning

## 3. Build and Test Verification

- [ ] 3.1 Run `just build_dir=/tmp/qeriplayer-build build` to verify compilation succeeds
- [ ] 3.2 Run `just build_dir=/tmp/qeriplayer-build test` to verify all tests pass
- [ ] 3.3 Run `just format` to format changed files
- [ ] 3.4 Launch app manually and verify: window appears, sidebar visible, navigation switches placeholder pages, Settings item aligned correctly
