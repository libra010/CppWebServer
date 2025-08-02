cmake_minimum_required(VERSION 3.10)
project(CppWebServer)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-O2 -g -Wall")

file(GLOB_RECURSE SOURCE_FILES
    ./code/*.cpp
)

add_executable(server ${SOURCE_FILES})

set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../bin")
