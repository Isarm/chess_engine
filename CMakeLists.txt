cmake_minimum_required(VERSION 3.17)
project(engine)

set(CMAKE_CXX_STANDARD 14)

include_directories(src/include)

add_executable(engine
        src/include/bitboard.h
        src/include/types.h
        src/include/position.h
        src/include/uci.h
        src/bitboard.cpp
        src/main.cpp
        src/position.cpp
        src/uci.cpp
        src/include/moveGenHelpFunctions.h)
