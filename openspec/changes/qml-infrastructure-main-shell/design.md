## Context

Phase 4 ViewModels are complete — all async methods return `QCoro::QmlTask`, all state is exposed via `Q_PROPERTY`. The app currently runs a placeholder `MainWindow` (QWidgets) created in `QeriPlayerApplication::initializeUi()`. Qt6Quick, Qt6QuickControls2, and Qt6QuickControls2Material are available on the system. `QCoro6::Qml` is already linked in CMake.

The goal is to replace the widget shell with a QML engine, register VMs as context properties, and provide a navigation skeleton that future PRs can fill with real views.

## Goals / Non-Goals

**Goals:**
- App launches with QML `ApplicationWindow` (Material Dark theme)
- Sidebar navigation switches between placeholder pages via `StackView`
- All top-level VMs accessible from QML as context properties
- `MainViewModel` can create detail VMs with real repo dependencies
- All existing tests pass

**Non-Goals:**
- Real view content (SearchView, PlaylistView, etc.)
- PlayerBar with real bindings
- Dialogs, theme switching, keyboard shortcuts

## Decisions

### 1. Full QML engine vs QQuickWidget

**Choice:** `QQmlApplicationEngine` (full QML)

**Rationale:** QQuickWidget adds complexity — it embeds QML inside a QWidget hierarchy, requiring careful lifecycle management and mixed event loops. Full QML is simpler, the standard approach for new Qt6 apps, and avoids the "widget soup" anti-pattern.

**Alternative:** `QQuickWidget` inside `MainWindow` — would let us keep the menu bar and status bar as widgets. Rejected because we'll build those in QML later anyway.

### 2. VM ownership and lifetime

**Choice:** VMs are owned by `QeriPlayerApplication` (via `unique_ptr`), created in `initializeUi()`, and exposed as raw pointers to QML via `setContextProperty()`.

**Rationale:** QML engine doesn't take ownership of context properties. The application object is the natural owner — it outlives the QML engine. Using `unique_ptr` ensures cleanup on shutdown.

### 3. MainViewModel constructor expansion

**Choice:** Add `ISongRepository*`, `IPlaylistRepository*`, `NeteaseClient*` to `MainViewModel` constructor.

**Rationale:** Detail VMs (`LocalPlaylistDetailViewModel`, `NeteasePlaylistDetailViewModel`) need repos at construction time. Constructor injection keeps `MainViewModel` testable (tests can pass nullptr or mocks). The alternative — passing `ServiceLocator*` — would couple MainViewModel to the service locator pattern.

### 4. QML file organization

**Choice:** Flat `src/qml/` directory with `qml.qrc` resource file.

**Rationale:** For the initial shell (3-4 files), subdirectories add complexity without benefit. When we reach 10+ files in later PRs, we can reorganize into `src/qml/views/`, `src/qml/components/`, `src/qml/dialogs/`.

### 5. Material Dark as default theme

**Choice:** Set `Material.theme: Material.Dark` in `main.qml`.

**Rationale:** Music players look better dark. Material Dark gives a modern look out of the box with zero custom styling. Theme switching (light/dark/system) is deferred to Phase 7.

## Risks / Trade-offs

**[Risk] QML loading failures are silent** → Mitigation: connect to `QQmlApplicationEngine::warnings()` signal and log via `Logger`. Also check `engine->rootObjects().isEmpty()` after load.

**[Risk] Context property names are stringly-typed** → Mitigation: use consistent naming (`mainVm`, `playerVm`, etc.) and document in a comment in `initializeUi()`. Future improvement: use `qmlRegisterSingletonType` instead.

**[Risk] Breaking existing tests** → Mitigation: MainViewModel test creates VM with 7 params now (3 new nullable). Tests pass `nullptr` for new params since they don't test detail VM creation.

## Open Questions

None — all dependencies are available and constructor signatures are verified.
