cmake_minimum_required(VERSION 3.16)

project(NeriPlayerQt VERSION 0.1.0 LANGUAGES CXX)

# C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network WebSockets)

# QCoro
find_package(QCoro6 REQUIRED COMPONENTS Core Network WebSockets)

# 启用协程支持
qcoro_enable_coroutines()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 源文件
set(SOURCES
    src/main.cpp
    src/app/NeriPlayerApplication.cpp
    src/core/network/HttpClient.cpp
    src/core/network/WebSocketClient.cpp
    src/core/network/NetworkManager.cpp
    src/core/network/NetworkMonitor.cpp
    src/mainwindow.cpp
)

# 头文件
set(HEADERS
    src/app/NeriPlayerApplication.h
    src/core/network/HttpClient.h
    src/core/network/WebSocketClient.h
    src/core/network/NetworkManager.h
    src/core/network/NetworkMonitor.h
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
    QCoro::Core
    QCoro::Network
    QCoro::WebSockets
)

# 包含目录
target_include_directories(NeriPlayerQt PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Install target
install(TARGETS NeriPlayerQt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
