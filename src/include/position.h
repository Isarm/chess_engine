#include <cstdint>
#include <iostream>
#include "types.h"
#include "slider_attacks.h"

using namespace std;
using namespace types;

#ifndef POSITION_H
#define POSITION_H

class Position{
public:

    bool turn = WHITE;
    bool isIncheck = false;
    uint64_t bitboards[2][7] = {0};
    uint64_t helpBitboards[2] = {0};

    unsigned previousMoves[1024] = {0};
    unsigned halfMoveNumber = 0;

    SliderAttacks sliderAttacks;

    explicit Position(string FEN);

    void GenerateMoves(moveList &movelist);

    void doMove(unsigned move);



private:


    void generateHelpBitboards();

    void GeneratePawnMoves(moveList &movelist);
    void GenerateKnightMoves(moveList &movelist);
    void GenerateSliderMoves(moveList &movelist);

    void bitboardsToMovelist(moveList &movelist, uint64_t origin, uint64_t moves, uint64_t capturemove);
    bool squareAttacked(uint64_t square, bool colour);
    uint64_t pinnedPieces(uint64_t pinnedOrigin, bool colour);
};

#endif