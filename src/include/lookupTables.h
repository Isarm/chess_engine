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

extern uint64_t knightAttacksLUT[64];
extern uint64_t kingAttacksLUT[64];

extern int rayDirectionsTable[64][64];

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

inline uint64_t knightAttacks(const uint64_t knight) {
    uint64_t currentKnightMoves = 0;
    if (notAFile(knight)) {
        currentKnightMoves |= ((knight >> NNW) | (knight << SSW));
    }
    if (notABFile(knight)) {
        currentKnightMoves |= ((knight >> NWW) | (knight << SWW));
    }
    if (notHFile(knight)) {
        currentKnightMoves |= ((knight >> NNE) | (knight << SSE));
    }
    if (notGHFile(knight)) {
        currentKnightMoves |= ((knight >> NEE) | (knight << SEE));
    }
    return currentKnightMoves;
}

inline void knightAttacksLUTinitialize(){
    for (int i = 0; i < 64; ++i) {
        knightAttacksLUT[i] = knightAttacks(1ull << i);
    }
}

inline uint64_t getKnightAttacks(const unsigned knight) {
    return knightAttacksLUT[knight];
}

inline uint64_t kingAttacks(const uint64_t king){
    uint64_t kingAttacks = 0;
    if(notAFile(king)){
        kingAttacks |= (king >> NW) | (king >> W) | (king << SW);
    }
    if(notHFile(king)) {
        kingAttacks |= (king >> NE) | (king << E) | (king << SE);
    }
    kingAttacks |= (king >> N) | (king << S);

    return kingAttacks;
}

inline void kingAttacksLUTinitialize(){
    for (int i = 0; i < 64; ++i) {
        kingAttacksLUT[i] = kingAttacks(1ull << i);
    }
}

inline uint64_t getKingAttacks(const unsigned king){
    return kingAttacksLUT[king];
}

#define ENGINE_ZOBRISTTABLES_H

#endif //ENGINE_ZOBRISTTABLES_H
