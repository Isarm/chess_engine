#ifndef UCI_H
#define UCI_H

#include <vector>
#include <atomic>
#include <chrono>
#include "definitions.h"



namespace UCI {

    void mainLoop();

    void start();

    void perft();

    void go(string fen, vector<string> moves, definitions::Settings settings, definitions::Results &results);

    void timer();
}// namespace UCI


#endif

