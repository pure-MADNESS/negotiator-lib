cmake_minimum_required(VERSION 3.20)
project(negotiator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(negotiator STATIC 
    src/negotiator.cpp 
    src/negotiator.hpp
)


target_include_directories(negotiator PUBLIC src)

find_package(nlohmann_json QUIET)
target_link_libraries(negotiator PUBLIC nlohmann_json::nlohmann_json)