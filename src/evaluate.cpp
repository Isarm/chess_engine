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
#include "lookupTables.h"
#include "uci.h"

atomic_bool exitFlag(false);

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

    /** alpha and beta, with a little buffer to stop any overflowing */
    int alpha = std::numeric_limits<int>::min() + 10000;
    int beta = std::numeric_limits<int>::max() - 10000;

    int score = 0;
    for(int iterativeDepth = 1; iterativeDepth <= depth; iterativeDepth++) {
        line = {};
        while(true) {
            score = AlphaBeta(iterativeDepth, alpha, beta, &line,
                              &stats, previousBestLine);
            if((score > alpha && score < beta) || abs(score) >= 1000000){
                break;
            }
            else{
                /** widen aspiration window */
                if(score <= alpha){
                    alpha -= 100;
                }
                if(score >= beta){
                    beta += 100;
                }
            }
        }

        /** aspiration window */
        alpha = score - 50;
        beta = score + 50;

        /** if the exitFlag is set, it exited the evaluation prematurely, so take the previous best line */
        if(exitFlag.load()){
            line = previousBestLine;
            break;
        }
        previousBestLine = line;

        auto t2 = std::chrono::high_resolution_clock::now();
        int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        printinformation(milliseconds, score, line, stats, iterativeDepth);

        if(abs(score) >= 1000000) {
            // this indicates that mate is found
            break;
        }
    }
    results.bestMove = moveToStrNotation(line.principalVariation[0]);
    return results;
}

int Evaluate::AlphaBeta(int ply, int alpha, int beta, LINE *pline, STATS *stats, LINE iterativeDeepeningLine) {
    /**
     * This has become quite ugly with a lot of repetition
     */
    int alphaStart = alpha; // to be able to check if alpha has increased with this position (for tr. table)

    if(exitFlag.load()){
        return 0;
    }

    LINE line;
    if(ply == 0){
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
            score = -AlphaBeta(ply - 1, -beta, -alpha, &line, stats, newBestLine);
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
        if(entry.depth >= ply){
            switch (entry.typeOfNode) {
                case EXACT_PV:
                    if(entry.score <= alpha){
                        return alpha;
                    }
                    if(entry.score >= beta){
                        return beta;
                    }

                    position.doMove(entry.bestMove);
                    if(position.isDraw()){
                        position.undoMove();
                        break;
                    }
                    position.undoMove();
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
        // if the ply is not deep enough, or the bounds above are not strong enough, the best moves can still be used
        // to improve move ordering in the search below. Note that alpha nodes have no valid best moves.
        if(entry.typeOfNode != UPPER_BOUND_ALPHA) {
            bestMove = entry.bestMove;
        }
    }

    // generate list of moves
    moveList movelist;
    position.GenerateMoves(movelist);
    scoreMoves(movelist, ply, position.turn);
    Position::sortMoves(movelist);

    // check for stalemate or checkmate in case of no moves left:
    if(movelist.moveLength == 0){
        if(position.isIncheck){
            pline->nmoves = 0;
            // high score that cant be reached by evaluation function, thus indicating mate.
            // subtract ply, indicating that higher ply (so faster mate) is better.
            return -1000000 - ply;

        }
        else{
            return 0; // stalemate
        }
    }


    /** if there was a transposition found, first evaluate that move */
    bool legal = false;
    if(bestMove != 0) {
        /** first check if this moves exists (this can go wrong in case of transposition table collisions */
        for (pair<unsigned, int> move : movelist.moves) {
            if ((uint64_t) move.first == bestMove) {
                legal = true;
                break;
            }
        }
    }
    if(legal){
        stats->totalNodes += 1;
        position.doMove(bestMove);

        //check if this moves has lead to a draw (3fold rep/50move rule); as then the search can be stopped
        int score;
        if(position.isDraw()){
            position.undoMove();
            score = 0;
        }
        else {
            score = -AlphaBeta(ply - 1, -beta, -alpha, &line, stats, LINE());
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


    // then the normal moves
    for(int i = 0; i < movelist.moveLength; i++){
        if(exitFlag.load()){
            return 0;
        }
        if(movelist.moves[i].first == bestMove || movelist.moves[i].first == iterativeDeepeningMove){
            continue; // as this has already been evaluated above
        }
        stats->totalNodes += 1;
        position.doMove(movelist.moves[i].first);

        //check if this moves has lead to a draw (3fold rep/50move rule); as then the search can be stopped
        int score;
        bool drawFlag = false;
        if(position.isDraw()){
            position.undoMove();
            drawFlag = true;
            score = 0;
        }
        else {
            score = -AlphaBeta(ply - 1, -beta, -alpha, &line, stats);
            position.undoMove();
        }

        if(score >= beta){
            stats->betaCutoffs += 1;
            if(!drawFlag) {
                TT.addEntry(score, movelist.moves[i].first, ply, LOWER_BOUND_BETA,
                            position.positionHashes[position.halfMoveNumber], position.halfMoveNumber);
                addKillerMove(ply, movelist.moves[i].first);
                if(!(movelist.moves[i].first & CAPTURE_MOVE_FLAG_MASK)){
                    updateButterflyTable(ply, movelist.moves[i].first, position.turn);
                }
            }
            return beta; // beta cutoff
        }

        if(score > alpha){ // principal variation found
            alpha = score;
            pline->principalVariation[0] = movelist.moves[i].first;
            memcpy(pline->principalVariation + 1, line.principalVariation, line.nmoves * sizeof (unsigned));
            pline->nmoves = line.nmoves + 1;
        }
    }
    if(exitFlag.load()){
        return 0;
    }

    if(alpha > alphaStart) { // this means that the PV is an exact score moves
        if(alpha != 0) { // if alpha is 0 there are some weird draw variations that you do not want in your TT
            TT.addEntry(alpha, pline->principalVariation[0], ply, EXACT_PV,
                        position.positionHashes[position.halfMoveNumber],
                        position.halfMoveNumber);
        }
    }
    else{
        // for an alpha cutoff, there is no known best moves so this is left as 0.
        TT.addEntry(alpha, 0, ply, UPPER_BOUND_ALPHA, position.positionHashes[position.halfMoveNumber], position.halfMoveNumber);
    }

    return alpha;
}

int Evaluate::Quiescence(int alpha, int beta, STATS *stats, int depth) {
    if(exitFlag.load()){
        return 0;
    }


    if(depth == 0){
        return position.getEvaluation();
    }

    // define stand_pat (adapted from chessprogramming wiki quiescence search)
    // first do lazy evaluation (with a safety margin of 50 centipawns)
    int stand_pat = position.getLazyEvaluation();

    if(stand_pat >= beta + 50){
        return beta;
    }

    if(stand_pat < alpha - 960){
        return alpha;
    }

    // do proper eval
    stand_pat = position.getEvaluation();


    if(stand_pat >= beta){
        return beta;
    }

    if(stand_pat < alpha - 910){
        return alpha;
    }

    if(alpha < stand_pat){
        alpha = stand_pat; // raise alpha to establish lower bound on postion
    }

    moveList movelist;
    position.GenerateMoves(movelist);

    // go over all capture moves to avoid horizon effect on tactical moves.
    for(int i = 0; i < movelist.moveLength; i++){
        if(!(movelist.moves[i].first & CAPTURE_MOVE_FLAG_MASK)){
            continue;
        }
        stats->quiescentNodes += 1;
        stats->totalNodes += 1;
        position.doMove(movelist.moves[i].first);
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


void Evaluate::scoreMoves(moveList &list, int ply, bool side) {
    for(int i = 0; i < list.moveLength; i++){
        if(isKiller(ply, list.moves[i].first)){
            list.moves[i].second += KILLER_BONUS;
        }
        list.moves[i].second += getButterflyScore(ply, list.moves[i].first, side);

    }
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



inline void Evaluate::addKillerMove(unsigned ply, unsigned move){
    /** shift old killer moves */
    for (int i = KILLER_MOVE_SLOTS - 2; i >= 0; i--)
        killerMoves[ply][i + 1] = killerMoves[ply][i];
    /** add new */
    killerMoves[ply][0] = move;
}

inline bool Evaluate::isKiller(unsigned ply, unsigned move){
    for(unsigned &kmove : killerMoves[ply]){
        if((kmove & (ORIGIN_SQUARE_MASK | DESTINATION_SQUARE_MASK)) == (move & (ORIGIN_SQUARE_MASK | DESTINATION_SQUARE_MASK))){
            return true;
        }
    }
    return false;
}


inline void Evaluate::updateButterflyTable(unsigned ply, unsigned move, bool side){
    unsigned from = (move & ORIGIN_SQUARE_MASK) >> ORIGIN_SQUARE_SHIFT;
    unsigned to = (move & DESTINATION_SQUARE_MASK) >> DESTINATION_SQUARE_SHIFT;

    butterflyTable[side][from][to] += ply * ply;
}

inline unsigned Evaluate::getButterflyScore(unsigned ply, unsigned move, bool side){
    unsigned from = (move & ORIGIN_SQUARE_MASK) >> ORIGIN_SQUARE_SHIFT;
    unsigned to = (move & DESTINATION_SQUARE_MASK) >> DESTINATION_SQUARE_SHIFT;

    return butterflyTable[side][from][to];
}
