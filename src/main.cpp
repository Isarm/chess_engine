#include <iostream>

using namespace std;

#include "uci.h"

bool debug = 1;

int main(int argc, char *argv[]) {
    if(!debug) {
        cout << "Engine initialized \n";

        // get first input
        string input;
        getline(cin, input);

        if (input == "uci") {
            UCI::start();
        } else {
            return 1;
        }

        UCI::mainLoop();
    }

    UCI::mainLoop();

}

