#ifndef UCI_H
#define UCI_H

#include <vector>
#include "definitions.h"

namespace UCI {

    void mainLoop();

    void start();

    void perft();

    void go(string fen, vector<string> moves, definitions::Settings settings, definitions::Results &results);
}// namespace UCI


#endif

