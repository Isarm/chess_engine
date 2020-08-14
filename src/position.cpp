
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
        bitboards[isupper(FENpieces.at(pieceChar))][tolower(FENpieces.at(pieceChar))] += 1uLL << BBindex; // shift the bits into place
    }

    if(FEN.at(++i) == 'b'){
        this->turn = BLACK_TURN;
    } // turn gets initialized as white's turn by default

    //TODO: castling rights
    //TODO: move count
    //TODO: en passant


    generateHelpBitboards();

//    for(unsigned long bitboard : bitboards){
//        Bitboard::print(bitboard);
//    }



}

void Position::generateHelpBitboards() {
    /* first 6 bitboards are black pieces
     * 6 after are white
    */
    bitboards[BLACK_PIECES] = 0;
    bitboards[WHITE_PIECES] = 0;
    for(int i = 0; i < 6; i++){
        bitboards[BLACK_PIECES] |= bitboards[i];
        bitboards[WHITE_PIECES] |= bitboards[i + 6];
    }
    bitboards[OCCUPIED_SQUARES] = bitboards[WHITE_PIECES] | bitboards[BLACK_PIECES];
}




void Position::GeneratePseudoLegalMoves(moveList &movelist) {

    int moveListLength = 0;

    GeneratePseudoLegalPawnMoves(movelist);
    GeneratePseudoLegalKnightMoves(movelist);

}

void Position::GeneratePseudoLegalPawnMoves(moveList &movelist) {
    //TODO: en passant, pins
    uint64_t pawns;

    if(this->turn == WHITE_TURN){
        pawns = this->bitboards[WHITE_PAWNS];
    }
    else{
        pawns = this->bitboards[BLACK_PAWNS];
    }

    unsigned pieceIndex;
    uint64_t pieceBB;
    while(pawns != 0){
        uint64_t currentPawnMoves = 0, currentPawnCaptureMoves = 0;

        pieceIndex = debruijnSerialization(pawns);

        pieceBB = 1uLL << pieceIndex;

        if(this->turn == WHITE_TURN){ // pawns going NORTH, so right shift
            currentPawnMoves = pieceBB >> 8u;
            currentPawnMoves &= ~bitboards[OCCUPIED_SQUARES]; // make sure destination square is empty

            if(currentPawnMoves && is2ndRank(pieceBB)){ //check for double pawn move, if single pawn move exists and pawn is on 2nd rank
                currentPawnMoves |= pieceBB >> 16u;
                currentPawnMoves &= ~bitboards[OCCUPIED_SQUARES]; // again make sure destination square is empty
            }

            // generate capture moves
            currentPawnCaptureMoves = (pieceBB >> 7u) | (pieceBB >> 9u);
            currentPawnCaptureMoves &= bitboards[BLACK_PIECES];
        }
        else{ //black's turn, pawns going south so left shift
            currentPawnMoves = pieceBB << 8u;
            currentPawnMoves &= ~bitboards[OCCUPIED_SQUARES]; // make sure destination square is empty

            if(currentPawnMoves && is7thRank(pieceBB)){ //check for double pawn move, if single pawn move exists and pawn is on 2nd rank
                currentPawnMoves |= pieceBB << 16u;
                currentPawnMoves &= ~bitboards[OCCUPIED_SQUARES]; // again make sure destination square is empty
            }

            // generate capture moves
            currentPawnCaptureMoves = (pieceBB << 7u) | (pieceBB << 9u);
            currentPawnCaptureMoves &= bitboards[WHITE_PIECES];
        }

        // update movelist
        bitboardsToMovelist(movelist, pieceBB, currentPawnMoves, currentPawnCaptureMoves);
    }
}


void Position::GeneratePseudoLegalKnightMoves(moveList &movelist) {

    uint64_t knights;
    if(this->turn == WHITE_TURN) {
        knights = this->bitboards[WHITE_KNIGHTS];
    }
    else{
        knights = this->bitboards[BLACK_KNIGHTS];
    }

    unsigned pieceIndex;
    uint64_t pieceBB; //pieceBitboard

    while (knights != 0) {
        //TODO: implement pinned piece check and remove pinned nights from this moveset.
        uint64_t currentKnightMoves = 0, currentKnightCaptureMoves = 0;

        //grabs the first knight and returns it's index. This knight also gets removed from the knights variable bibboard
        // so in the next loop the next knight will be returned.
        pieceIndex = debruijnSerialization(knights);
        pieceBB = 1uLL << pieceIndex;

        currentKnightMoves = knightAttacks(knights); //get the knight attacks (moveGenHelpFunctions.h)


        if(this->turn == WHITE_TURN){

            // use knightmoves AND NOT white pieces to remove blocked squares
            currentKnightMoves &= ~bitboards[WHITE_PIECES];

            // use knightmoves AND opponent pieces to get capture moves (storing capturemoves differently is efficient for the search tree)
            currentKnightCaptureMoves = currentKnightMoves & bitboards[BLACK_PIECES];
        }
        else{
            currentKnightMoves &= ~bitboards[BLACK_PIECES];

            currentKnightCaptureMoves = currentKnightMoves & bitboards[WHITE_PIECES];
        }

        // remove capturemoves from the normal moves bitboard
        currentKnightMoves = currentKnightMoves & (0xFFFFFFFFFFFFFFFF ^ currentKnightCaptureMoves);

        bitboardsToMovelist(movelist, pieceBB, currentKnightMoves, currentKnightCaptureMoves);

    }

}



void Position::GeneratePseudoLegalBishopMoves(moveList &movelist){

}

// checks if the king of color side is in check
bool Position::isInCheck(bool side){

}

/* This function converts a origin bitboard and destination bitboard into a movelist
 * for storing moves the following approach is used: (similar to stockfish)
 * bit 0-5 === origin square;
 * bit 6-11 === destination square;
 * bit 12-13 === promotion piece type (N, B, R, Q)
 * bit 14-15 === special move flag, promotion, en passant, castling
 *
 */

void Position::bitboardsToMovelist(moveList &movelist, uint64_t origin, uint64_t destinations, uint64_t captureDestinations) {
    unsigned originInt, destinationInt;
    originInt = debruijnSerialization(origin);

    while(destinations != 0){
        // get integer of destination position
        destinationInt = debruijnSerialization(destinations);

        // put origin and destination in movelist
        movelist.move[movelist.moveLength] = originInt;
        movelist.move[movelist.moveLength++] |= destinationInt << 6u;
    }

    while(captureDestinations != 0){
        destinationInt = debruijnSerialization(captureDestinations);

        movelist.captureMove[movelist.captureMoveLength] = originInt;
        movelist.captureMove[movelist.captureMoveLength++] |= destinationInt << 6u;
    }



}

void Position::doMove(unsigned move){
    unsigned originInt, destinationInt;

    // put into previous moves list
    this->previousMoves[this->halfMoveNumber++] = move;

    originInt = 0x3F & move;
    destinationInt = (0xFC0 & move) >> 6;

    uint64_t originBB, destinationBB;

    originBB = 1uLL << originInt;
    destinationBB = 1uLL << destinationInt;

    // find the piece that is moved.
    int start;
    this->turn ? start = 6 : start = 0; // set which bitboards to iterate over

    int bitboardIndex = -1;
    for(int i = start; i < (start + 6); i++) {
        if(bitboards[i] & originBB) {
            bitboardIndex = i;
            break;
        }
    }


    // remove origin piece
    bitboards[bitboardIndex] &= ~originBB;



    // switch to opponent bitboards and remove possible destination piece in case of capture
    start = (start - 6) % 12;
    for(int i = start; i < start + 6; i++){
        bitboards[i] &= ~destinationBB;
    }


    // add destination piece
    bitboards[bitboardIndex] |= destinationBB;

    // update help bitboards
    generateHelpBitboards();

    for(unsigned long bitboard : bitboards){
        Bitboard::print(bitboard);
    }


}
