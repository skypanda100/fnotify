cmake_minimum_required(VERSION 3.7)
project(fnotify)

set(CMAKE_C_STANDARD 99)
#set(CMAKE_BUILD_TYPE Release)

set(SOURCE_FILES main.c)
add_executable(fnotify ${SOURCE_FILES})