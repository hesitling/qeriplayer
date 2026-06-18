## Context

The QeriPlayer Qt application has completed Phase 4 (ViewModels) and Phase 5 (QML UI) implementation. All ViewModels have been converted to use `QCoro::QmlTask` for QML-friendly async operations, except `SettingsViewModel` which still uses `QCoro::Task<void>` for `loginNetease()` and `logoutNetease()`.

The QML infrastructure is in place with `main.qml`, `Sidebar.qml`, and placeholder pages. However, the Sidebar has a vertical alignment issue where the "Settings" item appears lower than other navigation items.

## Goals / Non-Goals

**Goals:**
- Achieve consistent `QCoro::QmlTask` usage across all ViewModels
- Fix Sidebar vertical alignment for consistent item positioning
- Verify complete QML UI integration works correctly
- Prepare for Phase 2-6 implementation (PlayerBar, SearchView, etc.)

**Non-Goals:**
- Implementing full SettingsView UI (PR 5)
- Adding animations or theme system (Phase 7)
- Modifying ViewModel business logic or error handling patterns

## Decisions

### Decision 1: Convert SettingsViewModel to QmlTask

**Choice:** Convert `loginNetease()` and `logoutNetease()` to return `QCoro::QmlTask`

**Rationale:**
- Consistency with other ViewModels (SearchViewModel, PlaylistViewModel, etc.)
- Enables clean `.then()` chaining in QML: `settingsVm.loginNetease(phone, pass).then(callback)`
- Aligns with Phase 5 PR 0 plan which specified QmlTask conversion

**Alternatives considered:**
- Keep `QCoro::Task<void>` with signal-based result handling
  - Rejected: More complex QML code, inconsistent with other VMs
- Add separate Q_INVOKABLE wrappers that return QmlTask
  - Rejected: Adds unnecessary API surface

### Decision 2: Sidebar Vertical Alignment Fix

**Choice:** Use `anchors.verticalCenter: parent.verticalCenter` on navigation items and expand Sidebar vertically

**Rationale:**
- Current implementation uses `anchors.fill: parent` with fixed margins
- Items are top-aligned by default, causing Settings to appear lower
- Vertical centering ensures consistent visual weight across all items

**Implementation approach:**
- Add `anchors.verticalCenter: parent.verticalCenter` to the RowLayout in ItemDelegate
- This anchors the contentItem directly to the parent ItemDelegate's vertical center
- Consider expanding Sidebar height to fill available space better

**Alternatives considered:**
- Adjust individual item heights
  - Rejected: Would require per-item styling, harder to maintain
- Use ColumnLayout instead of ListView
  - Rejected: ListView provides better scrolling and selection highlighting

### Decision 3: Test Strategy

**Choice:** Update existing tests to work with QmlTask return types

**Rationale:**
- Tests already use `QCoro::waitFor()` for async operations
- Minimal test changes required - mainly updating method call patterns
- Maintains test coverage without restructuring

## Risks / Trade-offs

**[Risk] QmlTask conversion may break existing QML bindings**
- Mitigation: QmlTask is backward-compatible with Q_INVOKABLE; QML can still call methods without .then()

**[Risk] Sidebar alignment fix may affect other navigation items**
- Mitigation: Test all navigation items after change; ensure consistent appearance

**[Risk] Test updates may introduce subtle timing issues**
- Mitigation: Use existing `QCoro::waitFor()` patterns; run full test suite

## Migration Plan

1. **Phase 1: SettingsViewModel conversion**
   - Update method signatures in header
   - Implement with co_await/co_return pattern
   - Update tests
   - Verify build and tests pass

2. **Phase 2: Sidebar UI fix**
   - Modify Sidebar.qml vertical alignment
   - Test visual appearance
   - Verify navigation still works

3. **Phase 3: Integration verification**
   - Launch app and verify all components
   - Document any remaining issues for Phase 7

## Open Questions

None — this is a straightforward conversion with clear implementation path.
