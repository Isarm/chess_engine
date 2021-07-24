//
// Created by isar on 24/07/2021.
//

#include "thread.h"
#include "definitions.h"


Thread::Thread(int threadID){
    id = threadID;
}

void Thread::go(Evaluate eval, SearchParams params) const {
    if(!(id % 2)){
        params.ply++;
    }
    LINE line{};
    STATS stats{};
    eval.AlphaBeta(params.ply, params.alpha, params.beta, &line, &stats, params.iterativeDeepeningLine);
}





