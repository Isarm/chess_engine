//
// Created by isar on 09/07/2021.
//

#include "exitTimer.h"
#include "search.h"
#include "atomic"
#include "threadManager.h"

void timerLoop(unsigned int timems){
    ExitTimer exitTimer;

    exitTimer.startTimer(timems);

    while(!exitTimer.check_timer() && !timerFlag.load()){
    }
}


void ExitTimer::startTimer(unsigned int timems) {
    t1 = std::chrono::high_resolution_clock::now();
    diff = timems;
    timerFlag.store(false);
}



bool ExitTimer::check_timer() {
    auto t2 = std::chrono::high_resolution_clock::now();
    int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    if(milliseconds > diff){
        timerFlag.store(true);
        return true;
    }
    return false;
}
