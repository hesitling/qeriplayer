# Core infrastructure tests

add_executable(TestDatabase
    TestDatabase.cpp
    ${CMAKE_SOURCE_DIR}/src/core/database/DatabaseManager.cpp
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
    TestLogger.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logger/Logger.cpp
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
    TestFileSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/core/filesystem/AppPaths.cpp
    ${CMAKE_SOURCE_DIR}/src/core/filesystem/FileUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/core/filesystem/FileWatcher.cpp
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
    TestCrypto.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/CryptoUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/Encryptor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/Decryptor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/crypto/SecureStorage.cpp
    ${CMAKE_SOURCE_DIR}/src/core/filesystem/FileUtils.cpp
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
