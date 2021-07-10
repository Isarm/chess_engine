# chess_engine

This is a chess engine that I am writing. It uses bitboards to efficiently calculate positions. In the evaluation alpha beta pruning is used, combined with transposition tables and iterative deepening to more efficiently prune the search tree. 

Game logic has been implemented and tested with the PERFT debugging method:
https://www.chessprogramming.org/Perft

## Structure
### Game logic
The position class is responsible for the chess game logic. It handles everything related to changing the position. 
The most important member function is the generateMoves function, which generates a list of possible legal moves in the current position. 
This, combined with the doMove and undoMove functions, can then be used to traverse the search tree.

The definitions.h file contains enums, to improve readability of the code.
The useful.h file contains some useful short helper functions that the position class uses. 
The magic-bits directory is taken from: 
https://github.com/goutham/magic-bits
This function is used to generate the slider attacks efficiently.  

The UCI protocol is implemented in the UCI class. 

The tests.cpp file is used to test the implementation using PERFT. Either common perft positions can be used from the chessprogramming wiki, 
or the perft debugging tool from:
https://github.com/agausmann/perftree

Using PERFT, around 14 million positions are found per second, on an Intel® Core™ i7-9750H CPU @ 2.60GHz × 12.

### Engine
The engine part is mainly implemented in the Evaluation class. The alpha beta pruning search algorithm is implemented in the alphabeta function. At the leaf nodes, a quiescence search is performed up to a depth of 7 to improve search stability. A transposition table is implemented in the transpositionTable class. Iterative deepening has also been implemented in the start search function, which repeatedly calls the alphabeta search function with increasing depth. 

## How to use
Clone the repository and build using:
```
cmake .
make
```
The engine uses UCI protocol, so a GUI like Arena can be used to load the engine and play against it. 

## TODO
* ~~Create a simple evaluation function~~
* ~~Implement the minimax algorithm~~
* ~~Implement the UCI protocol such that a GUI can be used to improve testing~~
* ~~Implement Alpha Beta pruning~~
* ~~Implement Iterative Deepening using a transposition table~~
* Further improve the evaluation function
* Improve moves ordering
