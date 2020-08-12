
#include <position.h>
#include <string>
#include <cstdint>

#include "position.h"
#include "types.h"
#include "bitboard.h"
#include "moveGenHelpFunctions.h"

using namespace std;
using namespace types;

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
    int i; // initialize here as it is also needed outside of the scope of the for loop
    for(i = 0; pieceChar != ' '; i++, BBindex++, pieceChar = FEN.at(i)){ // for loop for first part of FEN until first space
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

    if(FEN.at(++i) == 'b'){
        this->turn = BLACK_TURN;
    } // turn gets initialized as white's turn by default

    //TODO: castling rights
    //TODO: move count
    //TODO: en passant


    initializeHelpBitboards();

    for(unsigned long bitboard : bitboards){
        Bitboard::print(bitboard);
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




void Position::GeneratePseudoLegalMoves(moveList &movelist) {

    int moveListLength = 0;

    GeneratePseudoLegalKnightMoves(movelist);

}

void Position::GeneratePseudoLegalKnightMoves(moveList &movelist) {

    uint64_t knights;
    this->turn ? knights = this->bitboards[BLACK_KNIGHTS] : knights = this->bitboards[WHITE_KNIGHTS];


    uint64_t LS1B, pieceBB;
    unsigned pieceIndex;
    const auto debruijn64 = uint64_t(0x03f79d71b4cb0a89); // from the chess programming wiki

    while (knights != 0) {
        uint64_t currentKnightMoves = 0;

        LS1B = knights & -knights; // only keeps the 1st LSB bit so that the DeBruijn bitscan can be used
        knights ^= LS1B; // remove this bit for next cycle using XOR
        pieceIndex = index64[(LS1B * debruijn64) >> 58]; //index64 defined in types.h
        pieceBB = 1uLL << pieceIndex; // put the piece in bitboard

        if(notAFile(pieceBB)) {
            currentKnightMoves |= ((pieceBB >> NNW) | (pieceBB << SSW));
        }
        if(notABFile(pieceBB)) {
            currentKnightMoves |= ((pieceBB >> NWW) | (pieceBB << SWW));
        }
        if(notHFile(pieceBB)) {
            currentKnightMoves |= ((pieceBB >> NNE) | (pieceBB << SSE));
        }
        if(notGHFile(pieceBB)) {
            currentKnightMoves |= ((pieceBB >> NEE) | (pieceBB << SEE));
        }

        // use knightmoves XOR ownPieces, and then again AND to get rid of own piece blockers
        this->turn ? currentKnightMoves &= currentKnightMoves ^ bitboards[BLACK_PIECES] : currentKnightMoves &= currentKnightMoves ^ bitboards[WHITE_PIECES];



    }



}

bool Position::notAFile(uint64_t bb) {
    return uint64_t(0x7F7F7F7F7F7F7F7F) & bb;
}
bool Position::notABFile(uint64_t bb){
    return uint64_t(0xFCFCFCFCFCFCFCFC) & bb;
}
bool Position::notHFile(uint64_t bb) {
    return uint64_t(0x7F7F7F7F7F7F7F7F) & bb;
}
bool Position::notGHFile(uint64_t bb) {
    return uint64_t(0x3F3F3F3F3F3F3F3F) & bb;
}



