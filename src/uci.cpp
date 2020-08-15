
#include <iostream>
#include "uci.h"
#include "position.h"

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

    Position position = Position("rnbqkbnr/pppppppp/8/8/8/8/P6P/RNBQKBNR w KQkq - 0 1");
    moveList movelist;
    position.GeneratePseudoLegalMoves(movelist);
    position.doMove(movelist.move[0]);


}
