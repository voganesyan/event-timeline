cmake_minimum_required(VERSION 3.16)
project(event-timeline LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(event-timeline
    bookmark.h
    bookmarksmodel.cpp bookmarksmodel.h
    bookmarksview.cpp bookmarksview.h
    mainwindow.cpp mainwindow.h
    main.cpp
)

target_link_libraries(event-timeline PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)
