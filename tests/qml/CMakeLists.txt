# Qt Quick tests

find_package(Qt6 REQUIRED COMPONENTS QuickTest)

set(QML_TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")
set(QML_TEST_ENV "QT_QPA_PLATFORM=offscreen")

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
    QUICK_TEST_SOURCE_DIR="${QML_TEST_DATA_DIR}"
)

add_test(NAME QmlLibraryView COMMAND TestPlayerBar -input "${QML_TEST_DATA_DIR}/tst_LibraryView.qml")
add_test(NAME QmlLoginDialog COMMAND TestPlayerBar -input "${QML_TEST_DATA_DIR}/tst_LoginDialog.qml")
add_test(NAME QmlNowPlayingInfo COMMAND TestPlayerBar -input "${QML_TEST_DATA_DIR}/tst_NowPlayingInfo.qml")
add_test(NAME QmlSettingsView COMMAND TestPlayerBar -input "${QML_TEST_DATA_DIR}/tst_SettingsView.qml")
add_test(NAME QmlToast COMMAND TestPlayerBar -input "${QML_TEST_DATA_DIR}/tst_Toast.qml")
add_test(NAME QmlTransportControls COMMAND TestPlayerBar -input "${QML_TEST_DATA_DIR}/tst_TransportControls.qml")

set_tests_properties(
    QmlLibraryView
    QmlLoginDialog
    QmlNowPlayingInfo
    QmlSettingsView
    QmlToast
    QmlTransportControls
    PROPERTIES ENVIRONMENT "${QML_TEST_ENV}"
)
