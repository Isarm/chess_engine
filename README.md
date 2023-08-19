# DISCLAIMER
I've started this project over 4 years ago as one of my first big c++ (or even programming in general) projects. The code quality is therefore quite bad. It does not use modern c++ features like the STL or smart pointers, or things like inheritance and templates. Basically I wrote this as a (bad quality) C project with classes. Maybe I will write a new engine in the future, focusing on readability and extendability and applying the things I've learned over the years in other C++ projects.

# chess_engine

This chess engine uses bitboards to efficiently calculate positions. In the evaluation alpha beta pruning is used, combined with a transposition table and iterative deepening to more efficiently prune the search tree. 

Game logic has been implemented and tested with the PERFT debugging method:
https://www.chessprogramming.org/Perft

## Structure
### Game logic
The position class is responsible for the chess game logic. It handles everything related to changing the position. 
The most important method is `generateMoves`, which generates a list of possible legal moves in the current position. 
This, combined with the `doMove` and `undoMove` methods, can then be used to traverse the search tree.

The definitions.h file contains enums, to improve readability of the code.
The useful.h file contains some useful short helper functions that the position class uses. 
The magic-bits directory is taken from: 
https://github.com/goutham/magic-bits  
This library is used to generate the slider attacks efficiently.  

The UCI protocol is implemented in the UCI class. 

The tests.cpp file is used to test the implementation using PERFT. Either common perft positions can be used from the chessprogramming wiki, 
or the perft debugging tool from:
https://github.com/agausmann/perftree

Using PERFT, around 100 million positions are found per second, on an Intel® Core™ i7-9750H CPU @ 2.60GHz × 12.

### Engine
The engine part is mainly implemented in the Evaluation class. The alpha beta pruning search algorithm is implemented in the alphabeta function. At the leaf nodes, a quiescence search is performed to improve search stability. A transposition table is implemented in the transpositionTable class. Iterative deepening has also been implemented in the start search function, which repeatedly calls the alphabeta search function with increasing depth, while saving the previously examined best line.

Multithreading is implemented using the Lazy SMP approach.

## How to use
Clone the repository and build using:
```
cmake .
make
```
The engine uses UCI protocol, so a GUI like Arena can be used to load the engine and play against it. 

To demo in the command line:

```
uci
isready
```

Wait for the `readyok` answer

```
position startpos
go
```

`startpos` can be swapped for any valid FEN. To stop the search simply type `stop`. For more commands and information on the UCI protocol, go to: 

http://wbec-ridderkerk.nl/html/UCIProtocol.html


## TODO

* ~~Create a simple evaluation function~~
* ~~Implement the minimax algorithm~~
* ~~Implement the UCI protocol such that a GUI can be used to improve testing~~
* ~~Implement Alpha Beta pruning~~
* ~~Implement Iterative Deepening using a transposition table~~
* Fix inconsistent usage of constants (e.g. for mate score, move ordering)
* Cleanup uci.cpp: Make number of threads configureable at runtime, put the default settings in a proper place. 
* Cleanup in general, there is a lot of ugly code in this project.
* Further improve the evaluation function: 1st step is probably pawn structure evaluation
* Improve move ordering
