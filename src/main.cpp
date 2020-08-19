#include <iostream>

using namespace std;

#include "uci.h"
#include "tests.h"

bool debug = 1;

int main(int argc, char *argv[]) {
    if(!debug) {
        cout << "Engine initialized \n";

        // get first input
        string input;
        getline(cin, input);

        if (input == "uci") {
            UCI::start();
        }

        UCI::mainLoop();
    }
    else{
        testmain(argc, argv);
    }

}



