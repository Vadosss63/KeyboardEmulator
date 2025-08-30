cmake_minimum_required(VERSION 3.16)
project(KeyboardEmulator VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui SerialPort)

add_executable(${PROJECT_NAME} WIN32
    resources.qrc
    src/ButtonItem.cpp
    src/ButtonItem.h
    src/ComPortMenu.cpp
    src/ComPortMenu.h
    src/CustomScene.cpp
    src/CustomScene.h
    src/DiodeItem.cpp
    src/DiodeItem.h
    src/KeyboardControllerProtocol.h
    src/MainWindow.cpp
    src/MainWindow.h
    src/Project.cpp
    src/Project.h
    src/ProjectIO.cpp
    src/ProjectIO.h
    src/ResizableRectItem.cpp
    src/ResizableRectItem.h
    src/ResizeHandle.cpp
    src/ResizeHandle.h
    src/SerialPortModel.cpp
    src/SerialPortModel.h
    src/KeyboardController.cpp
    src/KeyboardController.h
    src/main.cpp
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE APP_VERSION=\"${PROJECT_VERSION}\"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui Qt6::SerialPort
)
