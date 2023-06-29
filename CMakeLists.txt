cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_FLAGS "-std=c99")


file(GLOB SRC_LIST "src/*.c" "thirdparty/log.c/src/*.c")
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/log.c/src
)

add_executable(rvcc ${SRC_LIST})