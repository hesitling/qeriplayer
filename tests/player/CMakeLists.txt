# Player tests

add_executable(TestPlayQueue
    TestPlayQueue.cpp
    ${CMAKE_SOURCE_DIR}/src/player/PlayQueue.cpp
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
    TestPlaybackController.cpp
    ${CMAKE_SOURCE_DIR}/src/player/PlaybackController.cpp
    ${CMAKE_SOURCE_DIR}/src/player/PlayQueue.cpp
    ${CMAKE_SOURCE_DIR}/src/player/backends/QtMultimediaBackend.cpp
    ${CMAKE_SOURCE_DIR}/src/player/IPlayerBackend.h
    ${CMAKE_SOURCE_DIR}/src/api/common/ApiError.cpp
    ${CMAKE_SOURCE_DIR}/src/core/logger/Logger.cpp
)
target_include_directories(TestPlaybackController PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${QERIPLAYER_QCORO_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)
target_link_libraries(TestPlaybackController PRIVATE
    Qt6::Core
    Qt6::Multimedia
    Qt6::Test
    ${QERIPLAYER_QCORO_LIBRARIES}
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
    TestQtMultimediaBackend.cpp
    ${CMAKE_SOURCE_DIR}/src/player/backends/QtMultimediaBackend.cpp
    ${CMAKE_SOURCE_DIR}/src/player/IPlayerBackend.h
)
target_include_directories(TestQtMultimediaBackend PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${QERIPLAYER_QCORO_INCLUDE_DIRS}
)
target_link_libraries(TestQtMultimediaBackend PRIVATE
    Qt6::Core
    Qt6::Multimedia
    Qt6::Test
    ${QERIPLAYER_QCORO_LIBRARIES}
)
target_compile_definitions(TestQtMultimediaBackend PRIVATE
    QT_NO_KEYWORDS
)
add_test(NAME TestQtMultimediaBackend COMMAND TestQtMultimediaBackend)
set_tests_properties(TestQtMultimediaBackend PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
