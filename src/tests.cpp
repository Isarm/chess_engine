//
// Created by isar on 18/08/2020.
//

#include "position.h"
#include "tests.h"
#include "definitions.h"
#include <chrono>
#include "useful.h"
#include "benchmarks.h"
#include "bitboard.h"

void drawTest();

void hashTest();

void benchmarks();

void fileAndStructureTest();

void testFrontSpans();

void testPawnStructure();

void testKingPawnShield();

void testMirror();

using namespace std;


void testmain(int argc, char *argv[]){
    perft(argc, argv);
//    perftDebug();
//    drawTest();
//    hashTest();
//    benchmarks();
//    fileAndStructureTest();
//    testFrontSpans();
//    testKingPawnShield();
//    testPawnStructure();
//    testMirror();
}

void testMirror() {
    Position position = Position("rnbqk2r/ppp2ppp/3pp3/P1PnP3/3P4/P7/4KPPP/RNBQ1BNR b kq - 0 1");
    position.prettyPrint();
    printf("%i\n", position.getLazyEvaluation());
    printf("%i\n\n", position.getFullEvaluation());

    position.mirror();
    position.prettyPrint();
    printf("%i\n", position.getLazyEvaluation());
    printf("%i\n\n", position.getFullEvaluation());
}

void testKingPawnShield() {
    LookupTables lut = LookupTables();
    for(int i = 0; i < 64; i++){
        printf("WHITE:\n");
        Bitboard::print(lut.kingPawnShield[WHITE][i]);
        printf("BLACK:\n");
        Bitboard::print(lut.kingPawnShield[BLACK][i]);
    }
}

void testPawnStructure() {
    Position position = Position("4k3/ppp1p3/p4pP1/6P1/8/2P3p1/PPP1P1pP/4K3 w - - 0 1");
    position = Position("4R3/8/p2r1k2/1p2pp2/2p5/2P1K3/2P5/8 w - - 0 1");
    /** Walk through this with debugger */
    printf("%i\n", position.getLazyEvaluation());
}

void testFrontSpans() {
    LookupTables lut = LookupTables();
    for(int i = 0; i < 64; i++){
        printf("WHITE:\n");
        Bitboard::print(lut.frontSpans[WHITE][i]);
        printf("BLACK:\n");
        Bitboard::print(lut.frontSpans[BLACK][i]);
    }
}

void fileAndStructureTest() {
    Position position = Position("4k3/ppp1p1pp/8/4P3/1P2P2P/P1P1P1PP/8/4KR1R w K - 0 1");
    position.getLazyEvaluation();

}

void benchmarks() {

    benchmarkDeBruijn();
    benchmarkMoveGen();

}

void hashTest() {
    Position position = Position("rnbqkbnr/p1pppppp/8/Pp6/8/1P6/2PPPPPP/RNBQKBNR w KQkq b6 0 1");
    moveList movelist;
    position.GenerateMoves(movelist);
}


void drawTest() {
    Position position = Position("pppppppp/pppppppp/pppppppp/pppppp2/pppppp2/2ppp3/2p3pk/K3Q3 w - - 0 1");
    string moves[] = {"e1h4", "h2g1", "h4e1", "g1h2", "e1h4", "h2g1", "h4e1", "g1h2", "e1h4", "h2g1", "h4e1", "g1h2", "e1h4", "h2g1", "h4e1", "g1h2"};
    for(string move : moves){
        position.doMove(move);
    }
    position.undoMove();
    position.undoMove();
    position.doMove("h4f4");
    position.undoMove();
    position.doMove("h4e1");
}

// function that keeps printing moves and waits for moves as input, such that you can manually traverse the search tree
[[noreturn]] void perftDebug(){
    Position position = Position("8/8/7k/1Pp5/8/8/1K6/8 w - c6 0 1");

    while(true){
        position.prettyPrint();
        position.PERFT(2, true);
        moveList movelist;
        position.prettyPrint();
        position.GenerateMoves(movelist);

        string move;
        std::cin >> move;

        position.doMove(move);
    }
}

// common perft test positions function
void perft(int argc, char *argv[]){

    int DEPTH;
    string posString;


    if(argc > 1){
        DEPTH = stoi(argv[1]);
        posString = argv[2];
    }
    else { // manual perft setup
        DEPTH = 5;
        int positionN = 1;
        // choose type of position (these are common PERFT test positions)
        switch (positionN) {
            case 1:
                posString = "startpos";
                break;
            case 2:
                posString = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0";
                break;
            case 3:
                posString = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
                break;
            case 4:
                posString = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
                break;
            case 5:
                posString = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ";
                break;
            case 6 :
                posString = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ";
                break;
            case 7:
                posString = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
                break;
            case 8:
                posString = "3r3q/q7/8/2BRB3/r1RKR1rk/2BRB3/8/q5q1 w - - 0 1";
                break;
            default:
                posString = "startpos";
                break;
        }

    }

    Position position = Position(posString);


    if(argc >= 4) {
        std::string moveList = argv[3];
        std::string singleMove;
        for (char i : moveList) {
            if (i != ' ') {
                singleMove.push_back(i);
            } else {
                position.doMove(singleMove);
                singleMove = "";
            }
        }
        position.doMove(singleMove);
    }


    auto t1 = std::chrono::high_resolution_clock::now();
    perftCounts pfcount = position.PERFT(DEPTH);
    auto t2 = std::chrono::high_resolution_clock::now();

    cout << "\n" << pfcount.total << "\n";

    if(argc == 1) {
        cout << "\n\nPerft " << DEPTH << "\n";
        cout << "Perft String:" << posString << "\n";
        cout << "Nodes: " << pfcount.total << "\n";
        cout << "Normal: " << pfcount.normal << "\n";

        int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        cout << "Time: "
             << milliseconds
             << " milliseconds\n";

        cout << "Nodes/s: " << pfcount.total / milliseconds * 1000 << "\n";
    }
}