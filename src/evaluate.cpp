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
    // add min+1 and max-1 because otherwise overflow occurs when inverting
    AlphaBeta(depth, std::numeric_limits<int>::min() + 1, std::numeric_limits<int>::max() - 1, &line);


    string pv[100];

    for(int i = 0; i < line.nmoves; i++){
        pv[i] = moveToStrNotation(line.principalVariation[i]);
        std::cout << pv[i] << "\n";
        std::cout.flush();
    }

    results.bestMove = pv[0];
    return results;
}

int Evaluate::AlphaBeta(int depth, int alpha, int beta, LINE *pline){
    LINE line;
    if(depth == 0){
        pline->nmoves = 0;
        // do a quiescent search for the leaf nodes, to try and reduce the horizon effect (TODO)
        return Quiescence();
    }

    // generate list of moves
    moveList movelist;
    position.GenerateMoves(movelist);

    // first do the capture moves
    for(int i = 0; i < movelist.captureMoveLength; i++){
        position.doMove(movelist.captureMove[i]);
        int score = -AlphaBeta(depth - 1, -beta, -alpha, &line);
        position.undoMove();

        if(score >= beta){
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
        position.doMove(movelist.move[i]);
        int score = -AlphaBeta(depth - 1, -beta, -alpha, &line);
        position.undoMove();

        if(score >= beta){
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

int Evaluate::Quiescence(){
    return this->position.Evaluate();
}