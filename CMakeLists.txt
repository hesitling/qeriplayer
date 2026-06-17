cmake_minimum_required(VERSION 3.16)

project(NeriPlayerQt VERSION 0.1.0 LANGUAGES CXX)

# C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

# Qt 6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network WebSockets Multimedia)

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

# 源文件
set(SOURCES
    src/main.cpp
    src/app/NeriPlayerApplication.cpp
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

# 头文件
set(HEADERS
    src/app/NeriPlayerApplication.h
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
    Qt6::Multimedia
    ${NERIPLAYER_QCORO_LIBRARIES}
    ${SQLITE3_LIBRARIES}
    ${SPDLOG_LIBRARIES}
    OpenSSL::Crypto
)

# 包含目录
target_include_directories(NeriPlayerQt PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)

# Install target
install(TARGETS NeriPlayerQt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

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
)
if(UNIX)
    target_link_libraries(TestDatabase PRIVATE dl pthread)
endif()
target_compile_definitions(TestDatabase PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestDatabase COMMAND TestDatabase)

add_executable(TestLogger
    tests/core/TestLogger.cpp
    src/core/logger/Logger.cpp
)
target_include_directories(TestLogger PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SPDLOG_INCLUDE_DIRS}
)
target_link_libraries(TestLogger PRIVATE
    Qt6::Core
    Qt6::Test
    ${SPDLOG_LIBRARIES}
)
if(UNIX)
    target_link_libraries(TestLogger PRIVATE pthread)
endif()
target_compile_definitions(TestLogger PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestLogger COMMAND TestLogger)

add_executable(TestFileSystem
    tests/core/TestFileSystem.cpp
    src/core/filesystem/AppPaths.cpp
    src/core/filesystem/FileUtils.cpp
    src/core/filesystem/FileWatcher.cpp
)
target_include_directories(TestFileSystem PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(TestFileSystem PRIVATE
    Qt6::Core
    Qt6::Test
)
target_compile_definitions(TestFileSystem PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestFileSystem COMMAND TestFileSystem)

add_executable(TestCrypto
    tests/core/TestCrypto.cpp
    src/core/crypto/CryptoUtils.cpp
    src/core/crypto/Encryptor.cpp
    src/core/crypto/Decryptor.cpp
    src/core/crypto/SecureStorage.cpp
    src/core/filesystem/FileUtils.cpp
)
target_include_directories(TestCrypto PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(TestCrypto PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Test
    OpenSSL::Crypto
)
target_compile_definitions(TestCrypto PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestCrypto COMMAND TestCrypto)

add_executable(TestNeteaseCrypto
    tests/api/TestNeteaseCrypto.cpp
    src/api/netease/NeteaseCrypto.cpp
)
target_include_directories(TestNeteaseCrypto PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(TestNeteaseCrypto PRIVATE
    Qt6::Core
    Qt6::Test
    OpenSSL::Crypto
)
target_compile_definitions(TestNeteaseCrypto PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestNeteaseCrypto COMMAND TestNeteaseCrypto)

add_executable(TestNeteaseParser
    tests/api/TestNeteaseParser.cpp
    src/api/netease/NeteaseParser.cpp
    src/core/logger/Logger.cpp
)
target_include_directories(TestNeteaseParser PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SPDLOG_INCLUDE_DIRS}
)
target_link_libraries(TestNeteaseParser PRIVATE
    Qt6::Core
    Qt6::Test
    ${SPDLOG_LIBRARIES}
)
target_compile_definitions(TestNeteaseParser PRIVATE
    QT_NO_KEYWORDS
    FIXTURES_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures/netease"
)
if(UNIX)
    target_link_libraries(TestNeteaseParser PRIVATE pthread)
endif()
add_test(NAME TestNeteaseParser COMMAND TestNeteaseParser)

add_executable(TestSchemaV2
    tests/repo/TestSchemaV2.cpp
    src/core/database/DatabaseManager.cpp
)
target_include_directories(TestSchemaV2 PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
)
target_link_libraries(TestSchemaV2 PRIVATE
    Qt6::Core
    Qt6::Test
    ${SQLITE3_LIBRARIES}
)
if(UNIX)
    target_link_libraries(TestSchemaV2 PRIVATE dl pthread)
endif()
target_compile_definitions(TestSchemaV2 PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestSchemaV2 COMMAND TestSchemaV2)

add_executable(TestSqlRowMapper
    tests/repo/TestSqlRowMapper.cpp
    src/repo/SqlRowMapper.cpp
    src/core/database/DatabaseManager.cpp
)
target_include_directories(TestSqlRowMapper PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
)
target_link_libraries(TestSqlRowMapper PRIVATE
    Qt6::Core
    Qt6::Test
    ${SQLITE3_LIBRARIES}
)
if(UNIX)
    target_link_libraries(TestSqlRowMapper PRIVATE dl pthread)
endif()
target_compile_definitions(TestSqlRowMapper PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestSqlRowMapper COMMAND TestSqlRowMapper)

add_executable(TestSongRepository
    tests/repo/TestSongRepository.cpp
    src/repo/SongRepository.cpp
    src/repo/SqlRowMapper.cpp
    src/core/database/DatabaseManager.cpp
    src/core/logger/Logger.cpp
)
target_include_directories(TestSongRepository PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)
target_link_libraries(TestSongRepository PRIVATE
    Qt6::Core
    Qt6::Test
    ${SQLITE3_LIBRARIES}
    ${SPDLOG_LIBRARIES}
)
if(UNIX)
    target_link_libraries(TestSongRepository PRIVATE dl pthread)
endif()
target_compile_definitions(TestSongRepository PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestSongRepository COMMAND TestSongRepository)

add_executable(TestSettingsRepository
    tests/repo/TestSettingsRepository.cpp
    src/repo/SettingsRepository.cpp
    src/core/database/DatabaseManager.cpp
)
target_include_directories(TestSettingsRepository PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
)
target_link_libraries(TestSettingsRepository PRIVATE
    Qt6::Core
    Qt6::Test
    ${SQLITE3_LIBRARIES}
)
if(UNIX)
    target_link_libraries(TestSettingsRepository PRIVATE dl pthread)
endif()
target_compile_definitions(TestSettingsRepository PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestSettingsRepository COMMAND TestSettingsRepository)

add_executable(TestPlayHistoryRepository
    tests/repo/TestPlayHistoryRepository.cpp
    src/repo/PlayHistoryRepository.cpp
    src/repo/SqlRowMapper.cpp
    src/core/database/DatabaseManager.cpp
    src/core/logger/Logger.cpp
)
target_include_directories(TestPlayHistoryRepository PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)
target_link_libraries(TestPlayHistoryRepository PRIVATE
    Qt6::Core
    Qt6::Test
    ${SQLITE3_LIBRARIES}
    ${SPDLOG_LIBRARIES}
)
if(UNIX)
    target_link_libraries(TestPlayHistoryRepository PRIVATE dl pthread)
endif()
target_compile_definitions(TestPlayHistoryRepository PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestPlayHistoryRepository COMMAND TestPlayHistoryRepository)

add_executable(TestPlaylistRepository
    tests/repo/TestPlaylistRepository.cpp
    src/repo/PlaylistRepository.cpp
    src/repo/SongRepository.cpp
    src/repo/SqlRowMapper.cpp
    src/core/database/DatabaseManager.cpp
    src/core/logger/Logger.cpp
)
target_include_directories(TestPlaylistRepository PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)
target_link_libraries(TestPlaylistRepository PRIVATE
    Qt6::Core
    Qt6::Test
    ${SQLITE3_LIBRARIES}
    ${SPDLOG_LIBRARIES}
)
if(UNIX)
    target_link_libraries(TestPlaylistRepository PRIVATE dl pthread)
endif()
target_compile_definitions(TestPlaylistRepository PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestPlaylistRepository COMMAND TestPlaylistRepository)

add_executable(TestPlayerStateRepository
    tests/repo/TestPlayerStateRepository.cpp
    src/repo/PlayerStateRepository.cpp
    src/repo/SqlRowMapper.cpp
    src/core/database/DatabaseManager.cpp
)
target_include_directories(TestPlayerStateRepository PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
)
target_link_libraries(TestPlayerStateRepository PRIVATE
    Qt6::Core
    Qt6::Test
    ${SQLITE3_LIBRARIES}
)
if(UNIX)
    target_link_libraries(TestPlayerStateRepository PRIVATE dl pthread)
endif()
target_compile_definitions(TestPlayerStateRepository PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestPlayerStateRepository COMMAND TestPlayerStateRepository)

# Player tests
add_executable(TestPlayQueue
    tests/player/TestPlayQueue.cpp
    src/player/PlayQueue.cpp
)
target_include_directories(TestPlayQueue PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(TestPlayQueue PRIVATE
    Qt6::Core
    Qt6::Test
)
target_compile_definitions(TestPlayQueue PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestPlayQueue COMMAND TestPlayQueue)

add_executable(TestPlaybackController
    tests/player/TestPlaybackController.cpp
    src/player/PlaybackController.cpp
    src/player/PlayQueue.cpp
    src/player/backends/QtMultimediaBackend.cpp
    src/player/IPlayerBackend.h
    src/api/common/ApiError.cpp
    src/core/logger/Logger.cpp
)
target_include_directories(TestPlaybackController PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${NERIPLAYER_QCORO_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)
target_link_libraries(TestPlaybackController PRIVATE
    Qt6::Core
    Qt6::Multimedia
    Qt6::Test
    ${NERIPLAYER_QCORO_LIBRARIES}
    ${SPDLOG_LIBRARIES}
)
target_compile_definitions(TestPlaybackController PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestPlaybackController COMMAND TestPlaybackController)
set_tests_properties(TestPlaybackController PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(TestQtMultimediaBackend
    tests/player/TestQtMultimediaBackend.cpp
    src/player/backends/QtMultimediaBackend.cpp
    src/player/IPlayerBackend.h
)
target_include_directories(TestQtMultimediaBackend PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${NERIPLAYER_QCORO_INCLUDE_DIRS}
)
target_link_libraries(TestQtMultimediaBackend PRIVATE
    Qt6::Core
    Qt6::Multimedia
    Qt6::Test
    ${NERIPLAYER_QCORO_LIBRARIES}
)
target_compile_definitions(TestQtMultimediaBackend PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestQtMultimediaBackend COMMAND TestQtMultimediaBackend)
set_tests_properties(TestQtMultimediaBackend PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# Shared library for Netease API + core infra (used by E2E tests and CLI tools)
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
    ${NERIPLAYER_QCORO_LIBRARIES}
    ${SQLITE3_LIBRARIES}
    ${SPDLOG_LIBRARIES}
    OpenSSL::Crypto
)

add_library(netease_test_lib STATIC ${NETEASE_E2E_SOURCES})
target_include_directories(netease_test_lib PUBLIC ${NETEASE_E2E_INCLUDES})
target_link_libraries(netease_test_lib PUBLIC ${NETEASE_E2E_LIBS})

add_executable(TestNeteaseE2E
    tests/api/TestNeteaseE2E.cpp
)
target_link_libraries(TestNeteaseE2E PRIVATE netease_test_lib Qt6::Test)
# Not added to ctest — requires credentials
# Run manually: ./TestNeteaseE2E --phone=xxx --password=xxx

add_executable(NeteaseCaptchaLogin
    tests/api/NeteaseCaptchaLogin.cpp
)
target_link_libraries(NeteaseCaptchaLogin PRIVATE netease_test_lib)
# Run: ./NeteaseCaptchaLogin --phone=13800138000
# Or just: ./NeteaseCaptchaLogin  (will prompt for phone)
