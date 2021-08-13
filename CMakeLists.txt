cmake_minimum_required(VERSION 3.17)
project(engine)

set(OPTIMIZATION_FLAGS "-Ofast -funroll-loops")
#set(OPTIMIZATION_FLAGS "")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-pthread ${OPTIMIZATION_FLAGS}" )

include_directories(src/include
                    src/
                    magic-bits/src)


set(PROJECT_HEADERS
        src/include/bitboard.h
        src/include/LookupTables.h
        src/include/definitions.h
        src/include/position.h
        src/include/uci.h
        src/include/useful.h
        src/include/threadManager.h
        magic-bits/src/slider_attacks.h
        src/include/tests.h
        src/include/evaluate.h
        src/include/transpositionTable.h
        src/include/thread.h
        )
set(PROJECT_SOURCES
        src/main.cpp
        src/LookupTables.cpp
        src/bitboard.cpp
        src/main.cpp
        src/position.cpp
        src/uci.cpp
        src/benchmarks.cpp
        src/threadManager.cpp
        magic-bits/src/slider_attacks.cpp
        src/tests.cpp
        src/evaluate.cpp
        src/transpositionTable.cpp
        src/thread.cpp
        )

add_executable(engine ${PROJECT_SOURCES} ${PROJECT_HEADERS})



