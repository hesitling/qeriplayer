# Repository tests

add_executable(TestSchemaV2
    TestSchemaV2.cpp
    ${CMAKE_SOURCE_DIR}/src/core/database/DatabaseManager.cpp
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
    TestSqlRowMapper.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/SqlRowMapper.cpp
    ${CMAKE_SOURCE_DIR}/src/core/database/DatabaseManager.cpp
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
    TestSongRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/SongRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/SqlRowMapper.cpp
    ${CMAKE_SOURCE_DIR}/src/core/database/DatabaseManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logger/Logger.cpp
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
    TestSettingsRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/SettingsRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/core/database/DatabaseManager.cpp
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
    TestPlayHistoryRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/PlayHistoryRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/SqlRowMapper.cpp
    ${CMAKE_SOURCE_DIR}/src/core/database/DatabaseManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logger/Logger.cpp
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
    TestPlaylistRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/PlaylistRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/SongRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/SqlRowMapper.cpp
    ${CMAKE_SOURCE_DIR}/src/core/database/DatabaseManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logger/Logger.cpp
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
    TestPlayerStateRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/PlayerStateRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/repo/SqlRowMapper.cpp
    ${CMAKE_SOURCE_DIR}/src/core/database/DatabaseManager.cpp
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
