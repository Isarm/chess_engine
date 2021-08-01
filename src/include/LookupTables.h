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
    LookupTables();

    // zobrist piece table for [colour][piece][index]
    uint64_t zobristPieceTable[2][6][64]{};
    uint64_t zobristCastlingRightsTable[16]{};
    uint64_t zobristBlackToMove{};
    uint64_t zobristEnPassantFile[8]{};

    int rayDirectionsTable[64][64]{};

    uint64_t knightAttacksLUT[64]{};
    uint64_t kingAttacksLUT[64]{};

    inline int rayDirectionLookup(const unsigned a, const unsigned b) {
        return rayDirectionsTable[a][b];
    }
    inline uint64_t getKnightAttacks(const unsigned knight) {
        return knightAttacksLUT[knight];
    }
    inline uint64_t getKingAttacks(const unsigned king){
        return kingAttacksLUT[king];
    }


private:
    void zobristPieceTableInitialize();

    void rayDirectionLookupInitialize();

    static uint64_t knightAttacks(uint64_t knight);

    void knightAttacksLUTinitialize();

    static uint64_t kingAttacks(uint64_t king);

    void kingAttacksLUTinitialize();

};


#endif //ENGINE_LOOKUPTABLES_H
