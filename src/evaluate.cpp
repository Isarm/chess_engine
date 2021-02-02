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
        return Quiescence(alpha, beta, stats);
    }

    // generate list of moves
    moveList movelist;
    position.GenerateMoves(movelist);

    // check for stalemate of checkmate in case of no moves left:
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

        // no draw can occur after a capture move due to 3fold rep/50move rule
        //  invert all values for other colour
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

        //check if this move has lead to a draw (3fold rep/50move rule); as then the search can be stopped
        int score;
        if(position.isDraw()){
            position.undoMove();
            score = 0;
        }
        else {
            score = -AlphaBeta(depth - 1, -beta, -alpha, &line, stats);
            position.undoMove();
        }

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

int Evaluate::Quiescence(int alpha, int beta, STATS *stats) {
    // define stand_pat (adapted from chessprogramming wiki quiescence search)
    int stand_pat = position.Evaluate();
    if(stand_pat >= beta){
        return beta; // fail hard
    }
    if(alpha < stand_pat){
        alpha = stand_pat; // raise alpha to establish lower bound on postion
    }

    moveList movelist;
    position.GenerateMoves(movelist);

    // go over all capture moves to avoid horizon effect on tactical moves.
    for(int i = 0; i < movelist.captureMoveLength; i++){
        stats->quiescentNodes += 1;
        stats->totalNodes += 1;
        position.doMove(movelist.captureMove[i]);
        int score = -Quiescence(-beta, -alpha, stats);
        position.undoMove();

        if(score >= beta){
            return beta; // fail hard
        }
        if(score > alpha){
            alpha = score;
        }
    }
    return alpha;
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

    std::cout << "quiescent nodes " << stats.quiescentNodes << "/" << stats.totalNodes << " total nodes\n";
    std::cout << float(stats.quiescentNodes)/stats.totalNodes << "\n";
    std::cout.flush();
}



