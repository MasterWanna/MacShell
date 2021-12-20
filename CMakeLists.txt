# Project
cmake_minimum_required(VERSION 3.16)
project(MacShell)

set(CMAKE_CXX_STANDARD 11)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

file(GLOB allScripts "${CMAKE_CURRENT_SOURCE_DIR}/scripts/*")

add_executable(readbyte src/readbyte.cpp)
add_custom_command(TARGET readbyte
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/scripts ${EXECUTABLE_OUTPUT_PATH}
)
