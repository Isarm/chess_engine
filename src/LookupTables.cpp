//
// Created by isar on 01/08/2021.
//

#include "LookupTables.h"


LookupTables::LookupTables() {
    zobristPieceTableInitialize();
    kingAttacksLUTinitialize();
    knightAttacksLUTinitialize();
    rayDirectionLookupInitialize();
    frontSpansInitialize();
    kingPawnShieldInitialize();
}

void LookupTables::zobristPieceTableInitialize(){
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

void LookupTables::rayDirectionLookupInitialize() {
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


inline uint64_t LookupTables::knightAttacks(const uint64_t knight) {
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

void LookupTables::knightAttacksLUTinitialize(){
    for (int i = 0; i < 64; ++i) {
        knightAttacksLUT[i] = knightAttacks(1ull << i);
    }
}


uint64_t LookupTables::kingAttacks(const uint64_t king){
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

void LookupTables::kingAttacksLUTinitialize(){
    for (int i = 0; i < 64; ++i) {
        kingAttacksLUT[i] = kingAttacks(1ull << i);
    }
}

void LookupTables::frontSpansInitialize(){
    /** First do from white's point of view */
    for(int i = 8; i < 64; i++){
        if(i % 8 == 0){
            /** A file */
            frontSpans[WHITE][i] = frontSpans[WHITE][i - 8] | (1ull << (i - 8) | 1ull << (i - 7));
        } else if (i % 8 == 7){
            /** H file */
            frontSpans[WHITE][i] = frontSpans[WHITE][i - 8] | (1ull << (i - 9) | 1ull << (i - 8));
        } else{
            frontSpans[WHITE][i] = frontSpans[WHITE][i - 8] | (1ull << (i - 9) | 1ull << (i - 8)) | 1ull << (i - 7);
        }
    }
    /** Black's point of view */
    for(int i = 55; i >= 0; i--){
        if(i % 8 == 0){
            /** A file */
            frontSpans[BLACK][i] = frontSpans[BLACK][i + 8] | (1ull << (i + 8) | 1ull << (i + 9));
        } else if (i % 8 == 7){
            /** H file */
            frontSpans[BLACK][i] = frontSpans[BLACK][i + 8] | (1ull << (i + 7) | 1ull << (i + 8));
        } else{
            frontSpans[BLACK][i] = frontSpans[BLACK][i + 8] | (1ull << (i + 9) | 1ull << (i + 8)) | 1ull << (i + 7);
        }
    }
}

void LookupTables::kingPawnShieldInitialize() {
    /** White */
    /** First row above king */
    for(int i = 8; i < 64; i++){
        if(i % 8 == 0){
            /** A file */
            kingPawnShield[WHITE][i] |= (1ull << (i - 8) | 1ull << (i - 7));
        }
        else if (i % 8 == 7){
            /** H file */
            kingPawnShield[WHITE][i] |= (1ull << (i - 9) | 1ull << (i - 8));
        } else{
            kingPawnShield[WHITE][i] |= (1ull << (i - 9) | 1ull << (i - 8)) | 1ull << (i - 7);
        }
    }
    /** Second row above king */
    for(int i = 16; i < 64; i++){
        if(i % 8 == 0){
            /** A file */
            kingPawnShield[WHITE][i] |= (1ull << (i - 16) | 1ull << (i - 15));
        }
        else if (i % 8 == 7){
            /** H file */
            kingPawnShield[WHITE][i] |= (1ull << (i - 17) | 1ull << (i - 16));
        } else{
            kingPawnShield[WHITE][i] |= (1ull << (i - 17) | 1ull << (i - 16)) | 1ull << (i - 15);
        }
    }

    /** Black */
    for(int i = 55; i >= 0; i--){
        if(i % 8 == 0){
            /** A file */
            kingPawnShield[BLACK][i] |= (1ull << (i + 8) | 1ull << (i + 9));
        } else if (i % 8 == 7){
            /** H file */
            kingPawnShield[BLACK][i] |= (1ull << (i + 7) | 1ull << (i + 8));
        } else{
            kingPawnShield[BLACK][i] |= (1ull << (i + 9) | 1ull << (i + 8)) | 1ull << (i + 7);
        }
    }
    for(int i = 47; i >= 0; i--){
        if(i % 8 == 0){
            /** A file */
            kingPawnShield[BLACK][i] |= (1ull << (i + 16) | 1ull << (i + 17));
        } else if (i % 8 == 7){
            /** H file */
            kingPawnShield[BLACK][i] |= (1ull << (i + 15) | 1ull << (i + 16));
        } else{
            kingPawnShield[BLACK][i] |= (1ull << (i + 17) | 1ull << (i + 16)) | 1ull << (i + 15);
        }
    }
}

