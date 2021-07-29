//
// Created by isar on 11/07/2021.
//

#include "lookupTables.h"

// zobrist piece table for [colour][piece][index]
uint64_t zobristPieceTable[2][6][64];
uint64_t zobristCastlingRightsTable[16];
uint64_t zobristBlackToMove;
uint64_t zobristEnPassantFile[8];

int rayDirectionsTable[64][64];

uint64_t knightAttacksLUT[64];
uint64_t kingAttacksLUT[64];
