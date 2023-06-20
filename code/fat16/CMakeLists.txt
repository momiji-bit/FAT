cmake_minimum_required(VERSION 3.25)
project(fat16 C)

set(CMAKE_C_STANDARD 23)

set(CMAKE_BUILD_TYPE Debug)

add_executable(fat16 main.c const.h dir.c disk.c file.c stack.c sys.c Disks.c)


