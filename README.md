# QeriPlayer Qt Desktop Client

A Qt 6 desktop client for QeriPlayer, providing a native cross-platform music playback experience.

## Features

- Native Qt Widgets-based user interface
- Cross-platform support for Windows, macOS, and Linux
- C++20 codebase with QCoro for asynchronous Qt workflows
- Application bootstrap module with service registration
- Core network scaffolding for HTTP, WebSocket, and network-state services
- Menu bar, toolbar, and status bar placeholders for early development

## Building

### Prerequisites

- Qt 6.5 or later with Core, Gui, Widgets, Network, and WebSockets components
- QCoro 0.10 or later with Core, Network, and WebSockets components
- CMake 3.16 or later
- C++20-compatible compiler

### Build Instructions

```bash
# Configure with local lib/qcoro when present, otherwise use installed QCoro
cmake -S . -B build

# Build the project
cmake --build build

# Run the application
./build/QeriPlayerQt
```

### QCoro Source Selection

By default, CMake uses a local `lib/qcoro` checkout when it exists and falls back to an installed `QCoro6` package otherwise.

```bash
# Force the installed QCoro package
cmake -S . -B build -DQERIPLAYER_USE_SYSTEM_QCORO=ON

# Use a custom QCoro source checkout
cmake -S . -B build -DQERIPLAYER_QCORO_SOURCE_DIR=/path/to/qcoro
```

## Project Structure

```
qeriplayer-qt/
├── CMakeLists.txt          # CMake build configuration
├── README.md               # Project overview and build instructions
├── docs/                   # Architecture and module design documents
└── src/
    ├── app/                # Application bootstrap and service registration
    ├── core/network/       # HTTP, WebSocket, and network status services
    ├── main.cpp            # Application entry point
    ├── mainwindow.h        # Main window header
    ├── mainwindow.cpp      # Main window implementation
    └── mainwindow.ui       # Qt Designer UI file
```

## Architecture Status

The project is in early implementation. The current code establishes the application shell and first core services. The architecture documents describe the intended layered design, with Android QeriPlayer used as a feature reference rather than a direct dependency-shape template.

Near-term implementation areas:

- Domain models shared by API, repositories, player, and UI
- Platform API clients for NetEase, Bilibili, YouTube Music, and QQ Music
- Repository layer for songs, playlists, settings, history, and player state
- PlaybackController for playback orchestration (queue, URL resolution, persistence)
- ViewModels that expose Qt properties/signals without depending on UI widgets
- Player integration and persistent local storage

## License

This project is part of the QeriPlayer ecosystem. See the main QeriPlayer repository for license information.
