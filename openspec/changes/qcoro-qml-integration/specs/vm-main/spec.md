## MODIFIED Requirements

### Requirement: App initialization
`Q_INVOKABLE QCoro::QmlTask initialize()` SHALL:
1. Call `SettingsViewModel::loadSettings()`
2. Call `PlaybackController::restoreState()` (via PlayerViewModel)
3. Call `PlaylistViewModel::loadLocalPlaylists()`

#### Scenario: initialize restores state
- **WHEN** `initialize()` is called
- **THEN** settings SHALL be loaded, playback state SHALL be restored, and local playlists SHALL be loaded

#### Scenario: QML awaits initialization
- **WHEN** QML calls `mainVm.initialize().then(() => showMainWindow())`
- **THEN** the main window SHALL appear after all initialization steps complete
