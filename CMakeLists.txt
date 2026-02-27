cmake_minimum_required(VERSION 3.20)
project(madness_dispatch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(madness_dispatch STATIC 
    src/negotiator.cpp 
    src/negotiator.hpp
)


target_include_directories(madness_dispatch PUBLIC src)

find_package(nlohmann_json QUIET)
target_link_libraries(madness_dispatch PUBLIC nlohmann_json::nlohmann_json)