cmake_minimum_required(VERSION 3.16)
project(KeyboardEmulator VERSION 0.7.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui SerialPort)

add_executable(${PROJECT_NAME} WIN32
    resources.qrc
    src/AbstractItem.cpp
    src/AbstractItem.h
    src/ButtonItem.cpp
    src/ButtonItem.h
    src/ComPortMenu.cpp
    src/ComPortMenu.h
    src/CustomScene.cpp
    src/CustomScene.h
    src/DiodeItem.cpp
    src/DiodeItem.h
    src/IFileDialogService.h
    src/ImageZoomWidget.cpp
    src/ImageZoomWidget.h
    src/KeyboardController.cpp
    src/KeyboardController.h
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/Project.cpp
    src/Project.h
    src/ProjectIO.cpp
    src/ProjectIO.h
    src/protocol/CommandDefinition.h
    src/protocol/KeyboardControllerProtocol.h
    src/protocol/PinsDefinition.h
    src/QtFileDialogService.cpp
    src/QtFileDialogService.h
    src/RecentProjects.cpp
    src/RecentProjects.h
    src/ResizableRectItem.cpp
    src/ResizableRectItem.h
    src/ResizeHandle.cpp
    src/ResizeHandle.h
    src/SerialPortConnectionManager.cpp
    src/SerialPortConnectionManager.h
    src/SerialPortModel.cpp
    src/SerialPortModel.h
    src/StartScreenWidget.cpp
    src/StartScreenWidget.h
    src/WorkMode.h
    src/WorkModeToolbar.cpp
    src/WorkModeToolbar.h
)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol
)


target_compile_definitions(${PROJECT_NAME}
    PRIVATE APP_VERSION=\"${PROJECT_VERSION}\"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui Qt6::SerialPort
)
