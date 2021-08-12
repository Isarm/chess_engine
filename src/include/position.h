#include <cstdint>
#include <iostream>
#include "definitions.h"
#include "slider_attacks.h"
#include "LookupTables.h"

using namespace std;
using namespace definitions;

#ifndef POSITION_H
#define POSITION_H

extern const int PIECEWEIGHTS[6];

class Position{
public:
    uint64_t getPositionHash();

    explicit Position(string FEN);

    void GenerateMoves(moveList &movelist, bool onlyCaptures = false);

    void doMove(unsigned moveL);
    void doMove(const string&);
    void undoMove();

    void doNullMove();
    void undoNullMove();

    void prettyPrint();

    perftCounts PERFT(int depth, bool tree = true);

    int Evaluate();

    bool isDraw();

    int getEvaluation();

    int getLazyEvaluation();

    static void sortMoves(moveList &list);

    unsigned halfMoveNumber = 0;
    bool isIncheck = false;
    bool turn = WHITE;
private:

    // used to determine when an endgame occurs.
    // all the values of the pieces (no pawns) are added together, and if they are lower than a certain threshold,
    // the isEndGame flag will be set to true.
    int allPiecesValue = 0;
    int allPiecesValues[1024];
    float endGameFraction = 0;

    unsigned castlingRights = 0;
    uint64_t bitboards[2][8] = {0};
    uint64_t helpBitboards[2] = {0};

    uint64_t positionHashes[1024] = {0};

    int positionEvaluations[1024] = {0};

    unsigned halfMovesSinceIrrepr = 0;

    uint64_t previousMoves[1024] = {0};
    unsigned halfMoveNumber50 = 0;
    unsigned fullMoveNumber;

    SliderAttacks sliderAttacks;

    void generateHelpBitboardsAndIsInCheck();
    uint64_t calculateHash();
    LookupTables LUTs;
    int filesAndPawns = 0;

    void GeneratePawnMoves(moveList &movelist, bool onlyCaptures = false);
    void GenerateKnightMoves(moveList &movelist, bool onlyCaptures = false);
    void GenerateSliderMoves(moveList &movelist, bool onlycaptures = false);
    void GenerateKingMoves(moveList &movelist, bool onlycaptures = false);
    void GenerateCastlingMoves(moveList &movelist);

    void bitboardsToLegalMovelist(moveList &movelist, uint64_t origin, uint64_t destinations, uint64_t captureDestinations, Pieces piece,
                                  bool kingMoveFlag = false, bool enPassantMoveFlag = false, bool promotionMoveFlag = false); // flags


    short squareAttackedBy(uint64_t square, const bool colour, uint64_t * attacker = nullptr);
    uint64_t pinnedPieces(uint64_t pinnedOrigin, bool colour);

    void MovePiece(uint64_t originBB, uint64_t destinationBB, bool colour);



    void CastlingToMovelist(moveList &movelist, unsigned int castlingType, uint64_t empty, uint64_t nonattacked);

    void doCastlingMove(bool side);

    void undoCastlingMove(bool side);

    void removePiece(unsigned int pieceType, uint64_t pieceBB, bool colour, unsigned int pieceInt);

    void addPiece(unsigned int pieceType, uint64_t pieceBB, bool colour, unsigned int pieceInt);

    int getPieceValue(bool side, uint64_t bb);

    int calculateMobilityBonus(int currentMobility, unsigned destinationInt, Pieces piece);

    static int popCount(uint64_t destinations);

    int calculateMobility(bool side);

    int staticExchangeEvaluation(uint64_t squareBB, bool side);

    short getPieceType(bool side, const uint64_t pieceBB);

    int staticExchangeEvaluationCapture(uint64_t from, uint64_t to, bool side);

    int calculateFileAndPawnScore(bool turn);

};

#endif