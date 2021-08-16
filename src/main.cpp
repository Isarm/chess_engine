#include <iostream>

using namespace std;

#include "uci.h"
#include "definitions.h"
#include "tests.h"


//#define DEBUG 1


int main(int argc, char *argv[]) {
#ifndef DEBUG
    cout << "Engine initialized v2.10\n";

    // get first input
    string input;
    getline(cin, input);

    UCI uci = UCI();

    if (input == "uci") {
        uci.start();
    }
#else
    testmain(argc, argv);
#endif
}



