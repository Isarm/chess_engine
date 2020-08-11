#include <iostream>

using namespace std;

#include "uci.h"

int main(int argc, char *argv[]) {

    cout << "Engine initialized \n";

    // get first input 
    string input;
    getline(cin, input);

    if (input == "uci") {
        UCI::start();
    }
    else{
         return 1;
    }


}

