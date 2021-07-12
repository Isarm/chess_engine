//
// Created by isar on 12/07/2021.
//

#include <chrono>
#include "benchmarks.h"
#include "useful.h"
#include "position.h"
#include "definitions.h"

unsigned totaltime = 0;

void benchmarkDeBruijn(){
    printf("debruijn:\n");
    uint64_t bb = 283742837;
    auto t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 10000000; i++){
        bb += 12;
        debruijnSerialization(bb);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    printf("%i\n", milliseconds);
}


void benchmarkMoveGen(){
    /**
    Position position = Position("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ");

    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000000; ++i) {
        moveList movelist;
        position.GeneratePawnMoves(movelist);
    }
    auto t2 = std::chrono::high_resolution_clock::now();

    int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    printf("pawns:\n%i\n", milliseconds);


    moveList movelist;
    position.GeneratePawnMoves(movelist);

    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000000; ++i) {
        position.doMove(movelist.moves[0].first);
        position.undoMove();
    }
    t2 = std::chrono::high_resolution_clock::now();

    milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    printf("doandundo:\n%i\n", milliseconds);

    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000000; ++i) {
        moveList movelist;
        position.GenerateKnightMoves(movelist);
    }
    t2 = std::chrono::high_resolution_clock::now();

    milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    printf("knights:\n%i\n", milliseconds);

    movelist = {};
    position.GenerateKnightMoves(movelist);

    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000000; ++i) {
        position.doMove(movelist.moves[0].first);
        position.undoMove();
    }
    t2 = std::chrono::high_resolution_clock::now();

    milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    printf("doandundo:\n%i\n", milliseconds);

    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000000; ++i) {
        moveList movelist;
        position.GenerateSliderMoves(movelist);
    }
    t2 = std::chrono::high_resolution_clock::now();

    milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    printf("sliders:\n%i\n", milliseconds);

    movelist = {};
    position.GenerateSliderMoves(movelist);

    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000000; ++i) {
        position.doMove(movelist.moves[0].first);
        position.undoMove();
    }
    t2 = std::chrono::high_resolution_clock::now();

    milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    printf("doandundo:\n%i\n", milliseconds);


    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000000; ++i) {
        moveList movelist;
        position.GenerateKingMoves(movelist);
    }
    t2 = std::chrono::high_resolution_clock::now();

    milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    printf("king:\n%i\n", milliseconds);


    movelist = {};
    position.GenerateKingMoves(movelist);

    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000000; ++i) {
        position.doMove(movelist.moves[0].first);
        position.undoMove();
    }
    t2 = std::chrono::high_resolution_clock::now();

    milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    printf("doandundo:\n%i\n", milliseconds);


    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000000; ++i) {
        moveList movelist;
        position.GenerateCastlingMoves(movelist);
    }
    t2 = std::chrono::high_resolution_clock::now();

    milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    printf("castling:\n%i\n", milliseconds);


    movelist = {};
    position.GenerateCastlingMoves(movelist);

    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000000; ++i) {
        position.doMove(movelist.moves[0].first);
        position.undoMove();
    }
    t2 = std::chrono::high_resolution_clock::now();

    milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    printf("doandundo:\n%i\n", milliseconds);


    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000000; ++i) {
        moveList movelist;
    }
    t2 = std::chrono::high_resolution_clock::now();

    milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    printf("movelist:\n%i\n", milliseconds);

    printf("bbtolegalmovelist:\n%i\n", totaltime);
    */
}