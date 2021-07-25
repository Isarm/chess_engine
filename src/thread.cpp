//
// Created by isar on 24/07/2021.
//

#include "thread.h"
#include "definitions.h"
#include "threadManager.h"
#include "mutex"
#include "useful.h"



Thread::Thread(int threadID){
    id = threadID;
}

void Thread::search() {
    LINE IDline {};
    copyline(&params.iterativeDeepeningLine, &IDline);
    int alpha = params.alpha; int beta = params.beta;
    int ply = params.ply;

    if(!(id % 2)){
        ply++;
    }

    LINE line{};
    STATS stats{};
    evaluate.AlphaBeta(ply, alpha, beta, &line, &stats, IDline);
}

void Thread::idleLoop() {
    /** Get position information */
    positionMutex.lock();
    string fen = mutexFen;
    vector<string> moves;
    for(string &move : mutexMoves){
        moves.push_back(move);
    }
    positionMutex.unlock();
    evaluate = Evaluate(fen, moves);

    /** Idle loop */
    while(true) {
        if(startID[id]){
            startID[id] = false;
            search();
        }
        if (killThreads){
            break;
        }
    }
}





