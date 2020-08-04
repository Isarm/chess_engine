cmake_minimum_required(VERSION 3.17)
project(engine)

set(CMAKE_CXX_STANDARD 14)

include_directories(src/include)

add_executable(engine
        src/include/uci.h
        src/main.cpp
        src/uci.cpp)
