cmake_minimum_required(VERSION 3.16)
project(test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_definitions(-std=c++17)
#add_definitions(-Wall -Wextra -Werror -pedantic)
#add_definitions(-g)

set (WRITE_SOURCES
    src/getCpuFrequency.cpp
    src/write.cpp
)

set (READ_SOURCES
    src/getCpuFrequency.cpp
    src/read.cpp
)

add_executable(write ${WRITE_SOURCES})
add_executable(read ${READ_SOURCES})

