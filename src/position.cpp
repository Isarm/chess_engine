
#include <position.h>

#include "position.h"
#include "dictionaries.h"
#include "cstring"

using namespace std;

Position::Position(string FEN){

    string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    if(FEN == "startpos") FEN = startFEN;

    // initialize bitboards to 0.
    for(int i = 0; i < 12; i++){
        bitboards[i] = 0;
    }

    int i = 0;
    char key = FEN.at(i);
    while(key != ' ') {
        key = FEN.at(i);
        bitboards[pieces.at(key), i] = 1;
    }




}


