//
// Created by isar on 31/01/2021.
//

#ifndef ENGINE_EVALUATE_H
#define ENGINE_EVALUATE_H


#include <chrono>
#include <atomic>
#include "position.h"

class Evaluate {

public:
    Position position = Position("startpos");

    Evaluate();
    Evaluate(string fen, vector<string> moves);


    int AlphaBeta(int ply, int alpha, int beta, LINE *pline, STATS *stats, int depth, LINE iterativeDeepeningLine = {});

private:

    int Quiescence(int alpha, int beta, STATS *pStats);

    void scoreMoves(moveList &list, int left, bool side, uint64_t bestmove = 0, uint64_t iterativeDeepeningMove = 0);


    unsigned int killerMoves[MAX_DEPTH][KILLER_MOVE_SLOTS] = {};
    uint64_t butterflyTable[2][64][64] = {};

    void addKillerMove(unsigned int ply, unsigned int move);

    bool isKiller(unsigned int ply, unsigned int move);

    void updateButterflyTable(unsigned int ply, unsigned int move, bool side);

    unsigned int getButterflyScore(unsigned int ply, unsigned int move, bool side);
};


#endif //ENGINE_EVALUATE_H
