cmake_minimum_required(VERSION 3.16)

project(NeriPlayerQt VERSION 0.1.0 LANGUAGES CXX)

# C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

# Qt 6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network WebSockets)

# QCoro
set(NERIPLAYER_QCORO_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/qcoro" CACHE PATH
    "Path to a local QCoro source checkout")
option(NERIPLAYER_USE_SYSTEM_QCORO "Always use an installed QCoro package" OFF)

if(NOT NERIPLAYER_USE_SYSTEM_QCORO AND EXISTS "${NERIPLAYER_QCORO_SOURCE_DIR}/CMakeLists.txt")
    set(QCORO_BUILD_EXAMPLES OFF CACHE BOOL "Build QCoro examples" FORCE)
    set(QCORO_BUILD_TESTING OFF CACHE BOOL "Build QCoro tests" FORCE)
    set(QCORO_WITH_QTDBUS OFF CACHE BOOL "Build QCoro QtDBus support" FORCE)
    set(QCORO_WITH_QTNETWORK ON CACHE BOOL "Build QCoro QtNetwork support" FORCE)
    set(QCORO_WITH_QTWEBSOCKETS ON CACHE BOOL "Build QCoro QtWebSockets support" FORCE)
    set(QCORO_WITH_QTQUICK OFF CACHE BOOL "Build QCoro QtQuick support" FORCE)
    set(QCORO_WITH_QML OFF CACHE BOOL "Build QCoro QML support" FORCE)
    set(QCORO_WITH_QTTEST OFF CACHE BOOL "Build QCoro QtTest support" FORCE)

    add_subdirectory("${NERIPLAYER_QCORO_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/_deps/qcoro")
    set(NERIPLAYER_QCORO_LIBRARIES QCoro6::Core QCoro6::Network QCoro6::WebSockets)
else()
    find_package(QCoro6 REQUIRED COMPONENTS Core Network WebSockets)
    qcoro_enable_coroutines()
    set(NERIPLAYER_QCORO_LIBRARIES QCoro::Core QCoro::Network QCoro::WebSockets)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 源文件
set(SOURCES
    src/main.cpp
    src/app/NeriPlayerApplication.cpp
    src/core/database/DatabaseManager.cpp
    src/core/network/HttpClient.cpp
    src/core/network/WebSocketClient.cpp
    src/core/network/NetworkManager.cpp
    src/core/network/NetworkMonitor.cpp
    src/mainwindow.cpp
)

# 头文件
set(HEADERS
    src/app/NeriPlayerApplication.h
    src/app/ServiceLocator.h
    src/core/database/DatabaseManager.h
    src/core/network/HttpClient.h
    src/core/network/WebSocketClient.h
    src/core/network/NetworkManager.h
    src/core/network/NetworkMonitor.h
    src/domain/Domain.h
    src/domain/Enums.h
    src/domain/Song.h
    src/domain/Album.h
    src/domain/Artist.h
    src/domain/Playlist.h
    src/domain/Lyrics.h
    src/domain/SearchResult.h
    src/mainwindow.h
)

# UI 文件
set(UI_FILES
    src/mainwindow.ui
)

add_executable(NeriPlayerQt ${SOURCES} ${HEADERS} ${UI_FILES})

target_link_libraries(NeriPlayerQt PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
    ${NERIPLAYER_QCORO_LIBRARIES}
    ${SQLITE3_LIBRARIES}
)

# 包含目录
target_include_directories(NeriPlayerQt PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
)

# Install target
install(TARGETS NeriPlayerQt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# SQLite
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(SQLITE3 sqlite3)
endif()
if(NOT SQLITE3_FOUND)
    find_library(SQLITE3_LIBRARIES NAMES sqlite3)
    find_path(SQLITE3_INCLUDE_DIRS sqlite3.h)
    if(NOT SQLITE3_LIBRARIES OR NOT SQLITE3_INCLUDE_DIRS)
        message(FATAL_ERROR "sqlite3 not found. Install libsqlite3-dev or equivalent.")
    endif()
endif()

# Testing
enable_testing()
find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(TestDomainModels
    tests/domain/TestDomainModels.cpp
)
target_include_directories(TestDomainModels PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(TestDomainModels PRIVATE
    Qt6::Core
    Qt6::Test
)
target_compile_definitions(TestDomainModels PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestDomainModels COMMAND TestDomainModels)

add_executable(TestDatabase
    tests/core/TestDatabase.cpp
    src/core/database/DatabaseManager.cpp
)
target_include_directories(TestDatabase PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
)
target_link_libraries(TestDatabase PRIVATE
    Qt6::Core
    Qt6::Test
    ${SQLITE3_LIBRARIES}
    dl
    pthread
)
target_compile_definitions(TestDatabase PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestDatabase COMMAND TestDatabase)
