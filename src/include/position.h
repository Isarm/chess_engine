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

    // used to determine when an endgame occurs.
    // all the values of the pieces (no pawns) are added together, and if they are lower than a certain threshold,
    // the isEndGame flag will be set to true.
    int allPiecesValue = 0;
    int allPiecesValues[1024];
    float endGameFraction = 0;

    bool isIncheck = false;

    unsigned castlingRights = 0;
    uint64_t bitboards[2][8] = {0};
    uint64_t helpBitboards[2] = {0};

    uint64_t positionHashes[1024] = {0};

    int positionEvaluations[1024] = {0};

    unsigned halfMovesSinceIrrepr = 0;

    uint64_t previousMoves[1024] = {0};
    unsigned halfMoveNumber50 = 0;
    unsigned halfMoveNumber = 0;
    unsigned fullMoveNumber;

    SliderAttacks sliderAttacks;

    explicit Position(string FEN);

    void generateHelpBitboards();
    void GenerateMoves(moveList &movelist);


    void doMove(unsigned moveL);
    void doMove(const string&);
    void undoMove();

    void prettyPrint();

    perftCounts PERFT(int depth, bool tree = true);

    int Evaluate();

    bool isDraw();
    uint64_t calculateHash();

    int getEvaluation();

    static void sortMoves(moveList &list);

private:

    void GeneratePawnMoves(moveList &movelist);
    void GenerateKnightMoves(moveList &movelist);
    void GenerateSliderMoves(moveList &movelist);

    void bitboardsToLegalMovelist(moveList &movelist, uint64_t origin, uint64_t destinations, uint64_t captureDestinations, Pieces piece,
                                  bool kingMoveFlag = false, bool enPassantMoveFlag = false, bool promotionMoveFlag = false); // flags


    bool squareAttacked(uint64_t square, bool colour);
    uint64_t pinnedPieces(uint64_t pinnedOrigin, bool colour);

    void MovePiece(uint64_t originBB, uint64_t destinationBB, bool colour);

    void GenerateKingMoves(moveList &movelist);

    void GenerateCastlingMoves(moveList &movelist);

    void CastlingToMovelist(moveList &movelist, unsigned int castlingType, uint64_t empty, uint64_t nonattacked);

    void doCastlingMove(bool side);

    void undoCastlingMove(bool side);

    void removePiece(unsigned int pieceType, uint64_t pieceBB, bool colour, unsigned int pieceInt);

    void addPiece(unsigned int pieceType, uint64_t pieceBB, bool colour, unsigned int pieceInt);

    int getPieceValue(bool side, uint64_t bb);

    int calculateMobilityBonus(int currentMobility, unsigned destinationInt, Pieces piece);

    static int popCount(uint64_t destinations);

    int calculateMobility(bool turn);
};

#endif