cmake_minimum_required(VERSION 3.10)

project(gofile-dl)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -g")

add_executable(gofile-dl src/main.c)

set(CURL_LIBRARY "-lcurl")
set(CURL_INCLUDE_DIR "/usr/include/curl")

include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(gofile-dl ${CURL_LIBRARY})

set(CJSON_LIBRARY "-lcjson")
set(CJSON_INCLUDE_DIR "/usr/include/cjson")

include_directories(${CJSON_INCLUDE_DIR})
target_link_libraries(gofile-dl ${CJSON_LIBRARY})
