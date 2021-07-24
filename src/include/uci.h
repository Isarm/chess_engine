#ifndef UCI_H
#define UCI_H

#include <vector>
#include <atomic>
#include <chrono>
#include "definitions.h"
#include "threadManager.h"


class UCI {
public:
    ThreadManager threadManager;
    Settings settings;

    void mainLoop();

    UCI();

    void start();

};// namespace UCI


#endif

