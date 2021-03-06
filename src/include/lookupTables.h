//
// Created by isar on 07/02/2021.
//


#ifndef ENGINE_ZOBRISTTABLES_H


#include <cstdint>
#include <random>
#include "definitions.h"
#include "useful.h"

using namespace definitions;

extern uint64_t zobristPieceTable[2][6][64];
extern uint64_t zobristCastlingRightsTable[16];
extern uint64_t zobristBlackToMove;
extern uint64_t zobristEnPassantFile[8];

inline void zobristPieceTableInitialize(){
    /* Random number generator */
    std::default_random_engine generator{static_cast<long long unsigned>(0xc9558c91601b5d95)};
    // use constant seed, such that hashes don't change between different runs, makes for easy debugging

    /* Distribution on which to apply the generator */
    std::uniform_int_distribution<unsigned long long> distribution(0, 0xFFFFFFFFFFFFFFFF);
    for(auto & i : zobristPieceTable){
        for(auto & j : i){
            for(unsigned long & k : j) {
                k = distribution(generator);
            }
        }
    }
    for(unsigned long & i : zobristCastlingRightsTable){
        i = distribution(generator);
    }
    zobristBlackToMove = distribution(generator);
    for(unsigned long & i : zobristEnPassantFile){
        i = distribution(generator);
    }

}

// lookup table for direction from square a to square b
extern int rayDirectionsTable[64][64];

inline void rayDirectionLookupInitialize() {
    for (int i = 0; i < 64; i++) {
        for(int j = i + 1; j%8 != 0; j++) rayDirectionsTable[i][j] = HORIZONTAL_RAY;
        for(int j = i - 1; (j%8 != 7 && j%8 != -1); j--) rayDirectionsTable[i][j] = HORIZONTAL_RAY;

        for(int j = i + 8; j < 64; j += 8) rayDirectionsTable[i][j] = VERTICAL_RAY;
        for(int j = i - 8; j >= 0 ; j -= 8) rayDirectionsTable[i][j] = VERTICAL_RAY;

        for(int j = i + 9; j%8 != 0 && j < 64; j += 9) rayDirectionsTable[i][j] = NORTHWEST_RAY;
        for(int j = i - 9; (j%8 != 7 && j%8 != -1) && j > 0;  j -= 9) rayDirectionsTable[i][j] = NORTHWEST_RAY;

        for(int j = i + 7; (j%8 != 7 && j%8 != -1) && j < 64; j += 7) rayDirectionsTable[i][j] = SOUTHWEST_RAY;
        for(int j = i - 7; j%8 != 0 && j > 0 ; j -= 7) rayDirectionsTable[i][j] = SOUTHWEST_RAY;
    }
}

inline int rayDirectionLookup(const unsigned a, const unsigned b) {
    return rayDirectionsTable[a][b];
}


extern unsigned int killerMoves[MAX_DEPTH][KILLER_MOVE_SLOTS];


inline void addKillerMove(unsigned ply, unsigned move){
    /** shift old killer moves */
    for (int i = KILLER_MOVE_SLOTS - 2; i >= 0; i--)
        killerMoves[ply][i + 1] = killerMoves[ply][i];
    /** add new */
    killerMoves[ply][0] = move;
}

inline bool isKiller(unsigned ply, unsigned move){
    for(unsigned &kmove : killerMoves[ply]){
        if((kmove & (ORIGIN_SQUARE_MASK | DESTINATION_SQUARE_MASK)) == (move & (ORIGIN_SQUARE_MASK | DESTINATION_SQUARE_MASK))){
            return true;
        }
    }
    return false;
}


extern uint64_t butterflyTable[2][64][64];

inline void updateButterflyTable(unsigned ply, unsigned move, bool side){
    unsigned from = (move & ORIGIN_SQUARE_MASK) >> ORIGIN_SQUARE_SHIFT;
    unsigned to = (move & DESTINATION_SQUARE_MASK) >> DESTINATION_SQUARE_SHIFT;

    butterflyTable[side][from][to] += ply * ply;
}

inline unsigned getButterflyScore(unsigned ply, unsigned move, bool side){
    unsigned from = (move & ORIGIN_SQUARE_MASK) >> ORIGIN_SQUARE_SHIFT;
    unsigned to = (move & DESTINATION_SQUARE_MASK) >> DESTINATION_SQUARE_SHIFT;

    return butterflyTable[side][from][to];
}

#define ENGINE_ZOBRISTTABLES_H

#endif //ENGINE_ZOBRISTTABLES_H
