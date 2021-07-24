#include <iostream>

using namespace std;

#include "uci.h"
#include "tests.h"

bool debug = 0;

int main(int argc, char *argv[]) {
    if(!debug) {
        cout << "Engine initialized v1.400\n";

        // get first input
        string input;
        getline(cin, input);

        UCI uci = UCI();
        if (input == "uci") {
            uci.start();
        }

    }
    else{

        testmain(argc, argv);
    }
}



