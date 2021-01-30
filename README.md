# chess_engine

This is a chess engine that I am writing. It uses bitboards to efficiently calculate positions.

Game logic has been implemented and tested with the PERFT debugging method:
https://www.chessprogramming.org/Perft

## Structure
The position class is responsible for the chess game logic. It handles everything related to changing the position. 
The most important member function is the generateMoves function, which generates a list of possible legal moves in the current position. 
This, combined with the doMove and undoMove functions, can then be used to traverse the search tree.

The definitions.h file contains enums, to improve readability of the code.
The useful.h file contains some useful short helper functions that the position class uses. 

The UCI protocol will later be further implemented in the UCI class.

Currently the tests.cpp file is used to test the implementation using PERFT. Either common perft positions can be used from the chessprogramming wiki, 
or the perft debugging tool from:
https://github.com/agausmann/perftree

Currently around 14 million positions are found per second, on an Intel® Core™ i7-9750H CPU @ 2.60GHz × 12.

## TODO
* Create a simple evaluation function
* Implement the minimax algorithm 
* Implement the UCI protocol such that a GUI can be used to improve testing
* Implement Alpha Beta pruning
* Implement Iterative Deepening using a transposition table
