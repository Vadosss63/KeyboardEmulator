cmake_minimum_required(VERSION 3.16)
project(KeyboardEmulator VERSION 0.2.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui SerialPort)

add_executable(${PROJECT_NAME}
    resources.qrc
    src/ButtonItem.h
    src/ButtonItem.cpp
    src/CustomScene.h
    src/DiodeItem.h
    src/DiodeItem.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/ResizableRectItem.h
    src/ResizableRectItem.cpp
    src/ResizeHandle.h
    src/ResizeHandle.cpp
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui Qt6::SerialPort
)
