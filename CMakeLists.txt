cmake_minimum_required(VERSION 3.5)

project(midie LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_subdirectory(midifile/)

include_directories(midifile/include)

add_executable(midie
  main.cpp
  mainwindow.cpp
  mainwindow.h
  mainwindow.ui
  pianorollwidget.cpp
  pianorollwidget.h
  pianorollutil.cpp
  pianorollutil.h
  midiworkspace.cpp
  midiworkspace.h
  midiutil.cpp
  midiutil.h
  trackchooser.cpp
  trackchooser.h
)

add_dependencies(midie midifile)

target_link_libraries(midie PRIVATE Qt5::Widgets midifile)
