# Domain tests

add_executable(TestDomainModels
    TestDomainModels.cpp
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
