//
// Created by isar on 01/08/2021.
//

#ifndef ENGINE_LOOKUPTABLES_H
#define ENGINE_LOOKUPTABLES_H

#include <cstdint>
#include <random>
#include "definitions.h"
#include "useful.h"

class LookupTables {
public:
    uint64_t zobristPieceTable[2][6][64]{};
    uint64_t zobristCastlingRightsTable[16]{};
    uint64_t zobristBlackToMove{};
    uint64_t zobristEnPassantFile[8]{};
    uint64_t frontSpans[2][64]{};
    uint64_t kingPawnShield[2][64]{};
    LookupTables();

    inline int rayDirectionLookup(const unsigned a, const unsigned b) {
        return rayDirectionsTable[a][b];
    }
    inline uint64_t getKnightAttacks(const unsigned knight) {
        return knightAttacksLUT[knight];
    }
    inline uint64_t getKingAttacks(const unsigned king){
        return kingAttacksLUT[king];
    }


private:    // zobrist piece table for [colour][piece][index]

    int rayDirectionsTable[64][64]{};

    uint64_t knightAttacksLUT[64]{};
    uint64_t kingAttacksLUT[64]{};
    void zobristPieceTableInitialize();

    void rayDirectionLookupInitialize();

    static uint64_t knightAttacks(uint64_t knight);

    void knightAttacksLUTinitialize();

    static uint64_t kingAttacks(uint64_t king);

    void kingAttacksLUTinitialize();

    void frontSpansInitialize();

    void kingPawnShieldInitialize();
};


#endif //ENGINE_LOOKUPTABLES_H
