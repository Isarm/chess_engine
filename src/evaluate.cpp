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
#include "transpositionTable.h"
#include <algorithm>


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
    LINE previousBestLine{};
    STATS stats{};


    auto t1 = std::chrono::high_resolution_clock::now();

    int score = 0;
    for(int iterativeDepth = 1; iterativeDepth <= depth; iterativeDepth++) {
        line = {};
        // add min+1 and max-1 because otherwise overflow occurs when inverting
        score = AlphaBeta(iterativeDepth, std::numeric_limits<int>::min() + 1, std::numeric_limits<int>::max() - 1, &line,
                          &stats, previousBestLine);
        previousBestLine = line;

        auto t2 = std::chrono::high_resolution_clock::now();
        int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        printinformation(milliseconds, score, line, stats, iterativeDepth);

        //* TODO: make this a proper setting */
        if(milliseconds > 2000){
            break;
        }

        if(abs(score) >= 1000000) {
            // this indicates that mate is found
            break;
        }
    }
    results.bestMove = moveToStrNotation(line.principalVariation[0]);
    return results;
}

int Evaluate::AlphaBeta(int depthLeft, int alpha, int beta, LINE *pline, STATS *stats, LINE iterativeDeepeningLine) {
    /**
     * This has become quite ugly with a lot of repetition
     */
    int alphaStart = alpha; // to be able to check if alpha has increased with this position (for tr. table)

    LINE line;
    if(depthLeft == 0){
        pline->nmoves = 0;
        // do a quiescent search for the leaf nodes, to reduce the horizon effect
        return Quiescence(alpha, beta, stats);
    }

    unsigned iterativeDeepeningMove = 0;
    if(iterativeDeepeningLine.nmoves > 0){
        // get the best iterativeDeepeningMove from the iterative deepening best line
        iterativeDeepeningMove = iterativeDeepeningLine.principalVariation[0];

        // remove this iterativeDeepeningMove from the iterative deepening line
        iterativeDeepeningLine.nmoves--;
        LINE newBestLine{};
        if(iterativeDeepeningLine.nmoves > 0) { // continue further along this variation
            // copy remaining moves to newbestline
            memcpy(newBestLine.principalVariation, iterativeDeepeningLine.principalVariation + 1, iterativeDeepeningLine.nmoves * sizeof(unsigned));
            newBestLine.nmoves = iterativeDeepeningLine.nmoves;
        }
        // if no further moves are left in the variation, newBestLine will be initialized with 0s, so the alphabeta
        // that is called below will ignore this and continue along as normal.

        position.doMove(iterativeDeepeningMove);

        //check if this iterativeDeepeningMove has lead to a draw (3fold rep/50move rule); as then the search can be stopped
        int score;
        if(position.isDraw()){
            position.undoMove();
            score = 0;
        }
        else {
            score = -AlphaBeta(depthLeft - 1, -beta, -alpha, &line, stats, newBestLine);
            position.undoMove();
        }

        if(score >= beta){
            stats->betaCutoffs += 1;
            return beta; // beta cutoff
        }

        if(score > alpha){
            // PV is found
            alpha = score;
            pline->principalVariation[0] = iterativeDeepeningMove;
            memcpy(pline->principalVariation + 1, line.principalVariation, line.nmoves * sizeof (unsigned));
            pline->nmoves = line.nmoves + 1;
        }
    }

    // check if the position has been evaluated before with the transposition table
    uint64_t bestMove = 0;
    Entry entry;
    uint64_t currentPositionHash = position.positionHashes[position.halfMoveNumber];

    if(TT.contains(currentPositionHash, entry)){
        stats->transpositionHits++;
        // this means that the values are useful for this search
        if(entry.depth >= depthLeft){
            switch (entry.typeOfNode) {
                case EXACT_PV:
                    if(entry.score <= alpha){
                        return alpha;
                    }
                    if(entry.score >= beta){
                        return beta;
                    }
                    pline->principalVariation[0] = entry.bestMove;
                    pline->nmoves = 1;
                    return entry.score; // return the exact score of this position
                case UPPER_BOUND_ALPHA:
                    if(entry.score < alpha){
                        return alpha; // we know for sure that the evaluation of this position will be lower than alpha,
                    }
                    break;
                case LOWER_BOUND_BETA:
                    if(entry.score >= beta){
                        return beta; // we have a lower bound, which is higher than beta
                    }
                    break;
                default:
                    cout << "Incorrect entry in TT" << "\n";
                    break;

            }
        }
        // if the depthLeft is not deep enough, or the bounds above are not strong enough, the best move can still be used
        // to improve move ordering in the search below. Note that alpha nodes have no valid best move.
        if(entry.typeOfNode != UPPER_BOUND_ALPHA) {
            bestMove = entry.bestMove;
        }
    }

    // generate list of moves
    moveList movelist;
    position.GenerateMoves(movelist);

    // check for stalemate or checkmate in case of no moves left:
    if(movelist.moveLength + movelist.captureMoveLength == 0){
        if(position.isIncheck){
            pline->nmoves = 0;
            // high score that cant be reached by evaluation function, thus indicating mate.
            // subtract depthLeft, indicating that higher depthLeft (so faster mate) is better.
            return -1000000 - depthLeft;

        }
        else{
            return 0; // stalemate
        }
    }


    /** if there was a transposition found, first evaluate that move */
    bool legal = false;
    if(bestMove != 0) {
        /** first check if this move exists (this can go wrong in case of transposition table collisions */
        for (unsigned int move : movelist.captureMove) {
            if ((uint64_t) move == bestMove) {
                legal = true;
                break;
            }
        }
        for (unsigned int move : movelist.move) {
            if ((uint64_t) move == bestMove) {
                legal = true;
                break;
            }
        }
        if(!legal){
            printf("collision handled\n");
        }
    }


    if(legal){
        stats->totalNodes += 1;
        position.doMove(bestMove);

        //check if this move has lead to a draw (3fold rep/50move rule); as then the search can be stopped
        int score;
        if(position.isDraw()){
            position.undoMove();
            score = 0;
        }
        else {
            score = -AlphaBeta(depthLeft - 1, -beta, -alpha, &line, stats, LINE());
            position.undoMove();
        }

        if(score >= beta){
            stats->betaCutoffs += 1;
            return beta; // beta cutoff
        }

        if(score > alpha){ // principal variation found
            alpha = score;
            pline->principalVariation[0] = bestMove;
            memcpy(pline->principalVariation + 1, line.principalVariation, line.nmoves * sizeof (unsigned));
            pline->nmoves = line.nmoves + 1;
        }
    }


    // next do the capture moves, as often the best move is a capture
    for(int i = 0; i < movelist.captureMoveLength; i++){
        if(movelist.captureMove[i] == bestMove || movelist.captureMove[i] == iterativeDeepeningMove){
            continue; // as this has already been evaluated above
        }
        stats->totalNodes += 1;
        position.doMove(movelist.captureMove[i]);

        // no draw can occur after a capture move due to 3fold rep/50move rule
        // so no need to check for that here

        //  invert all values for other colour
        int score = -AlphaBeta(depthLeft - 1, -beta, -alpha, &line, stats);
        position.undoMove();

        if(score >= beta){
            stats->betaCutoffs += 1;
            TT.addEntry(score, movelist.captureMove[i], depthLeft, LOWER_BOUND_BETA, position.positionHashes[position.halfMoveNumber], position.halfMoveNumber);
            return beta; // beta cutoff
        }

        if(score > alpha){ // principal variation found
            alpha = score;
            pline->principalVariation[0] = movelist.captureMove[i];
            memcpy(pline->principalVariation + 1, line.principalVariation, line.nmoves * sizeof (unsigned));
            pline->nmoves = line.nmoves + 1;
        }
    }

    // then finally the normal moves
    for(int i = 0; i < movelist.moveLength; i++){
        if(movelist.move[i] == bestMove || movelist.move[i] == iterativeDeepeningMove){
            continue; // as this has already been evaluated above
        }
        stats->totalNodes += 1;
        position.doMove(movelist.move[i]);

        //check if this move has lead to a draw (3fold rep/50move rule); as then the search can be stopped
        int score;
        if(position.isDraw()){
            position.undoMove();
            score = 0;
        }
        else {
            score = -AlphaBeta(depthLeft - 1, -beta, -alpha, &line, stats);
            position.undoMove();
        }

        if(score >= beta){
            stats->betaCutoffs += 1;
            TT.addEntry(score, movelist.move[i], depthLeft, LOWER_BOUND_BETA, position.positionHashes[position.halfMoveNumber], position.halfMoveNumber);
            return beta; // beta cutoff
        }

        if(score > alpha){ // principal variation found
            alpha = score;
            pline->principalVariation[0] = movelist.move[i];
            memcpy(pline->principalVariation + 1, line.principalVariation, line.nmoves * sizeof (unsigned));
            pline->nmoves = line.nmoves + 1;
        }
    }

    if(alpha > alphaStart) { // this means that the PV is an exact score move
        TT.addEntry(alpha, pline->principalVariation[0], depthLeft, EXACT_PV, position.positionHashes[position.halfMoveNumber],
                    position.halfMoveNumber);
    }
    else{
        // for an alpha cutoff, there is no known best move so this is left as 0.
        TT.addEntry(alpha, 0, depthLeft, UPPER_BOUND_ALPHA, position.positionHashes[position.halfMoveNumber], position.halfMoveNumber);
    }
    return alpha;
}

int Evaluate::Quiescence(int alpha, int beta, STATS *stats, int depth) {
    // define stand_pat (adapted from chessprogramming wiki quiescence search)
    int stand_pat = position.getEvaluation();

    if(depth == 0){
        return stand_pat;
    }

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
        int score = -Quiescence(-beta, -alpha, stats, depth-1);
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

void Evaluate::printinformation(int milliseconds, int score, LINE line, STATS stats, int depth) {
    string pv[100];

    std::cout << "info depth " << depth;
    std::cout << " time " << milliseconds;
    std::cout << " nodes " << stats.totalNodes;

    if(milliseconds !=0) {
        std::cout << " nps " << int(1000 * float(stats.totalNodes) / float(milliseconds));
    }
    std::cout << " score ";
    if(score >= 1000000){
        // this indicates that mate is found
        int mateIn = int((depth - score + 1000001)/2);
        std::cout << "mate " << mateIn << " pv ";
    }
    else if(score <= -1000000){
        // this indicates that the engine is getting mated
        int mateIn = -int((depth + score + 1000001)/2);
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

    std::cout << "quiescent nodes " << stats.quiescentNodes << "/" << stats.totalNodes << " total nodes\n";
    std::cout << float(stats.quiescentNodes)/stats.totalNodes << "\n";

//    std::cout << stats.totalNodes - stats.quiescentNodes << " normal nodes\n";
//
//    std::cout << "table hits: " << stats.transpositionHits << "\n";

    std::cout <<'\n';
    std::cout.flush();
}
