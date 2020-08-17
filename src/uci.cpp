
#include <iostream>
#include "uci.h"
#include "position.h"
#include <chrono>

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
    string blacktoplay = "1kq5/8/r5b1/2B5/8/3R4/R1KN2r1/8 b - - 0 1";
    string startP2 = "rnbqkbnr/pppppppp/8/8/P7/8/1PPPPPPP/RNBQKBNR b KQkq - 0 1";

    Position position = Position("startpos");
    position.prettyPrint();


    auto t1 = std::chrono::high_resolution_clock::now();
    perftCounts pfcount = position.PERFT(6);
    auto t2 = std::chrono::high_resolution_clock::now();

    cout << "\n\n" << "Nodes: " << pfcount.total << "\n";
    cout << "Normal: " << pfcount.normal << "\n";
    cout << "Capture: " << pfcount.captures << "\n";

    int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();

    cout << "Time: "
              << milliseconds
              << " milliseconds\n";

    cout << "Nodes/s: "  << pfcount.total / milliseconds * 1000 << "\n";

}
