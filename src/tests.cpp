//
// Created by isar on 18/08/2020.
//

#include "position.h"
#include "tests.h"
#include "definitions.h"
#include <chrono>

using namespace std;


void testmain(){
    perftDebug();
}


[[noreturn]] void perftDebug(){
    Position position = Position("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");

    while(true){
        position.prettyPrint();
        position.PERFT(2, true);
        moveList movelist;
        position.prettyPrint();
        position.GenerateMoves(movelist);

        int movenr;
        std::cin >> movenr;

        position.doMove(movelist.move[movenr]);
    }
}


void perft(){

    int DEPTH = 2;
    int positionN = 3;

    string posString;

    switch (positionN) {
        case 1:
            posString = "startpos";
            break;
        case 3:
            posString = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1" ;
            break;
        default:
            posString = "startpos";
            break;
    }



    Position position = Position(posString);

    auto t1 = std::chrono::high_resolution_clock::now();
    perftCounts pfcount = position.PERFT(DEPTH);
    auto t2 = std::chrono::high_resolution_clock::now();

    cout << "\n\nPerft " << DEPTH << "\n";
    cout << "Nodes: " << pfcount.total << "\n";
    cout << "Normal: " << pfcount.normal << "\n";
    cout << "Capture: " << pfcount.captures << "\n";

    int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();

    cout << "Time: "
         << milliseconds
         << " milliseconds\n";

    cout << "Nodes/s: "  << pfcount.total / milliseconds * 1000 << "\n";
}