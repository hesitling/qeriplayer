# Qt Quick tests

find_package(Qt6 REQUIRED COMPONENTS QuickTest)

# Test executable
add_executable(TestPlayerBar
    test_main.cpp
)
target_include_directories(TestPlayerBar PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(TestPlayerBar PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickTest
    Qt6::QuickControls2
)
target_compile_definitions(TestPlayerBar PRIVATE
    QT_NO_KEYWORDS
    QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

add_test(NAME TestPlayerBar COMMAND TestPlayerBar -input "${CMAKE_CURRENT_SOURCE_DIR}/data")
set_tests_properties(TestPlayerBar PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
