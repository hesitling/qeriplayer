# NeriPlayer Qt Architecture Design Document

## 1. Overview

NeriPlayer Qt is a cross-platform desktop music player client developed using the Qt framework. It uses the Android version of NeriPlayer as a feature and behavior reference while adopting Qt-native layered architecture, module boundaries, and asynchronous patterns for multi-source music platform integration, local playback, playlist management, and other core features.

## 2. Design Principles

### 2.1 Core Principles
- **Local First**: Data is stored locally by default; synchronization is optional
- **Modular**: High cohesion and low coupling; each module has a single responsibility
- **Extensible**: Reserved extension interfaces for adding new music platforms and features
- **Cross-Platform**: Support for Windows, macOS, and Linux desktop systems

### 2.2 Architectural Style
- **MVVM (Model-View-ViewModel)**: Separation of UI and business logic
- **Layered Architecture**: Clear separation of presentation, business, and data layers
- **Dependency Injection**: Application composition root registers long-lived services and injects them into consumers
- **Reactive Programming**: Qt signal-slot mechanism and property binding

## 3. Technology Stack

| Category | Technology | Description |
|----------|-----------|-------------|
| **UI Framework** | Qt 6 Widgets / QML | Native desktop UI |
| **Build System** | CMake 3.16+ | Modern C++ build |
| **C++ Standard** | C++20 | Coroutine support |
| **Multimedia** | Qt Multimedia / mpv | Audio playback |
| **Network** | Qt Network / QCoro | HTTP requests (coroutine-based) |
| **Database** | SQLite (sqlite3 C API) | Local data storage |
| **Serialization** | nlohmann/json | JSON processing |
| **Logging** | spdlog | Logging with daily file rotation |

## 4. Document Index

- [Layered Architecture](layers.md) — Detailed layered architecture design
- [C++20 Coroutines & QCoro](coroutines.md) — Coroutine support and asynchronous programming
- [Porting from Android NeriPlayer](porting-from-android.md) — Rules for using Android as a feature reference

See also: [Module Design Documents](../modules/index.md)

## 5. Overall Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                  Presentation Layer                          │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │  Main Window │  │  Player     │  │  Settings/Dialogs   │  │
│  │             │  │  Controls   │  │                     │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                    Business Layer                            │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │  ViewModel   │  │  Service    │  │  Domain Models      │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                      Data Layer                              │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │  Repository  │  │  API Client │  │  Local Storage      │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                  Infrastructure Layer                        │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │  Network     │  │  Database   │  │  FileSystem         │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

## 6. Module Dependencies

```
┌─────────────────────────────────────────────────────────────┐
│                         UI Module                            │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                      ViewModel Module                        │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                       Service Module                         │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                      Repository Module                       │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                       Core Module                            │
└─────────────────────────────────────────────────────────────┘
```
