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
#include "uci.h"
#include "threadManager.h"
#include "bitboard.h"


Evaluate::Evaluate() = default;

Evaluate::Evaluate(string fen, vector<string> moves) {
    this->position = Position(fen);

    // do the moves that are given by the UCI protocol to update the position
    for(string move : moves){
        this->position.doMove(move);
    }
}


int Evaluate::AlphaBeta(int ply, int alpha, int beta, LINE *pline, STATS *stats, int depth, LINE iterativeDeepeningLine) {
    /**
     * This has become quite ugly with a lot of repetition
     */
    int alphaStart = alpha; // to be able to check if alpha has increased with this position (for tr. table)

    if(exitCondition()){
        return 0;
    }

    stats->totalNodes++;

    if(ply == 0){
        pline->nmoves = 0;
        // do a quiescent search for the leaf nodes, to reduce the horizon effect
        return Quiescence(alpha, beta, stats, false, depth);
    }

    unsigned iterativeDeepeningMove = 0;
    if(iterativeDeepeningLine.nmoves > depth){
        // get the iterativeDeepeningMove from the iterative deepening line
        iterativeDeepeningMove = iterativeDeepeningLine.principalVariation[depth];
    }


    // check if the position has been evaluated before with the transposition table
    uint64_t bestMove = 0;
    Entry entry;
    uint64_t currentPositionHash = position.getPositionHash();

    if(TT.contains(currentPositionHash, entry)){
        /** The halfmovenumber should be the same, otherwise it will have trouble detecting 3fold repetitions */
        if(entry.age == position.halfMoveNumber) {
            stats->transpositionHits++;
            // this means that the values are useful for this search
            if (entry.depth >= ply) {
                switch (entry.typeOfNode) {
                    case EXACT_PV:
                        if (entry.score <= alpha) {
                            return alpha;
                        }
                        if (entry.score >= beta) {
                            return beta;
                        }

                        position.doMove(entry.bestMove);
                        if (position.isDraw()) {
                            position.undoMove();
                            break;
                        }
                        position.undoMove();
                        pline->principalVariation[0] = entry.bestMove;
                        pline->nmoves = 1;
                        return entry.score; // return the exact score of this position
                    case UPPER_BOUND_ALPHA:
                        if (entry.score < alpha) {
                            return alpha; // we know for sure that the evaluation of this position will be lower than alpha,
                        }
                        break;
                    case LOWER_BOUND_BETA:
                        if (entry.score >= beta) {
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
            if (entry.typeOfNode != UPPER_BOUND_ALPHA) {
                bestMove = entry.bestMove;
            }
        }
    }


    /** Null move pruning */
    int R = 3; // TODO: Zugzwang

    if(!position.isIncheck && position.getLazyEvaluation() >= beta - 50){
        position.doNullMove();
        LINE line {};
        int score = -AlphaBeta(max(0, ply - 1 - R), -beta, -beta + 1, &line, stats, depth + 1);
        position.undoNullMove();
        if(score >= beta){
            return beta;
        }
    }
    // generate list of moves
    moveList movelist;
    position.GenerateMoves(movelist);
    scoreMoves(movelist, ply, depth, position.turn, bestMove, iterativeDeepeningMove);
    Position::sortMoves(movelist);

    // check for stalemate or checkmate in case of no moves left:
    if(movelist.moveLength == 0){
        if(position.isIncheck){
            pline->nmoves = 0;
            // high score that cant be reached by evaluation function, thus indicating mate.
            // add depth, indicating that lower depth (so faster mate) is better.
            return -1000000 + depth;

        }
        else{
            return 0; // stalemate
        }
    }



    /** Search for principal variation if alpha has not yet been increased */
    bool searchPV = alphaStart == alpha;

    /** Go through the movelist */
    for(int i = 0; i < movelist.moveLength; i++){
        /** Futility pruning */
        bool fprune = ply == 1 && i > 3 && !position.isIncheck;
        if(fprune && position.getLazyEvaluation() + movelist.moves[i].second + 80 < alpha){
            continue;
        }

        LINE line {};
        int score;
        position.doMove(movelist.moves[i].first);

        //check if this moves has lead to a draw (3fold rep/50move rule); as then the search can be stopped
        bool drawFlag = false;
        if(position.isDraw()){
            position.undoMove();
            drawFlag = true;
            score = 0;
        }
        else {
            /** Principal variation search */
            if(searchPV || position.isIncheck){
                score = -AlphaBeta(ply - 1, -beta, -alpha, &line, stats, depth + 1);
            }else{
                /** Late move reductions */
                int LMR = 0;
                if (i > 3 && depth > 2 && !(movelist.moves[i].first & CAPTURE_MOVE_FLAG_MASK)){
                    if(i < 6){
                        LMR = 1;
                    } else{
                        LMR = i / 3;
                    }
                }
                /** Null window search */
                score = -AlphaBeta(max(0, ply - 1 - LMR), -alpha - 1, -alpha, &line, stats, depth + 1);
                if(score > alpha){
                    line = LINE();
                    /** Do a research with the full window and depth if the null window ends above alpha*/
                    score = -AlphaBeta(ply - 1, -beta, -alpha, &line, stats, depth + 1);
                }
            }
            position.undoMove();
        }

        if(score >= beta){
            stats->betaCutoffs += 1;
            if(!drawFlag) {
                TT.addEntry(score, movelist.moves[i].first, ply, LOWER_BOUND_BETA,
                            position.getPositionHash(), position.halfMoveNumber);
                if(!(movelist.moves[i].first & CAPTURE_MOVE_FLAG_MASK)){
                    addKillerMove(depth, movelist.moves[i].first);
                    updateButterflyTable(ply, movelist.moves[i].first, position.turn);
                }
            }
            return beta; // beta cutoff
        }

        if(score > alpha){ // principal variation found
            searchPV = false;
            alpha = score;
            pline->principalVariation[0] = movelist.moves[i].first;
            memcpy(pline->principalVariation + 1, line.principalVariation, line.nmoves * sizeof (unsigned));
            pline->nmoves = line.nmoves + 1;
        }
    }

    if(alpha > alphaStart) { // this means that the PV is an exact score moves
        if(alpha != 0) { // if alpha is 0 there are some weird draw variations that you do not want in your TT
            TT.addEntry(alpha, pline->principalVariation[0], ply, EXACT_PV,
                        position.getPositionHash(),
                        position.halfMoveNumber);
        }
    }
    else{
        // for an alpha cutoff, there is no known best moves so this is left as 0.
        TT.addEntry(alpha, 0, ply, UPPER_BOUND_ALPHA, position.getPositionHash(), position.halfMoveNumber);
    }

    return alpha;
}

int Evaluate::Quiescence(int alpha, int beta, STATS *stats, bool evasion, int depth) {
    if(exitCondition()){
        return 0;
    }
    stats->quiescentNodes++;
    stats->totalNodes++;

    bool check = position.isIncheck;

    // define stand_pat (adapted from chessprogramming wiki quiescence search)
    // first do lazy evaluation (with a safety margin of 150 centipawns)
    int stand_pat = position.getLazyEvaluation();

    if(stand_pat >= beta + 50 && !check){
        return beta;
    }

    if(stand_pat < alpha - 960){
        return alpha;
    }

    // do proper eval
    stand_pat = position.getEvaluation();

    if(stand_pat >= beta && !check){
        return beta;
    }

    if(stand_pat < alpha - 910){
        return alpha;
    }

    if(alpha < stand_pat){
        alpha = stand_pat; // raise alpha to establish lower bound on position
    }

    moveList movelist;
    /** if it is not an evasion or check, only evaluate captures, otherwise evaluate all moves */
    position.GenerateMoves(movelist, !check);
    Position::sortMoves(movelist);

    // check for checkmate in case of no moves left and if the side was evading
    if(movelist.moveLength == 0 && position.isIncheck){
        return -1000000 + depth;
    }

    // go over all ((good) capture) moves to avoid horizon effect on tactical moves.
    for(int i = 0; i < movelist.moveLength; i++){
        /** Prune moves with bad SEE */
        if(movelist.moves[i].second < 0 && !check){
            break;
        }

        /** Check SEE */
        int SEE = 0;
        if(movelist.moves[i].second > CAPTURE_SCORE){
            SEE = movelist.moves[i].second - CAPTURE_SCORE;
        }
        /** If stand pat is below alpha - safety margin - SEE then return early
         * Note that SEE will only decrease with the next moves in the movelist. */
        if(stand_pat < alpha - SEE - 50 && !check){
            return alpha;
        }

        position.doMove(movelist.moves[i].first);
        int score = -Quiescence(-beta, -alpha, stats, false, depth + 1);
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


void Evaluate::scoreMoves(moveList &list, int ply, int depth, bool side, uint64_t bestMove, uint64_t iterativeDeepeningMove) {
    for(int i = 0; i < list.moveLength; i++){
        if(list.moves[i].first == iterativeDeepeningMove){
            list.moves[i].second += 3000000;
        }
        if(list.moves[i].first == bestMove){
            list.moves[i].second += 2000000;
        }
        if(isKiller(depth, list.moves[i].first)){
            list.moves[i].second += KILLER_BONUS;
        }
        list.moves[i].second += getButterflyScore(ply, list.moves[i].first, side);
    }
}


inline void Evaluate::addKillerMove(unsigned depth, unsigned move){
    /** shift old killer moves */
    for (int i = KILLER_MOVE_SLOTS - 2; i >= 0; i--)
        killerMoves[depth][i + 1] = killerMoves[depth][i];
    /** add new */
    killerMoves[depth][0] = move;
}

inline bool Evaluate::isKiller(unsigned depth, unsigned move){
    for(unsigned &kmove : killerMoves[depth]){
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
