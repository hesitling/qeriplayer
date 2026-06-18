## Purpose

QML engine bootstrap, ViewModel context property registration, and ApplicationWindow shell with sidebar navigation.

## ADDED Requirements

### Requirement: QML engine initialization
`QeriPlayerApplication::initializeUi()` SHALL create a `QQmlApplicationEngine`, register all top-level ViewModels as QML context properties, and load `main.qml`.

#### Scenario: Engine loads main.qml
- **WHEN** `initializeUi()` completes
- **THEN** the QML engine SHALL have loaded `qrc:/qml/main.qml` and `rootObjects()` SHALL NOT be empty

#### Scenario: QML load failure is logged
- **WHEN** the QML engine emits warnings during load
- **THEN** each warning SHALL be logged via `Logger::get("app")`

### Requirement: ViewModel context properties
The following context properties SHALL be registered before QML load:

| Property Name | Type | Source |
|---------------|------|--------|
| `mainVm` | `MainViewModel*` | created in `initializeUi()` |
| `playerVm` | `PlayerViewModel*` | `mainVm->playerViewModel()` |
| `searchVm` | `SearchViewModel*` | `mainVm->searchViewModel()` |
| `playlistVm` | `PlaylistViewModel*` | `mainVm->playlistViewModel()` |
| `settingsVm` | `SettingsViewModel*` | `mainVm->settingsViewModel()` |

#### Scenario: QML can access mainVm
- **WHEN** QML reads `mainVm.currentView`
- **THEN** it SHALL resolve to `MainViewModel::View::Home` (initial value)

#### Scenario: QML can access playerVm
- **WHEN** QML reads `playerVm.isPlaying`
- **THEN** it SHALL resolve to `false` (initial value)

### Requirement: ApplicationWindow shell
`main.qml` SHALL define an `ApplicationWindow` with:
- Width: 1000, Height: 700
- Title: "QeriPlayer Qt"
- Material Dark theme
- Layout: `ColumnLayout` containing a `RowLayout` (sidebar + StackView) and a placeholder player bar area

#### Scenario: Window dimensions
- **WHEN** the app starts
- **THEN** the window SHALL be 1000×700 pixels

#### Scenario: Material Dark theme applied
- **WHEN** the app starts
- **THEN** `Material.theme` SHALL be `Material.Dark`

### Requirement: Sidebar navigation
`Sidebar.qml` SHALL display a vertical list of navigation items: Home, Search, Library, Settings. Selecting an item SHALL call `mainVm.navigateTo(view)`.

#### Scenario: Click Search navigates
- **WHEN** user clicks "Search" in the sidebar
- **THEN** `mainVm.navigateTo(MainViewModel::View::Search)` SHALL be called

#### Scenario: Active item highlight
- **WHEN** `mainVm.currentView` is `Search`
- **THEN** the "Search" item SHALL be visually highlighted

### Requirement: StackView content area
`main.qml` SHALL contain a `StackView` that displays placeholder pages based on `mainVm.currentView`.

#### Scenario: Home page shown initially
- **WHEN** the app starts
- **THEN** a placeholder Home page SHALL be visible in the StackView

#### Scenario: Navigation updates StackView
- **WHEN** `mainVm.currentView` changes to `Settings`
- **THEN** the StackView SHALL display the Settings placeholder page

### Requirement: MainWindow removal
The `MainWindow` class (`mainwindow.h`, `mainwindow.cpp`, `mainwindow.ui`) SHALL be removed. `QeriPlayerApplication` SHALL no longer reference `MainWindow`.

#### Scenario: No MainWindow dependency
- **WHEN** the project compiles
- **THEN** no compilation unit SHALL include `mainwindow.h`

### Requirement: VM dependency wiring
`QeriPlayerApplication::initializeUi()` SHALL create all VMs with real dependencies from `ServiceLocator`:

```text
PlayerViewModel(PlaybackController*, PlayHistoryRepository*)
SearchViewModel({NeteaseClient*}, SongRepository*)
PlaylistViewModel(PlaylistRepository*, NeteaseClient*)
SettingsViewModel(SettingsRepository*, NeteaseClient*, PlayHistoryRepository*)
MainViewModel(PlayerVM*, SearchVM*, PlaylistVM*, SettingsVM*,
              SongRepository*, PlaylistRepository*, NeteaseClient*)
```

#### Scenario: VMs receive non-null dependencies
- **WHEN** `initializeUi()` completes
- **THEN** all VM pointers passed to MainViewModel SHALL be non-null (given a properly initialized ServiceLocator)
