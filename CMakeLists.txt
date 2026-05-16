cmake_minimum_required(VERSION 3.14)
project(LFServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB NET_SOURCES src/net/*.cpp)

add_executable(lfserver
    src/main.cpp
    ${NET_SOURCES}
)

target_link_libraries(lfserver pthread)
