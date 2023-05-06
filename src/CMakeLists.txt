cmake_minimum_required(VERSION 3.16)
project(event-timeline LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(event-timeline
    bookmarksview.cpp bookmarksview.h
    mainwindow.cpp mainwindow.h
    main.cpp
)

target_link_libraries(event-timeline PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)
