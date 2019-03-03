cmake_minimum_required(VERSION 3.5)

project(ceng391_03T CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets CONFIG REQUIRED)

set(app_target
  image-viewer
)

set(app_target_SRCS
  image_viewer.cc
  image_window.cc
  image.cc
)

set(app_target_MOC_HDRS
  image_window.h
)

add_executable(${app_target} ${app_target_SRCS} ${app_target_MOC_SRCS})
target_link_libraries(${app_target} Qt5::Widgets)
set_target_properties(${app_target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
install(TARGETS ${app_target} RUNTIME DESTINATION bin)

