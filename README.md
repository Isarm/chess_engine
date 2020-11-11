# chess_engine

This is a chess engine that I am writing. It uses bitboards to efficiently calculate positions.

Currently, I am in the final stages of implementing the chess game logic. The last thing that has to be implemented is pawn promotion.
Everything else has been tested using the PERFT debugging method:

https://www.chessprogramming.org/Perft

## Structure
The position class is responsible for the chess game logic. It handles everything related to changing the position. 
The most important member function is the generateMoves function, which generates a list of possible legal moves in the current position. 
This, combined with the doMove and undoMove functions, can then be used to traverse the search tree.

The definitions.h file contains usefuls enums, to improve readability of the code.
The useful.h file contains some useful short helper functions that the position class uses. 

The UCI protocol will later be further implemented in the UCI class.

Currently the tests.cpp file is used to test the implementation.

## TODO
* Implement promotion
* Create a simple evaluation function
* Implement the minimax algorithm 
* Implement the UCI protocol such that a GUI can be used to improve testing
* Implement Alpha Beta pruning
* Implement Iterative Deepening using a transposition table
