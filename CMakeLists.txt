cmake_minimum_required(VERSION 3.10)
project(buffersim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -w -MMD -MP")

set(SOURCES
    src/manager/buffer_manager.cpp
    src/manager/disk_manager.cpp
    src/buffer_simulator.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(buffersim ${SOURCES})
