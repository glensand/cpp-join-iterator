cmake_minimum_required(VERSION 3.11)


project(galculator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB G_HEADERS
    *.h)

file(GLOB G_SOURSES
    *.cpp)

add_executable(galculator ${G_HEADERS} ${G_SOURSES})
