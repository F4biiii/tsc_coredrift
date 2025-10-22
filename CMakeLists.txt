cmake_minimum_required(VERSION 3.16)
project(test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_definitions(-std=c++17)
#add_definitions(-Wall -Wextra -Werror -pedantic)
#add_definitions(-g)

if(NOT DEFINED ENV{RTE_SDK})
    message(FATAL_ERROR "RTE_SDK environment variable not set")
endif()

include_directories($ENV{RTE_SDK}/build/include)
link_directories($ENV{RTE_SDK}/build/lib)

set (WRITE_SOURCES
    getCpuFrequency.cpp
    write.cpp
)

set (READ_SOURCES
    getCpuFrequency.cpp
    read.cpp
)

add_executable(write ${WRITE_SOURCES})
add_executable(read ${READ_SOURCES})

target_link_libraries(write
    rte_eal
    rte_mempool
    rte_mbuf
    rte_ethdev
)

target_link_libraries(read
    rte_eal
    rte_mempool
    rte_mbuf
    rte_ethdev
)
