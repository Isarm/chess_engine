#include <cstdint>
#include <iostream>

using namespace std;

#ifndef POSITION_H
#define POSITION_H

class Position{
public:

    uint64_t bitboards[12]{};
    uint64_t WHITE_PIECES;
    uint64_t BLACK_PIECES;
    explicit Position(string FEN);


    string GenerateMoves();


    string GeneratePseudoLegalMoves();
};

#endif