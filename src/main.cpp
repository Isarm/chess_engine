#include <iostream>

using namespace std;

#include "uci.h"
#include "tests.h"

bool debug = 0;

int main(int argc, char *argv[]) {
    if(!debug) {
        cout << "Engine initialized v1.1002\n";

        // get first input
        string input;
        getline(cin, input);

        if (input == "uci") {
            UCI::start();
        }

    }
    else{
        testmain(argc, argv);
    }
}



