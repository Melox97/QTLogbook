cmake_minimum_required(VERSION 3.16)


project(QTLogbook VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Network)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/contact.cpp
    src/database.cpp
    src/apiservice.cpp
    src/mainwindow.cpp
    src/logbookmodel.cpp
    src/setupdialog.cpp
    src/settingsdialog.cpp
    src/adifhandler.cpp
)

set(HEADERS
    src/contact.h
    src/database.h
    src/apiservice.h
    src/mainwindow.h
    src/logbookmodel.h
    src/setupdialog.h
    src/settingsdialog.h
    src/adifhandler.h
)

set(UI_FILES
    ui/mainwindow.ui
    ui/setupdialog.ui
)

qt_add_executable(QTLogbook
    ${SOURCES}
    ${HEADERS}
)

qt_add_resources(QTLogbook "resources"
    PREFIX "/"
    FILES
        data/bands.json
        data/modes.json
        styles/modern_style.qss
        styles/down_arrow.svg
)

target_link_libraries(QTLogbook PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
)

qt_finalize_executable(QTLogbook)