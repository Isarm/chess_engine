//
// Created by isar on 09/07/2021.
//

#include "exitTimer.h"
#include "evaluate.h"
#include "atomic"

void timerLoop(unsigned int timems){
    ExitTimer exitTimer;

    exitTimer.startTimer(timems);

    while(!exitTimer.check_timer() && !exitFlag.load()){
    }
}


void ExitTimer::startTimer(unsigned int timems) {
    t1 = std::chrono::high_resolution_clock::now();
    diff = timems;
    exitFlag.store(false);
}



bool ExitTimer::check_timer() {
    auto t2 = std::chrono::high_resolution_clock::now();
    int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    if(milliseconds > diff){
        exitFlag.store(true);
        return true;
    }
    return false;
}
