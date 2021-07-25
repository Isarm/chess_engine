cmake_minimum_required(VERSION 3.17)
project(engine)



set(OPTIMIZATION_FLAGS "-Ofast -funroll-loops")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-pthread" )

include_directories(src/include
                magic-bits/src)

add_executable(engine
        src/include/bitboard.h
        src/include/definitions.h
        src/include/position.h
        src/include/uci.h
        src/include/lookupTables.h
        src/bitboard.cpp
        src/main.cpp
        src/position.cpp
        src/uci.cpp
        src/include/useful.h
        src/lookupTables.cpp
        src/benchmarks.cpp
        src/threadManager.cpp
        src/include/threadManager.h
        #magic-bits
        magic-bits/src/slider_attacks.h
        magic-bits/src/slider_attacks.cpp src/tests.cpp src/include/tests.h src/evaluate.cpp src/include/evaluate.h src/transpositionTable.cpp src/include/transpositionTable.h src/include/lookupTables.h src/thread.cpp src/include/thread.h)

