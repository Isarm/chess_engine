//
// Created by isar on 31/01/2021.
//

#include <string>
#include <vector>
#include <position.h>
#include "evaluate.h"
#include "definitions.h"
#include <cstring>
#include <limits>
#include <chrono>
#include "useful.h"


Evaluate::Evaluate(string fen, vector<string> moves, Settings settings) {
    this->position = Position(fen);
    this->depth = settings.depth;


    // do the moves that are given by the UCI protocol to update the position
    for(string move : moves){
        this->position.doMove(move);
    }
}


// start evaluation
Results Evaluate::StartSearch(){
    Results results;

    LINE line{};
    STATS stats{};


    auto t1 = std::chrono::high_resolution_clock::now();
    // add min+1 and max-1 because otherwise overflow occurs when inverting
    int score = AlphaBeta(depth, std::numeric_limits<int>::min() + 1, std::numeric_limits<int>::max() - 1, &line, &stats);
    auto t2 = std::chrono::high_resolution_clock::now();
    int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();


    printinformation(milliseconds, score, line, stats);

    results.bestMove = moveToStrNotation(line.principalVariation[0]);
    return results;
}

int Evaluate::AlphaBeta(int depth, int alpha, int beta, LINE *pline, STATS *stats){
    LINE line;
    if(depth == 0){
        pline->nmoves = 0;
        // do a quiescent search for the leaf nodes, to try and reduce the horizon effect (TODO)
        return Quiescence(alpha, beta);
    }

    // generate list of moves
    moveList movelist;
    position.GenerateMoves(movelist);

    // check for stalemate of checkmate:
    if(movelist.moveLength + movelist.captureMoveLength == 0){
        if(position.isIncheck){
            pline->nmoves = 0;
            // high score that cant be reached by evaluation function, thus indicating mate.
            // subtract detpth, indicating that higher depth (so faster mate) is better.
            return -1000000 - depth;

        }
        else{
            return 0; // stalemate
        }
    }

    // first do the capture moves
    for(int i = 0; i < movelist.captureMoveLength; i++){
        stats->totalNodes += 1;
        position.doMove(movelist.captureMove[i]);
        int score = -AlphaBeta(depth - 1, -beta, -alpha, &line, stats);
        position.undoMove();

        if(score >= beta){
            stats->betaCutoffs += 1;
            return beta; // beta cutoff
        }

        if(score > alpha){ // principal variation found
            alpha = score;
            pline->principalVariation[0] = movelist.captureMove[i];
            memcpy(pline->principalVariation + 1, line.principalVariation, line.nmoves * sizeof (unsigned));
            pline->nmoves = line.nmoves + 1;
        }
    }

    // then normal
    for(int i = 0; i < movelist.moveLength; i++){
        stats->totalNodes += 1;
        position.doMove(movelist.move[i]);
        int score = -AlphaBeta(depth - 1, -beta, -alpha, &line, stats);
        position.undoMove();

        if(score >= beta){
            stats->betaCutoffs += 1;
            return beta; // beta cutoff
        }

        if(score > alpha){ // principal variation found
            alpha = score;
            pline->principalVariation[0] = movelist.move[i];
            memcpy(pline->principalVariation + 1, line.principalVariation, line.nmoves * sizeof (unsigned));
            pline->nmoves = line.nmoves + 1;
        }
    }
    return alpha;
}

int Evaluate::Quiescence(int alpha, int beta){
    return this->position.Evaluate();
}

void Evaluate::printinformation(int milliseconds, int score, LINE line, STATS stats) {
    string pv[100];

    std::cout << "info score ";
    if(score >= 1000000){
        // this indicates that mate is found
        int mateIn = int((depth - score + 1000001)/2);
        std::cout << "mate " << mateIn << " pv ";
    }
    else{
        std::cout << "cp " << score << " pv ";
    }

    for(int i = 0; i < line.nmoves; i++){
        pv[i] = moveToStrNotation(line.principalVariation[i]);
        std::cout << pv[i] << " ";
    }
    std::cout << "\n";
    std::cout.flush();

    std::cout << "info time " << milliseconds;

    std::cout << " nodes " << stats.totalNodes;

    if(milliseconds !=0) {
        std::cout << " nps " << int(1000 * float(stats.totalNodes) / float(milliseconds));
    }
    std::cout << "\n";
    std::cout.flush();
}



