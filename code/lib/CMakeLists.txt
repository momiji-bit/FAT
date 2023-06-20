









cmake_minimum_required(VERSION 3.25)
project(lib C)

set(CMAKE_C_STANDARD 23)

# 添加要编译为库的源代码文件
set(SOURCE_FILES dir.c disk.c file.c main.c stack.c sys.c)
# 生成动态链接库
#add_library(lib SHARED ${SOURCE_FILES})
# 生成静态链接库
add_library(lib STATIC ${SOURCE_FILES})

set_target_properties(lib PROPERTIES OUTPUT_NAME "XXX")
# 设置静态链接库的文件后缀名为 .lib
set(CMAKE_STATIC_LIBRARY_SUFFIX ".lib")








