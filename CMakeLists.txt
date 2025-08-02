
# 注意：在build文件夹下面执行 cmake .. 命令，否则会在代码目录生成很多垃圾文件

cmake_minimum_required(VERSION 3.10)
project(CppWebServer)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-O2 -g -Wall")

file(GLOB_RECURSE SOURCE_FILES
    ./code/*.cpp
)

add_executable(server ${SOURCE_FILES})

set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../bin")
