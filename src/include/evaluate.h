//
// Created by isar on 31/01/2021.
//

#ifndef ENGINE_EVALUATE_H
#define ENGINE_EVALUATE_H


#include <chrono>
#include <atomic>
#include "position.h"

extern std::atomic_bool exitFlag;

class Evaluate {

public:
    Position position = Position("startpos");

    Evaluate(string fen, vector<string> moves, Settings settings);

    int depth;

    Results StartSearch();


private:

    int Quiescence(int alpha, int beta, STATS *pStats, int depth = 7);

    int AlphaBeta(int ply, int alpha, int beta, LINE *pline, STATS *stats, LINE iterativeDeepeningLine = {});

    static void printinformation(int milliseconds, int score, LINE line, STATS stats, int i);

    void scoreMoves(moveList &list, int left, bool side);


    unsigned int killerMoves[MAX_DEPTH][KILLER_MOVE_SLOTS];
    uint64_t butterflyTable[2][64][64];

    void addKillerMove(unsigned int ply, unsigned int move);

    bool isKiller(unsigned int ply, unsigned int move);

    void updateButterflyTable(unsigned int ply, unsigned int move, bool side);

    unsigned int getButterflyScore(unsigned int ply, unsigned int move, bool side);
};


#endif //ENGINE_EVALUATE_H
