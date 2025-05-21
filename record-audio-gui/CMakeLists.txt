cmake_minimum_required(VERSION 3.21.1)

project(recorder VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS .)

find_package(PkgConfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()
pkg_search_module(AVUTIL REQUIRED libavutil)
pkg_search_module(AVCODEC REQUIRED libavcodec)
pkg_search_module(AVDEVICE REQUIRED libavdevice)
pkg_search_module(AVFORMAT REQUIRED libavformat)

qt_add_executable(recorder
  main.ui
  main.cc)

target_include_directories(recorder
  PRIVATE
  ${AVUTIL_INCLUDE_DIRS})

target_link_libraries(recorder
  PRIVATE
  Qt6::Widgets
  ${AVUTIL_LIBRARIES}
  ${AVCODEC_LIBRARIES}
  ${AVDEVICE_LIBRARIES}
  ${AVFORMAT_LIBRARIES})

set_target_properties(recorder
  PROPERTIES
  MACOSX_BUNDLE ON
  WIN32_EXECUTABLE ON)
