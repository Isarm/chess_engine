
#include <position.h>
#include <string>
#include <cstdint>

#include "position.h"
#include "types.h"
#include "bitboard.h"
#include "moveGenHelpFunctions.h"
#include "slider_attacks.h"

using namespace std;
using namespace types;

Position::Position(string FEN){

    string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    if(FEN == "startpos") FEN = startFEN;


    // loop through the FEN string until first space
    unsigned BBindex = 0; // this gets incremented when the FEN contains numbers as well
    char pieceChar = FEN.at(0);
    int i; // initialize here as it is also needed outside of the scope of the for loop
    for(i = 0; pieceChar != ' '; i++, BBindex++, pieceChar = FEN.at(i)){ // loop for first part of FEN until first space
        if(pieceChar == '/'){
            BBindex--; // account for (incorrect) increment, as it is simply a new row
            continue;
        }
        if(pieceChar <= '9' && pieceChar > '0') {
            BBindex += pieceChar - '1';
            continue;
        }

        bitboards[isupper(pieceChar) != 0][FENpieces.at(tolower(pieceChar))] |= 1uLL << BBindex; // shift the bits into place
    }

    if(FEN.at(++i) == 'b'){
        this->turn = BLACK;
    } // turn gets initialized as white's turn by default

    //TODO: castling rights
    //TODO: move count
    //TODO: en passant


    generateHelpBitboards();

    sliderAttacks = SliderAttacks();
    sliderAttacks.Initialize(); // initialize slider attacks

//    for(unsigned long bitboard : bitboards){
//        Bitboard::print(bitboard);
//    }



}

void Position::generateHelpBitboards() {

    bitboards[BLACK][PIECES] = 0;
    bitboards[WHITE][PIECES] = 0;
    for(int i = 0; i < 6; i++){
        bitboards[BLACK][PIECES] |= bitboards[BLACK][i];
        bitboards[WHITE][PIECES] |= bitboards[WHITE][i];
    }
    helpBitboards[OCCUPIED_SQUARES] = bitboards[BLACK][PIECES] | bitboards[WHITE][PIECES];
}




void Position::GeneratePseudoLegalMoves(moveList &movelist) {

    int moveListLength = 0;

    GeneratePseudoLegalPawnMoves(movelist);
    GeneratePseudoLegalKnightMoves(movelist);
    GeneratePseudoLegalSliderMoves(movelist);

}

void Position::GeneratePseudoLegalPawnMoves(moveList &movelist) {
    //TODO: en passant, pins
    uint64_t pawns;

    pawns = bitboards[this->turn][PAWNS];

    unsigned pieceIndex;
    uint64_t pieceBB;
    while(pawns != 0){
        uint64_t currentPawnMoves = 0, currentPawnCaptureMoves = 0;

        pieceIndex = debruijnSerialization(pawns);

        pieceBB = 1uLL << pieceIndex;

        if(this->turn == WHITE){ // pawns going NORTH, so right shift
            currentPawnMoves = pieceBB >> 8u;
            currentPawnMoves &= ~helpBitboards[OCCUPIED_SQUARES]; // make sure destination square is empty

            if(currentPawnMoves && is2ndRank(pieceBB)){ //check for double pawn move, if single pawn move exists and pawn is on 2nd rank
                currentPawnMoves |= pieceBB >> 16u;
                currentPawnMoves &= ~helpBitboards[OCCUPIED_SQUARES]; // again make sure destination square is empty
            }

            // generate capture moves
            currentPawnCaptureMoves = (pieceBB >> 7u) | (pieceBB >> 9u);
            currentPawnCaptureMoves &= bitboards[BLACK][PIECES];
        }
        else{ //black's turn, pawns going south so left shift
            currentPawnMoves = pieceBB << 8u;
            currentPawnMoves &= ~helpBitboards[OCCUPIED_SQUARES]; // make sure destination square is empty

            if(currentPawnMoves && is7thRank(pieceBB)){ //check for double pawn move, if single pawn move exists and pawn is on 2nd rank
                currentPawnMoves |= pieceBB << 16u;
                currentPawnMoves &= ~helpBitboards[OCCUPIED_SQUARES]; // again make sure destination square is empty
            }

            // generate capture moves
            currentPawnCaptureMoves = (pieceBB << 7u) | (pieceBB << 9u);
            currentPawnCaptureMoves &= bitboards[WHITE][PIECES];
        }

        // update movelist
        bitboardsToMovelist(movelist, pieceBB, currentPawnMoves, currentPawnCaptureMoves);
    }
}


void Position::GeneratePseudoLegalKnightMoves(moveList &movelist) {

    uint64_t knights;
    knights = bitboards[this->turn][KNIGHTS];

    unsigned pieceIndex;
    uint64_t pieceBB, currentKnightMoves, currentKnightCaptures; //pieceBitboard

    while (knights != 0) {
        //TODO: implement pinned piece check and remove pinned nights from this moveset.
        currentKnightMoves = 0, currentKnightCaptures = 0;

        //grabs the first knight and returns it's index. This knight also gets removed from the knights variable bibboard
        // so in the next loop the next knight will be returned.
        pieceIndex = debruijnSerialization(knights);
        pieceBB = 1uLL << pieceIndex;

        currentKnightMoves = knightAttacks(pieceBB); //get the knight attacks (moveGenHelpFunctions.h)


        // use knightmoves AND NOT white pieces to remove blocked squares
        currentKnightMoves &= ~bitboards[this->turn][PIECES];

        // use knightmoves AND opponent pieces to get capture moves (storing capturemoves differently is efficient for the search tree)
        currentKnightCaptures = currentKnightMoves & bitboards[!this->turn][PIECES];


        // remove capturemoves from the normal moves bitboard
        currentKnightMoves  &= ~currentKnightCaptures;

        bitboardsToMovelist(movelist, pieceBB, currentKnightMoves, currentKnightCaptures);

    }

}



void Position::GeneratePseudoLegalSliderMoves(moveList &movelist){

    unsigned pieceIndex;
    uint64_t pieceBB, currentPieceMoves, currentPieceCaptures;
    int sliders[] = {BISHOPS, ROOKS, QUEENS}; // sliders to loop over
    for(int &currentSlider: sliders) {
        uint64_t currentPiece;
        currentPiece = bitboards[this->turn][currentSlider];
        while (currentPiece != 0) {
            currentPieceMoves = 0, currentPieceCaptures = 0;

            pieceIndex = debruijnSerialization(currentPiece);
            pieceBB = 1uLL << pieceIndex;

            // get slider attacks
            switch (currentSlider) {
                case BISHOPS:
                    currentPieceMoves = sliderAttacks.BishopAttacks(helpBitboards[OCCUPIED_SQUARES], int(pieceIndex));
                    break;
                case ROOKS:
                    currentPieceMoves = sliderAttacks.RookAttacks(helpBitboards[OCCUPIED_SQUARES], int(pieceIndex));
                    break;
                case QUEENS:
                    currentPieceMoves = sliderAttacks.QueenAttacks(helpBitboards[OCCUPIED_SQUARES], int(pieceIndex));
                    break;
            }

            // remove same side blockers
            currentPieceMoves &= ~bitboards[this->turn][PIECES];

            // divide in capture and normal moves
            currentPieceCaptures = currentPieceMoves & bitboards[!this->turn][PIECES];
            currentPieceMoves &= ~currentPieceCaptures;

            bitboardsToMovelist(movelist, pieceBB, currentPieceMoves, currentPieceCaptures);
        }
    }
}

// pinnedOrigin is the location that will be checked for pins (e.g. pinnedOrigin is king for legality check)
// pinned pieces are of color $colour
uint64_t Position::pinnedPieces(uint64_t pinnedOrigin, bool colour){

    unsigned squareIndex = debruijnSerialization(pinnedOrigin);
    uint64_t pinnedPieces;

    // check bishop/queen diagonal pins
    uint64_t diagonalAttacks = sliderAttacks.BishopAttacks(helpBitboards[OCCUPIED_SQUARES], squareIndex);

    //get $colour blockers
    uint64_t blockers = bitboards[colour][PIECES] & diagonalAttacks;

    // regenerate diagonal attacks, but with the blockers removed
    diagonalAttacks = sliderAttacks.BishopAttacks(helpBitboards[OCCUPIED_SQUARES] & ~blockers, squareIndex);


    //similar procedure for rook/queen rook attacks
    uint64_t rookAttacks = sliderAttacks.RookAttacks(helpBitboards[OCCUPIED_SQUARES], squareIndex);
    blockers = bitboards[colour][PIECES] & rookAttacks;
    rookAttacks = sliderAttacks.RookAttacks(helpBitboards[OCCUPIED_SQUARES] & ~blockers, squareIndex);



}

// checks if the square is attacked by $colour
bool Position::squareAttackedBy(uint64_t squareAttacked, bool colour){
    unsigned squareIndex = debruijnSerialization(squareAttacked);
    squareAttacked = 1uLL << squareIndex; // as the square bit gets removed in debruijnSerialization function

    //check knight attacks
    if(knightAttacks(squareAttacked) & bitboards[colour][KNIGHTS]) return true;

    // check bishop attacks (+ queen diagonal attack)
    if(sliderAttacks.BishopAttacks(helpBitboards[OCCUPIED_SQUARES], int(squareIndex)) & (bitboards[colour][BISHOPS] | bitboards[colour][QUEENS])) return true;

    // check rook attacks (+ queen straight line attack)
    if(sliderAttacks.RookAttacks(helpBitboards[OCCUPIED_SQUARES], int(squareIndex)) & (bitboards[colour][ROOKS]) | bitboards[colour][QUEENS]) return true;

    // check pawn attacks
    if(colour == BLACK){ // pawns going north (because white does attacking), so squareAttacked "attacks" south, so left shift
        if((squareAttacked << 9u | squareAttacked << 7u) & bitboards[BLACK][PAWNS]) return true;
    }
    else{
        if((squareAttacked >> 9u | squareAttacked >> 7u) & bitboards[WHITE][PAWNS]) return true;
    }

    return false;


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

    int pieceToMove = -1;
    for(int i = 0; i < 6; i++) {
        if(bitboards[this->turn][i] & originBB) {
            pieceToMove = i;
            break;
        }
    }

    // remove origin piece
    bitboards[this->turn][pieceToMove] &= ~originBB;


    // switch to opponent bitboards and remove possible destination piece in case of capture
    for(int i = 0; i < 6; i++){
        bitboards[!this->turn][i] &= ~destinationBB;
    }


    // add destination piece
    bitboards[this->turn][pieceToMove] |= destinationBB;

    // update help bitboards
    generateHelpBitboards();

    Bitboard::printAll(bitboards);



}
