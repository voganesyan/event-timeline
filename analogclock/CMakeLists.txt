cmake_minimum_required(VERSION 3.16)
project(analogclock LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(analogclock
    analogclock.cpp analogclock.h
    main.cpp
)

target_link_libraries(analogclock PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)
