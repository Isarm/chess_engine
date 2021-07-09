//
// Created by isar on 09/07/2021.
//

#ifndef ENGINE_EXITTIMER_H
#define ENGINE_EXITTIMER_H

#include <thread>

void timerLoop(unsigned int timems);

class ExitTimer {
private:
    std::thread timerLoopThread;
    std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<int64_t, std::ratio<1, 1000000000>>> t1;
    unsigned int diff = 5000;
public:
    void startTimer(unsigned int timems = 5000);
    bool check_timer();
};



#endif //ENGINE_EXITTIMER_H
