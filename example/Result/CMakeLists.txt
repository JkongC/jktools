cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(example)

add_executable(example)

target_sources(example PRIVATE src/main.cpp)
target_include_directories(example PRIVATE src
${CMAKE_SOURCE_DIR}/../../include)