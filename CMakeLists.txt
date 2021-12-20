# Project
cmake_minimum_required(VERSION 3.16)
project(MacShell)

set(CMAKE_CXX_STANDARD 11)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

file(GLOB SOURCES ./src/*.cpp)

foreach(file_cpp IN ITEMS ${SOURCES})
  # Get file name without directory
  get_filename_component(name_cpp ${file_cpp} NAME_WE)
  add_executable(${name_cpp} ${file_cpp})
  set(TARGETNAME ${name_cpp})
endforeach()

add_custom_command(
  TARGET ${TARGETNAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/scripts ${EXECUTABLE_OUTPUT_PATH}
)
