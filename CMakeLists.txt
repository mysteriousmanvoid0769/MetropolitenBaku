cmake_minimum_required(VERSION 3.10)


project(MetropolitenBaku)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
#(.dll/.so) instead of static ones (.a/.lib)

add_library(metro metro.cpp)

target_include_directories(metro PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(metro_app main.cpp)

target_link_libraries(metro_app metro)
