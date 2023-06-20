cmake_minimum_required(VERSION 3.25)
project(uselib C)

set(CMAKE_C_STANDARD 23)

# 指定搜索目录
link_directories(lib)

add_executable(uselib main.c)

# 指定具体链接库
target_link_libraries(uselib libXXX.dll)

