# NeriPlayer Qt Desktop Client

A Qt-based desktop client for NeriPlayer, providing a native desktop experience for music playback.

## Features

- Native Qt Widgets-based user interface
- Cross-platform support (Windows, macOS, Linux)
- Basic media player controls (Play, Pause, Stop)
- Menu bar with standard application options
- Status bar for feedback and messages
- Modern C++17 codebase

## Building

### Prerequisites

- Qt 5.15 or later
- CMake 3.16 or later
- C++17 compatible compiler

### Build Instructions

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build .

# Run the application
./NeriPlayerQt
```

### Alternative: Using qmake

If you prefer qmake, you can create a .pro file:

```qmake
QT += core gui widgets

TARGET = NeriPlayerQt
TEMPLATE = app

SOURCES += src/main.cpp \
           src/mainwindow.cpp

HEADERS += src/mainwindow.h

FORMS += src/mainwindow.ui
```

Then build with:

```bash
qmake
make
```

## Project Structure

```
neriplayer-qt/
├── CMakeLists.txt          # CMake build configuration
├── README.md               # This file
├── .gitignore              # Git ignore rules
└── src/
    ├── main.cpp            # Application entry point
    ├── mainwindow.h        # Main window header
    ├── mainwindow.cpp      # Main window implementation
    └── mainwindow.ui       # Qt Designer UI file
```

## Development Status

This is an initial project setup with basic window functionality. Future development will include:

- Media library management
- Audio playback engine integration
- Playlist management
- Audio visualization
- Settings and preferences
- Theme support
- Keyboard shortcuts

## License

This project is part of the NeriPlayer ecosystem. See the main NeriPlayer repository for license information.