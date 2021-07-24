#ifndef UCI_H
#define UCI_H

#include <vector>
#include <atomic>
#include <chrono>
#include "definitions.h"
#include "threadManager.h"


class UCI {
public:
    ThreadManager threadManager = ThreadManager(Settings());

    void mainLoop();

    void start();

    void go(string fen, vector<string> moves, definitions::Results &results);

    void timer(int ms);

    UCI();
};// namespace UCI


#endif

