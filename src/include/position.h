#include <cstdint>
#include <iostream>
#include "types.h"

using namespace std;
using namespace types;

#ifndef POSITION_H
#define POSITION_H

class Position{
public:

    bool turn = WHITE;
    uint64_t bitboards[2][7] = {0};
    uint64_t helpBitboards[1] = {0};
    unsigned previousMoves[1024] = {0};
    unsigned halfMoveNumber = 0;
    explicit Position(string FEN);

    void generateHelpBitboards();

    void GeneratePseudoLegalMoves(moveList &movelist);

    void GeneratePseudoLegalPawnMoves(moveList &movelist);
    void GeneratePseudoLegalKnightMoves(moveList &movelist);

    void bitboardsToMovelist(moveList &movelist, uint64_t origin, uint64_t moves, uint64_t capturemove);

    void doMove(unsigned move);

    void GeneratePseudoLegalBishopMoves(moveList &movelist);

    bool isInCheck(bool side);
};

#endif