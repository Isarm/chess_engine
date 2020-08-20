#include <cstdint>
#include <iostream>
#include "definitions.h"
#include "slider_attacks.h"

using namespace std;
using namespace definitions;

#ifndef POSITION_H
#define POSITION_H

class Position{
public:

    bool turn = WHITE;
    bool isIncheck = false;
    unsigned castlingRights = 0;
    uint64_t bitboards[2][8] = {0};
    uint64_t helpBitboards[2] = {0};

    uint64_t previousMoves[1024] = {0};
    unsigned halfMoveNumberTotal = 0;
    unsigned halfMoveNumber50 = 0;
    unsigned fullMoveNumber;

    SliderAttacks sliderAttacks;

    explicit Position(string FEN);

    void generateHelpBitboards();
    void GenerateMoves(moveList &movelist);

    void doMove(unsigned moveL);
    void doMove(char *move);
    void undoMove();

    void prettyPrint();

    perftCounts PERFT(int depth, bool tree = true);
private:



    void GeneratePawnMoves(moveList &movelist);
    void GenerateKnightMoves(moveList &movelist);
    void GenerateSliderMoves(moveList &movelist);

    void bitboardsToLegalMovelist(moveList &movelist, uint64_t origin, uint64_t destinations, uint64_t captureDestinations, bool kingMoveFlag = false, bool enPassantMoveFlag = false);
    bool squareAttacked(uint64_t square, bool colour);
    uint64_t pinnedPieces(uint64_t pinnedOrigin, bool colour);

    void MovePiece(uint64_t originBB, uint64_t destinationBB, bool colour);

    void GenerateKingMoves(moveList &movelist);

    void enPassantToMoveList(moveList &movelist, uint64_t pieceBB, uint64_t enPassantMove);

    void GenerateCastlingMoves(moveList &movelist);

    void CastlingToMovelist(moveList &movelist, unsigned int castlingType, uint64_t empty, uint64_t nonattacked);

    void doCastlingMove(bool side);

    void undoCastlingMove(bool side);
};

#endif