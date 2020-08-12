#include <cstdint>
#include <iostream>
#include "types.h"

using namespace std;
using namespace types;

#ifndef POSITION_H
#define POSITION_H

class Position{
public:

    bool turn = WHITE_TURN;
    uint64_t bitboards[14]{};
    explicit Position(string FEN);

    void initializeHelpBitboards();

    void GeneratePseudoLegalMoves(moveList &movelist);

    void GeneratePseudoLegalKnightMoves(moveList &movelist);


};

#endif