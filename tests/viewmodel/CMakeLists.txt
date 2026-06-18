# ViewModel tests

add_executable(TestViewModelError
    TestViewModelError.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/ViewModelError.cpp
    ${CMAKE_SOURCE_DIR}/src/api/common/ApiError.cpp
)
target_include_directories(TestViewModelError PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(TestViewModelError PRIVATE
    Qt6::Core
    Qt6::Test
)
target_compile_definitions(TestViewModelError PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestViewModelError COMMAND TestViewModelError)

add_executable(TestSongListModel
    TestSongListModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/SongListModel.cpp
    ${CMAKE_SOURCE_DIR}/src/domain/Song.cpp
)
target_include_directories(TestSongListModel PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(TestSongListModel PRIVATE
    Qt6::Core
    Qt6::Test
)
target_compile_definitions(TestSongListModel PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestSongListModel COMMAND TestSongListModel)

add_executable(TestPlayerViewModel
    TestPlayerViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/PlayerViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/ViewModelError.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/SongListModel.cpp
    ${CMAKE_SOURCE_DIR}/src/player/PlaybackController.cpp
    ${CMAKE_SOURCE_DIR}/src/player/PlayQueue.cpp
    ${CMAKE_SOURCE_DIR}/src/player/IPlayerBackend.h
    ${CMAKE_SOURCE_DIR}/src/api/common/ApiError.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logger/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/domain/Song.cpp
)
target_include_directories(TestPlayerViewModel PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SPDLOG_INCLUDE_DIRS}
)
target_link_libraries(TestPlayerViewModel PRIVATE
    Qt6::Core
    Qt6::Multimedia
    Qt6::Test
    ${QERIPLAYER_QCORO_LIBRARIES}
    ${SPDLOG_LIBRARIES}
)
target_compile_definitions(TestPlayerViewModel PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestPlayerViewModel COMMAND TestPlayerViewModel)
set_tests_properties(TestPlayerViewModel PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(TestSettingsViewModel
    TestSettingsViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/SettingsViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/ViewModelError.cpp
    ${CMAKE_SOURCE_DIR}/src/api/common/ApiError.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClientCore.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClientAuth.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClientContent.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClientUser.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseCrypto.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseParser.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClient.h
    ${CMAKE_SOURCE_DIR}/src/core/network/HttpClient.cpp
    ${CMAKE_SOURCE_DIR}/src/core/network/HttpClient.h
    ${CMAKE_SOURCE_DIR}/src/core/crypto/CryptoUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/Encryptor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/Decryptor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/SecureStorage.cpp
    ${CMAKE_SOURCE_DIR}/src/core/filesystem/AppPaths.cpp
    ${CMAKE_SOURCE_DIR}/src/core/filesystem/FileUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/core/filesystem/FileWatcher.cpp
    ${CMAKE_SOURCE_DIR}/src/core/database/DatabaseManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logger/Logger.cpp
)
target_include_directories(TestSettingsViewModel PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)
target_link_libraries(TestSettingsViewModel PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    Qt6::Test
    ${QERIPLAYER_QCORO_LIBRARIES}
    ${SQLITE3_LIBRARIES}
    ${SPDLOG_LIBRARIES}
    OpenSSL::Crypto
)
if(UNIX)
    target_link_libraries(TestSettingsViewModel PRIVATE dl pthread)
endif()
add_test(NAME TestSettingsViewModel COMMAND TestSettingsViewModel)
set_tests_properties(TestSettingsViewModel PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(TestSearchViewModel
    TestSearchViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/SearchViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/SongListModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/ViewModelError.cpp
    ${CMAKE_SOURCE_DIR}/src/api/common/ApiError.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logger/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/domain/Song.cpp
)
target_include_directories(TestSearchViewModel PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SPDLOG_INCLUDE_DIRS}
)
target_link_libraries(TestSearchViewModel PRIVATE
    Qt6::Core
    Qt6::Test
    ${QERIPLAYER_QCORO_LIBRARIES}
    ${SPDLOG_LIBRARIES}
)
target_compile_definitions(TestSearchViewModel PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestSearchViewModel COMMAND TestSearchViewModel)

add_executable(TestPlaylistViewModel
    TestPlaylistViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/PlaylistViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/ViewModelError.cpp
    ${CMAKE_SOURCE_DIR}/src/domain/PlaylistSummary.cpp
    ${CMAKE_SOURCE_DIR}/src/api/common/ApiError.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClientCore.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClientAuth.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClientContent.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClientUser.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseCrypto.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseParser.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClient.h
    ${CMAKE_SOURCE_DIR}/src/core/network/HttpClient.cpp
    ${CMAKE_SOURCE_DIR}/src/core/network/HttpClient.h
    ${CMAKE_SOURCE_DIR}/src/core/crypto/CryptoUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/Encryptor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/Decryptor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/SecureStorage.cpp
    ${CMAKE_SOURCE_DIR}/src/core/filesystem/AppPaths.cpp
    ${CMAKE_SOURCE_DIR}/src/core/filesystem/FileUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/core/filesystem/FileWatcher.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/PlaylistRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/SongRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/SqlRowMapper.cpp
    ${CMAKE_SOURCE_DIR}/src/core/database/DatabaseManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logger/Logger.cpp
)
target_include_directories(TestPlaylistViewModel PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)
target_link_libraries(TestPlaylistViewModel PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    Qt6::Test
    ${QERIPLAYER_QCORO_LIBRARIES}
    ${SQLITE3_LIBRARIES}
    ${SPDLOG_LIBRARIES}
    OpenSSL::Crypto
)
if(UNIX)
    target_link_libraries(TestPlaylistViewModel PRIVATE dl pthread)
endif()
add_test(NAME TestPlaylistViewModel COMMAND TestPlaylistViewModel)
set_tests_properties(TestPlaylistViewModel PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(TestLocalPlaylistDetailViewModel
    TestLocalPlaylistDetailViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/LocalPlaylistDetailViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/SongListModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/ViewModelError.cpp
    ${CMAKE_SOURCE_DIR}/src/api/common/ApiError.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/PlaylistRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/SongRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/SqlRowMapper.cpp
    ${CMAKE_SOURCE_DIR}/src/core/database/DatabaseManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logger/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/domain/Song.cpp
)
target_include_directories(TestLocalPlaylistDetailViewModel PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)
target_link_libraries(TestLocalPlaylistDetailViewModel PRIVATE
    Qt6::Core
    Qt6::Test
    ${QERIPLAYER_QCORO_LIBRARIES}
    ${SQLITE3_LIBRARIES}
    ${SPDLOG_LIBRARIES}
)
target_compile_definitions(TestLocalPlaylistDetailViewModel PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestLocalPlaylistDetailViewModel COMMAND TestLocalPlaylistDetailViewModel)

add_executable(TestNeteasePlaylistDetailViewModel
    TestNeteasePlaylistDetailViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/NeteasePlaylistDetailViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/SongListModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/ViewModelError.cpp
    ${CMAKE_SOURCE_DIR}/src/api/common/ApiError.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClient.h
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClientCore.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClientAuth.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClientContent.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClientUser.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseCrypto.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseParser.cpp
    ${CMAKE_SOURCE_DIR}/src/core/network/HttpClient.h
    ${CMAKE_SOURCE_DIR}/src/core/network/HttpClient.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/CryptoUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/Encryptor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/Decryptor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/SecureStorage.cpp
    ${CMAKE_SOURCE_DIR}/src/core/filesystem/AppPaths.cpp
    ${CMAKE_SOURCE_DIR}/src/core/filesystem/FileUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/core/filesystem/FileWatcher.cpp
    ${CMAKE_SOURCE_DIR}/src/core/database/DatabaseManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logger/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/domain/Song.cpp
)
target_include_directories(TestNeteasePlaylistDetailViewModel PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)
target_link_libraries(TestNeteasePlaylistDetailViewModel PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    Qt6::Test
    ${QERIPLAYER_QCORO_LIBRARIES}
    ${SQLITE3_LIBRARIES}
    ${SPDLOG_LIBRARIES}
    OpenSSL::Crypto
)
if(UNIX)
    target_link_libraries(TestNeteasePlaylistDetailViewModel PRIVATE dl pthread)
endif()
add_test(NAME TestNeteasePlaylistDetailViewModel COMMAND TestNeteasePlaylistDetailViewModel)
set_tests_properties(TestNeteasePlaylistDetailViewModel PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(TestMainViewModel
    TestMainViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/MainViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/domain/PlaylistSummary.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/PlayerViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/SearchViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/PlaylistViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/SettingsViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/LocalPlaylistDetailViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/NeteasePlaylistDetailViewModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/SongListModel.cpp
    ${CMAKE_SOURCE_DIR}/src/viewmodel/ViewModelError.cpp
    ${CMAKE_SOURCE_DIR}/src/player/PlaybackController.cpp
    ${CMAKE_SOURCE_DIR}/src/player/PlayQueue.cpp
    ${CMAKE_SOURCE_DIR}/src/player/IPlayerBackend.h
    ${CMAKE_SOURCE_DIR}/src/api/common/ApiError.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClient.h
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClientCore.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClientAuth.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClientContent.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseClientUser.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseCrypto.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseParser.cpp
    ${CMAKE_SOURCE_DIR}/src/core/network/HttpClient.h
    ${CMAKE_SOURCE_DIR}/src/core/network/HttpClient.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/CryptoUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/Encryptor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/Decryptor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/SecureStorage.cpp
    ${CMAKE_SOURCE_DIR}/src/core/filesystem/AppPaths.cpp
    ${CMAKE_SOURCE_DIR}/src/core/filesystem/FileUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/core/filesystem/FileWatcher.cpp
    ${CMAKE_SOURCE_DIR}/src/core/database/DatabaseManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logger/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/domain/Song.cpp
)
target_include_directories(TestMainViewModel PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${SQLITE3_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)
target_link_libraries(TestMainViewModel PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    Qt6::Multimedia
    Qt6::Test
    ${QERIPLAYER_QCORO_LIBRARIES}
    ${SQLITE3_LIBRARIES}
    ${SPDLOG_LIBRARIES}
    OpenSSL::Crypto
)
if(UNIX)
    target_link_libraries(TestMainViewModel PRIVATE dl pthread)
endif()
add_test(NAME TestMainViewModel COMMAND TestMainViewModel)
set_tests_properties(TestMainViewModel PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
