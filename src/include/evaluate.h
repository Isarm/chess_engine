//
// Created by isar on 31/01/2021.
//

#ifndef ENGINE_EVALUATE_H
#define ENGINE_EVALUATE_H


#include <chrono>

class Evaluate {

public:
    Position position = Position("startpos");

    Evaluate(string fen, vector<string> moves, Settings settings);

    int depth;

    Results StartSearch();

private:

    int Quiescence(int alpha, int beta, STATS *pStats);

    int AlphaBeta(int depthLeft, int alpha, int beta, LINE *pline, STATS *stats, LINE iterativeDeepeningLine = {});

    void printinformation(int milliseconds, int score, LINE line, STATS stats, int i);
};


#endif //ENGINE_EVALUATE_H
