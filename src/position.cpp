
#include <position.h>
#include <string>
#include <cstdint>

#include "position.h"
#include "types.h"
#include "bitboard.h"

using namespace std;

Position::Position(string FEN){

    string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    if(FEN == "startpos") FEN = startFEN;

    // initialize bitboards to 0.
    for(uint64_t& i : bitboards){
        i = 0;
    }

    // loop through the FEN string until first space
    unsigned BBindex = 0; // this gets incremented when the FEN contains numbers as well
    char pieceChar = FEN.at(0);
    for(int i = 0; pieceChar != ' '; i++, BBindex++, pieceChar = FEN.at(i)){
        if(pieceChar == '/'){
            BBindex--; // account for (incorrect) increment, as it is simply a new row
            continue;
        }
        if(pieceChar <= '9' && pieceChar > '0') {
            BBindex += pieceChar - '1';
            continue;
        }
        bitboards[FENpieces.at(pieceChar)] += 1uLL << BBindex; // shift the bits into place
    }

    initializeHelpBitboards();

    for(unsigned long bitboard : bitboards){
        cout << Bitboard::print(bitboard);
        cout << "\n\n";
    }
}

void Position::initializeHelpBitboards() {
    /* first 6 bitboards are black pieces
     * 6 after are white
    */
    for(int i = 0; i < 6; i++){
        bitboards[BLACK_PIECES] |= bitboards[i];
        bitboards[WHITE_PIECES] |= bitboards[i + 6];
    }
}

int *Position::GeneratePseudoLegalMoves(int moveList[254], int *moveListLength){

    moveList = GeneratePseudoLegalKnightMoves(moveList);

    return moveList;
}

int *Position::GeneratePseudoLegalKnightMoves(int *moveList) {
    uint64_t knights;
    this->turn ? knights = BLACK_KNIGHTS : knights = WHITE_KNIGHTS;

    uint64_t LS1B;
    int pieceIndex;
    while(knights != 0){
        const auto debruijn64 = uint64_t (0x03f79d71b4cb0a89); // from the chess programming wiki

        LS1B = knights & -knights; // only keeps the 1st LSB bit so that the DeBruijn bitscan can be used
        knights ^= LS1B; // remove this bit for next cycle using XOR

        pieceIndex = index64[(LS1B * debruijn64) >> 58]; //index64 defined in types.h

        pieceIndex
    }


    return nullptr;
}




int Position::bitScanForward(uint64_t bb) {
}


