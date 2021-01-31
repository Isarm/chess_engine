//
// Created by isar on 31/01/2021.
//

#include <string>
#include <vector>
#include <position.h>
#include "evaluate.h"
#include "definitions.h"

Evaluate::Evaluate(string fen, vector<string> moves, Settings settings) {
    this->position = Position(fen);
    this->depth = settings.depth;


    for(string move : moves){
        this->position.doMove(move);
    }
}

Results Evaluate::Start(){
    Results results;
    results.bestMove = "e2e4";

    return results;
}