
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

void Position::prettyPrint() {

    char printArray[64] = {};
    unsigned index;

    for (int i = 0; i < 6; i++) {

        if(bitboards[WHITE][i]) {
            index = debruijnSerialization(bitboards[WHITE][i]);
            printArray[index] = char(toupper(FENpiecesReverse.at(i)));
        }
        if(bitboards[BLACK][i]) {
            index = debruijnSerialization(bitboards[BLACK][i]);
            printArray[index] = FENpiecesReverse.at(i);
        }
    }

    cout << "__________\n|";
    if (printArray[0]) {
        cout << " " << printArray[0] << " ";
    } else {
        cout << " . ";
    }
    for (int i = 1; i < 64; i++) {
        if (i % 8 == 0) {
            cout << "|\n|";
        }
        if (printArray[i]) {
            cout << " " << printArray[i] << " ";
        } else {
            cout << " . ";
        }

    }
    cout << "|\n__________\n\n";
}



void Position::GenerateMoves(moveList &movelist) {

    this->isIncheck = squareAttacked(bitboards[this->turn][KING], this->turn);
    this->helpBitboards[PINNED_PIECES] = pinnedPieces(bitboards[this->turn][KING], this->turn);
    GeneratePawnMoves(movelist);
    GenerateKnightMoves(movelist);
    GenerateSliderMoves(movelist);

}

void Position::GeneratePawnMoves(moveList &movelist) {
    //TODO: en passant, pins
    uint64_t pawns;

    pawns = bitboards[this->turn][PAWNS];

    unsigned pieceIndex;
    uint64_t pieceBB;
    while(pawns != 0){
        uint64_t currentPawnMoves, currentPawnCaptureMoves;

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


void Position::GenerateKnightMoves(moveList &movelist) {

    uint64_t knights;
    knights = bitboards[this->turn][KNIGHTS];

    unsigned pieceIndex;
    uint64_t pieceBB, currentKnightMoves, currentKnightCaptures; //pieceBitboard

    while (knights != 0) {
        //TODO: implement pinned piece check and remove pinned nights from this moveset.

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



void Position::GenerateSliderMoves(moveList &movelist){

    unsigned pieceIndex;
    uint64_t pieceBB, currentPieceMoves, currentPieceCaptures;
    int sliders[] = {BISHOPS, ROOKS, QUEENS}; // sliders to loop over
    for(int &currentSlider: sliders) {
        uint64_t currentPiece;
        currentPiece = bitboards[this->turn][currentSlider];
        while (currentPiece != 0) {
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
                default:
                    currentPieceMoves = 0;
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

    int squareIndex = (int) debruijnSerialization(pinnedOrigin);
    uint64_t pinnedPieces = 0;

    // check bishop/queen diagonal pins
    uint64_t diagonalAttacks = sliderAttacks.BishopAttacks(helpBitboards[OCCUPIED_SQUARES], squareIndex);

    //get $colour blockers
    uint64_t blockers = bitboards[colour][PIECES] & diagonalAttacks;

    // regenerate diagonal attacks, but remove the blockers one by one
    uint64_t singleBlocker;
    while(blockers!= 0) {
        // get a single blocker
        singleBlocker = debruijnSerialization(blockers);
        singleBlocker = 1uLL << singleBlocker;

        diagonalAttacks = sliderAttacks.BishopAttacks(helpBitboards[OCCUPIED_SQUARES] & ~singleBlocker, squareIndex);
        if(diagonalAttacks & (bitboards[!colour][BISHOPS] | bitboards[!colour][QUEENS])){
            pinnedPieces |= singleBlocker;
        }
    }


    //similar procedure for rook/queen rook attacks
    uint64_t rookAttacks = sliderAttacks.RookAttacks(helpBitboards[OCCUPIED_SQUARES], squareIndex);
    blockers = bitboards[colour][PIECES] & rookAttacks;

    while(blockers!= 0) {
        // get a single blocker
        singleBlocker = debruijnSerialization(blockers);
        singleBlocker = 1uLL << singleBlocker;

        diagonalAttacks = sliderAttacks.RookAttacks(helpBitboards[OCCUPIED_SQUARES] & ~singleBlocker, squareIndex);
        if(diagonalAttacks & (bitboards[!colour][ROOKS] | bitboards[!colour][QUEENS])){
            pinnedPieces |= singleBlocker;
        }
    }
    return pinnedPieces;
}

// checks if the square of $colour is attacked
bool Position::squareAttacked(uint64_t square, bool colour){
    unsigned squareIndex = debruijnSerialization(square);
    square = 1uLL << squareIndex; // as the square bit gets removed in debruijnSerialization function

    //check knight attacks
    if(knightAttacks(square) & bitboards[!colour][KNIGHTS]) return true;

    // check bishop attacks (+ queen diagonal attack)
    if(sliderAttacks.BishopAttacks(helpBitboards[OCCUPIED_SQUARES], int(squareIndex)) & (bitboards[!colour][BISHOPS] | bitboards[!colour][QUEENS])) return true;

    // check rook attacks (+ queen straight line attack)
    if(sliderAttacks.RookAttacks(helpBitboards[OCCUPIED_SQUARES], int(squareIndex)) & (bitboards[!colour][ROOKS] | bitboards[!colour][QUEENS])) return true;

    // check pawn attacks
    if(colour == BLACK){ // pawns going north (because white does attacking), so squareAttacked "attacks" south, so left shift
        if((square << 9u | square << 7u) & bitboards[BLACK][PAWNS]) return true;
    }
    else{
        if((square >> 9u | square >> 7u) & bitboards[WHITE][PAWNS]) return true;
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
        movelist.move[movelist.moveLength] = originInt << ORIGIN_SQUARE_SHIFT;
        movelist.move[movelist.moveLength++] |= destinationInt << DESTINATION_SQUARE_SHIFT;
    }

    while(captureDestinations != 0){
        destinationInt = debruijnSerialization(captureDestinations);

        movelist.captureMove[movelist.captureMoveLength] = originInt;
        movelist.captureMove[movelist.captureMoveLength++] |= destinationInt << DESTINATION_SQUARE_SHIFT;
    }



}

/*
 * This method checks which piece occupies originInt, and moves that piece to destinationInt. It does NOT check captures.
 * @param originInt origin bitboard
 * @param destinationInt destination bitboard
 * @param colour specify which colour moves (speeds up the search for the piece)
 */
void Position::MovePiece(uint64_t originBB, uint64_t destinationBB, bool colour){

    // find the piece that is moved.
    int pieceToMove = -1;
    for(int i = 0; i < 6; i++) {
        if(bitboards[colour][i] & originBB) {
            pieceToMove = i;
            break;
        }
    }

    // remove origin piece
    bitboards[this->turn][pieceToMove] &= ~originBB;

    // add destination piece
    bitboards[this->turn][pieceToMove] |= destinationBB;
}


// doMove  does the move and stores move information in the previesMoves array
// other than the 15 bits used in the bitboardsToMoveList convention the following bits are used:
// bit 16 capturemoveflag
// bit 17-19 captured piece
// bit 20-25 captured piece index
// in types.h the enum can be found for the shifts and the masks
void Position::doMove(unsigned move){
    unsigned originInt, destinationInt;


    originInt = (ORIGIN_SQAURE_MASK & move) >> ORIGIN_SQUARE_SHIFT;
    destinationInt = (DESTINATION_SQARE_MASK & move) >> DESTINATION_SQUARE_SHIFT;

    uint64_t originBB, destinationBB;

    originBB = 1uLL << originInt;
    destinationBB = 1uLL << destinationInt;

    MovePiece(originBB, destinationBB, this->turn);

    // switch to opponent bitboards and remove possible destination piece in case of capture
    // also save this piece in the previousMoves array as it needs to be restored in case of undoMove
    uint64_t capturedPiece;
    for(unsigned i = 0; i < 6; i++){
        capturedPiece = bitboards[!this->turn][i] & destinationBB;
        if(capturedPiece) {
            bitboards[!this->turn][i] &= ~capturedPiece;

            // store the capture move specifications
            move |= 1uLL << CAPTURE_MOVE_FLAG_SHIFT;
            move |= i << CAPTURED_PIECE_TYPE_SHIFT;
            move |= debruijnSerialization(capturedPiece) << CAPTURED_PIECE_INDEX_SHIFT;
            break;
        }
    }
    // put into previous moves list
    this->previousMoves[this->halfMoveNumber++] = move;
}


void Position::undoMove() {
    unsigned originInt, destinationInt;

    // get the move
    unsigned move = this->previousMoves[this->halfMoveNumber];
    // revert back to 0
    this->previousMoves[this->halfMoveNumber--] = 0;

    originInt = (move & ORIGIN_SQAURE_MASK) >> ORIGIN_SQUARE_SHIFT;
    destinationInt = (move & DESTINATION_SQARE_MASK) >> DESTINATION_SQUARE_SHIFT;

    uint64_t originBB = 1uLL << originInt;
    uint64_t destinationBB = 1uLL << destinationInt;

    // move the piece from destination to origin (so a possible wrong order warning is expected)
    MovePiece(destinationBB, originBB, !this->turn);

    // put the captured piece back
    if(move & CAPTURE_MOVE_FLAG_MASK){
        unsigned pieceIndex = (move & CAPTURED_PIECE_INDEX_MASK) >> CAPTURED_PIECE_INDEX_SHIFT;
        unsigned pieceType = (move & CAPTURED_PIECE_TYPE_MASK) >> CAPTURED_PIECE_TYPE_SHIFT;
        bitboards[this->turn][pieceType] |= 1uLL << pieceIndex;
    }
}


