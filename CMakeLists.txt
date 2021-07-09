cmake_minimum_required(VERSION 3.17)
project(engine)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -pthread)

include_directories(src/include
                magic-bits/src)

add_executable(engine
        src/include/bitboard.h
        src/include/definitions.h
        src/include/position.h
        src/include/uci.h
        src/bitboard.cpp
        src/main.cpp
        src/position.cpp
        src/uci.cpp
        src/include/useful.h
        src/exitTimer.cpp
        #magic-bits
        magic-bits/src/slider_attacks.h
        magic-bits/src/slider_attacks.cpp src/tests.cpp src/include/tests.h src/evaluate.cpp src/include/evaluate.h src/transpositionTable.cpp src/include/transpositionTable.h src/include/lookupTables.h)

