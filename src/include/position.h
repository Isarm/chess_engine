#include <cstdint>
#include <iostream>
#include "types.h"

using namespace std;

#ifndef POSITION_H
#define POSITION_H

class Position{
public:

    bool turn = WHITE_TURN;
    uint64_t bitboards[14]{};
    explicit Position(string FEN);


    int *GeneratePseudoLegalMoves(int *moveList);

    int *GeneratePseudoLegalKnightMoves(int *moveList);

    int *GeneratePseudoLegalMoves(int *moveList, int *moveListLength);

    void initializeHelpBitboards();
    int bitScanForward(uint64_t bb)
};

#endif