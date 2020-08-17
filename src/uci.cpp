
#include <iostream>
#include "uci.h"
#include "position.h"
#include "moveGenHelpFunctions.h"

using namespace std;

void UCI::start() {

    // output standard info
    cout << "id name Isar Engine\n";
    cout << "id author Isar Meijer\n";

    // uci is ready
    cout << "uciok\n";

    // wait for the isready command
    string input;
    do {
        getline(cin, input);
    } while (input != "isready");

    // give the ready signal
    cout << "readyok\n";

}


void UCI::mainLoop(){



    string pinnedPiecesTest = "2q5/8/6b1/2B5/8/3R4/RrKN2r1/8 w - - 0 1";


    Position position = Position(pinnedPiecesTest);
    moveList movelist;
    position.prettyPrint();
    position.GenerateMoves(movelist);
    position.prettyPrint();

    position.doMove(movelist.move[0]);

    position.prettyPrint();

}
