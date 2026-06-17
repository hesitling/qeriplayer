# API tests

add_executable(TestNeteaseCrypto
    TestNeteaseCrypto.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseCrypto.cpp
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
    TestNeteaseParser.cpp
    ${CMAKE_SOURCE_DIR}/src/api/netease/NeteaseParser.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logger/Logger.cpp
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

# E2E tests (not added to ctest — requires credentials)
# Run manually: ./TestNeteaseE2E --phone=xxx --password=xxx

add_executable(TestNeteaseE2E
    TestNeteaseE2E.cpp
)
target_link_libraries(TestNeteaseE2E PRIVATE netease_test_lib Qt6::Test)

add_executable(NeteaseCaptchaLogin
    NeteaseCaptchaLogin.cpp
)
target_link_libraries(NeteaseCaptchaLogin PRIVATE netease_test_lib)
# Run: ./NeteaseCaptchaLogin --phone=13800138000
# Or just: ./NeteaseCaptchaLogin  (will prompt for phone)
