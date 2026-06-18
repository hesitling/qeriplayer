cmake_minimum_required(VERSION 3.16)

project(QeriPlayerQt VERSION 0.1.0 LANGUAGES CXX)

# C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

# ─── Dependencies ─────────────────────────────────────────────────

# Qt 6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network WebSockets Multimedia)

# QCoro
set(QERIPLAYER_QCORO_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/qcoro" CACHE PATH
    "Path to a local QCoro source checkout")
option(QERIPLAYER_USE_SYSTEM_QCORO "Always use an installed QCoro package" OFF)

if(NOT QERIPLAYER_USE_SYSTEM_QCORO AND EXISTS "${QERIPLAYER_QCORO_SOURCE_DIR}/CMakeLists.txt")
    set(QCORO_BUILD_EXAMPLES OFF CACHE BOOL "Build QCoro examples" FORCE)
    set(QCORO_BUILD_TESTING OFF CACHE BOOL "Build QCoro tests" FORCE)
    set(QCORO_WITH_QTDBUS OFF CACHE BOOL "Build QCoro QtDBus support" FORCE)
    set(QCORO_WITH_QTNETWORK ON CACHE BOOL "Build QCoro QtNetwork support" FORCE)
    set(QCORO_WITH_QTWEBSOCKETS ON CACHE BOOL "Build QCoro QtWebSockets support" FORCE)
    set(QCORO_WITH_QTQUICK OFF CACHE BOOL "Build QCoro QtQuick support" FORCE)
    set(QCORO_WITH_QML OFF CACHE BOOL "Build QCoro QML support" FORCE)
    set(QCORO_WITH_QTTEST OFF CACHE BOOL "Build QCoro QtTest support" FORCE)

    add_subdirectory("${QERIPLAYER_QCORO_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/_deps/qcoro")
    set(QERIPLAYER_QCORO_LIBRARIES QCoro6::Core QCoro6::Network QCoro6::WebSockets)
else()
    find_package(QCoro6 REQUIRED COMPONENTS Core Network WebSockets)
    qcoro_enable_coroutines()
    set(QERIPLAYER_QCORO_LIBRARIES QCoro::Core QCoro::Network QCoro::WebSockets)
endif()

# SQLite
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(SQLITE3 QUIET sqlite3)
endif()
if(NOT SQLITE3_FOUND)
    find_library(SQLITE3_LIBRARIES NAMES sqlite3)
    find_path(SQLITE3_INCLUDE_DIRS sqlite3.h)
    if(NOT SQLITE3_LIBRARIES OR NOT SQLITE3_INCLUDE_DIRS)
        message(FATAL_ERROR "sqlite3 not found. Install libsqlite3-dev or equivalent.")
    endif()
endif()

# spdlog
if(PkgConfig_FOUND)
    pkg_check_modules(SPDLOG QUIET spdlog)
endif()
if(NOT SPDLOG_FOUND)
    find_path(SPDLOG_INCLUDE_DIRS spdlog/spdlog.h)
    find_library(SPDLOG_LIBRARIES NAMES spdlog)
    if(NOT SPDLOG_INCLUDE_DIRS OR NOT SPDLOG_LIBRARIES)
        message(FATAL_ERROR "spdlog not found. Install libspdlog-dev or equivalent.")
    endif()
endif()

# OpenSSL
find_package(OpenSSL REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ─── Main Application ─────────────────────────────────────────────

set(SOURCES
    src/main.cpp
    src/app/QeriPlayerApplication.cpp
    src/core/crypto/CryptoUtils.cpp
    src/core/crypto/Encryptor.cpp
    src/core/crypto/Decryptor.cpp
    src/core/crypto/SecureStorage.cpp
    src/core/database/DatabaseManager.cpp
    src/core/filesystem/AppPaths.cpp
    src/core/filesystem/FileUtils.cpp
    src/core/filesystem/FileWatcher.cpp
    src/core/logger/Logger.cpp
    src/core/network/HttpClient.cpp
    src/core/network/WebSocketClient.cpp
    src/core/network/NetworkManager.cpp
    src/core/network/NetworkMonitor.cpp
    src/repo/SqlRowMapper.cpp
    src/repo/SongRepository.cpp
    src/repo/SettingsRepository.cpp
    src/repo/PlayHistoryRepository.cpp
    src/repo/PlaylistRepository.cpp
    src/repo/PlayerStateRepository.cpp
    src/mainwindow.cpp
    src/api/common/ApiError.cpp
    src/api/netease/NeteaseCrypto.cpp
    src/api/netease/NeteaseParser.cpp
    src/api/netease/NeteaseClientCore.cpp
    src/api/netease/NeteaseClientAuth.cpp
    src/api/netease/NeteaseClientContent.cpp
    src/api/netease/NeteaseClientUser.cpp
    src/player/backends/QtMultimediaBackend.cpp
    src/player/PlayQueue.cpp
    src/player/PlaybackController.cpp
    src/player/BackendFactory.cpp
)

set(HEADERS
    src/app/QeriPlayerApplication.h
    src/app/ServiceLocator.h
    src/core/crypto/CryptoUtils.h
    src/core/crypto/Encryptor.h
    src/core/crypto/Decryptor.h
    src/core/crypto/SecureStorage.h
    src/core/database/DatabaseManager.h
    src/core/filesystem/AppPaths.h
    src/core/filesystem/FileUtils.h
    src/core/filesystem/FileWatcher.h
    src/core/logger/Logger.h
    src/core/network/HttpClient.h
    src/core/network/WebSocketClient.h
    src/core/network/NetworkManager.h
    src/core/network/NetworkMonitor.h
    src/repo/SqlRowMapper.h
    src/repo/SongRepository.h
    src/repo/SettingsRepository.h
    src/repo/PlayHistoryRepository.h
    src/repo/PlaylistRepository.h
    src/repo/PlayerStateRepository.h
    src/repo/ISongRepository.h
    src/repo/IPlaylistRepository.h
    src/repo/IPlayHistoryRepository.h
    src/repo/IPlayerStateRepository.h
    src/repo/ISettingsRepository.h
    src/domain/Domain.h
    src/domain/Enums.h
    src/domain/Song.h
    src/domain/Album.h
    src/domain/Artist.h
    src/domain/Playlist.h
    src/domain/Lyrics.h
    src/domain/PersistedPlayerState.h
    src/domain/PlaylistSummary.h
    src/domain/SearchResult.h
    src/domain/SongIdentity.h
    src/domain/SongUrlResult.h
    src/api/common/ApiError.h
    src/api/common/ApiResult.h
    src/api/common/VoidResult.h
    src/api/common/LoginResult.h
    src/api/common/QrCodeData.h
    src/api/common/PlayHistory.h
    src/api/common/IMusicPlatformPlugin.h
    src/api/netease/NeteaseCrypto.h
    src/api/netease/NeteaseParser.h
    src/api/netease/NeteaseClient.h
    src/player/IPlayerBackend.h
    src/player/backends/QtMultimediaBackend.h
    src/player/PlayQueue.h
    src/player/PlaybackController.h
    src/player/BackendFactory.h
    src/mainwindow.h
)

set(UI_FILES
    src/mainwindow.ui
)

add_executable(QeriPlayerQt ${SOURCES} ${HEADERS} ${UI_FILES})

target_link_libraries(QeriPlayerQt PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
    Qt6::Multimedia
    ${QERIPLAYER_QCORO_LIBRARIES}
    ${SQLITE3_LIBRARIES}
    ${SPDLOG_LIBRARIES}
    OpenSSL::Crypto
)

target_include_directories(QeriPlayerQt PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)

install(TARGETS QeriPlayerQt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ─── ViewModel Library ────────────────────────────────────────────

set(VIEWMODEL_SOURCES
    src/viewmodel/ViewModelError.cpp
    src/viewmodel/SongListModel.cpp
    src/viewmodel/PlayerViewModel.cpp
    src/viewmodel/SettingsViewModel.cpp
    src/viewmodel/SearchViewModel.cpp
    src/viewmodel/PlaylistViewModel.cpp
    src/viewmodel/LocalPlaylistDetailViewModel.cpp
    src/viewmodel/NeteasePlaylistDetailViewModel.cpp
    src/viewmodel/MainViewModel.cpp
)
set(VIEWMODEL_HEADERS
    src/viewmodel/ViewModelError.h
    src/viewmodel/SongListModel.h
    src/viewmodel/PlayerViewModel.h
    src/viewmodel/SettingsViewModel.h
    src/viewmodel/SearchViewModel.h
    src/viewmodel/PlaylistViewModel.h
    src/viewmodel/LocalPlaylistDetailViewModel.h
    src/viewmodel/NeteasePlaylistDetailViewModel.h
    src/viewmodel/MainViewModel.h
)
add_library(qeriplayer_viewmodel STATIC ${VIEWMODEL_SOURCES} ${VIEWMODEL_HEADERS})
target_include_directories(qeriplayer_viewmodel PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(qeriplayer_viewmodel PUBLIC
    Qt6::Core
    ${QERIPLAYER_QCORO_LIBRARIES}
)
target_compile_definitions(qeriplayer_viewmodel PRIVATE
    QT_NO_KEYWORDS
)

# ─── Netease E2E Library (shared by E2E tests and CLI tools) ──────

set(NETEASE_E2E_SOURCES
    src/api/netease/NeteaseClient.h
    src/api/netease/NeteaseClientCore.cpp
    src/api/netease/NeteaseClientAuth.cpp
    src/api/netease/NeteaseClientContent.cpp
    src/api/netease/NeteaseClientUser.cpp
    src/api/netease/NeteaseCrypto.cpp
    src/api/netease/NeteaseParser.cpp
    src/api/common/ApiError.cpp
    src/core/network/HttpClient.cpp
    src/core/network/NetworkManager.cpp
    src/core/network/NetworkMonitor.cpp
    src/core/network/WebSocketClient.cpp
    src/core/crypto/CryptoUtils.cpp
    src/core/crypto/Encryptor.cpp
    src/core/crypto/Decryptor.cpp
    src/core/crypto/SecureStorage.cpp
    src/core/filesystem/AppPaths.cpp
    src/core/filesystem/FileUtils.cpp
    src/core/filesystem/FileWatcher.cpp
    src/core/database/DatabaseManager.cpp
    src/core/logger/Logger.cpp
)
set(NETEASE_E2E_INCLUDES
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)
set(NETEASE_E2E_LIBS
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    ${QERIPLAYER_QCORO_LIBRARIES}
    ${SQLITE3_LIBRARIES}
    ${SPDLOG_LIBRARIES}
    OpenSSL::Crypto
)

add_library(netease_test_lib STATIC ${NETEASE_E2E_SOURCES})
target_include_directories(netease_test_lib PUBLIC ${NETEASE_E2E_INCLUDES})
target_link_libraries(netease_test_lib PUBLIC ${NETEASE_E2E_LIBS})

# ─── Tests ────────────────────────────────────────────────────────

enable_testing()
find_package(Qt6 REQUIRED COMPONENTS Test)

add_subdirectory(tests)
