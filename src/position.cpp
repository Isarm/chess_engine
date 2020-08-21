
#include <position.h>
#include <string>
#include <cstdint>

#include "position.h"
#include "definitions.h"
#include "bitboard.h"
#include "useful.h"
#include "slider_attacks.h"

using namespace std;
using namespace definitions;

Position::Position(string FEN){

    string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    if(FEN == "startpos") FEN = startFEN;


    // loop through the FEN string until first space
    unsigned BBindex = 0; // this gets incremented when the FEN contains numbers as well
    char pieceChar = FEN.at(0);
    int i; // initialize here as it is also needed outside of the scope of the for loop

    if(FEN.at(i) == '"') i++; // skip possible leading quotation marks (in case of perft debug)

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

    //move on to castling rights
    ++i;

    while(FEN.at(++i) != ' '){
        switch (FEN.at(i)){
            case 'K':
                this->castlingRights |= WHITE_KINGSIDE_CASTLING_RIGHTS;
                break;
            case 'Q':
                this-> castlingRights |= WHITE_QUEENSIDE_CASTLING_RIGHTS;
                break;
            case 'k':
                this->castlingRights |= BLACK_KINGSIDE_CASTLING_RIGHTS;
                break;
            case 'q':
                this-> castlingRights |= BLACK_QUEENSIDE_CASTLING_RIGHTS;
                break;
            default:
                break;
        }
    }

    if(FEN.at(++i) != '-'){
        unsigned enPassantInt = FEN.at(i) - 'a';
        enPassantInt += (8 - (FEN.at(++i) - '0')) * 8;
        bitboards[!this->turn][EN_PASSANT_SQUARES] = 1uLL << enPassantInt;
    }

    ++i;

    char halfMove50[3];
    int hmi = 0;
    while(FEN.at(++i) != ' '){
        halfMove50[hmi] = FEN.at(i);
    }

    this->halfMoveNumber50 = stoi(halfMove50);

    char fullMove[5];
    int fmi = 0;
    while(++i < FEN.length() && FEN.at(i) != '"'){
        fullMove[fmi] = FEN.at(i);
    }

    this->fullMoveNumber = stoi(fullMove);

    generateHelpBitboards();

    sliderAttacks = SliderAttacks();
    sliderAttacks.Initialize(); // initialize slider attacks

    rayDirectionLookupInitialize(); // initialize ray lookup table;

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
    uint64_t BB;
    for (int i = 0; i < 6; i++) {

        if(bitboards[WHITE][i]) {
            BB = bitboards[WHITE][i];
            while(BB != 0) {
                index = debruijnSerialization(BB);
                BB &= ~(1uLL << index); // remove this piece from BB
                printArray[index] = char(toupper(FENpiecesReverse.at(i)));
            }
        }
        if(bitboards[BLACK][i]) {
            BB = bitboards[BLACK][i];
            while (BB != 0) {
                index = debruijnSerialization(BB);
                BB &= ~(1uLL << index); // remove this piece from BB
                printArray[index] = FENpiecesReverse.at(i);
            }
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
    if(!isIncheck) {
        this->helpBitboards[PINNED_PIECES] = pinnedPieces(bitboards[this->turn][KING], this->turn);
    }
    else{
        this->helpBitboards[PINNED_PIECES] = 0; // as pinnedPieces function only works when the king is not in check TODO:??
    }
    GeneratePawnMoves(movelist);
    GenerateKnightMoves(movelist);
    GenerateSliderMoves(movelist);
    GenerateKingMoves(movelist);
    if(!isIncheck) {
        GenerateCastlingMoves(movelist);
    }
}

void Position::GeneratePawnMoves(moveList &movelist) {
    //TODO: en passant, pins
    uint64_t pawns;

    pawns = bitboards[this->turn][PAWNS];

    unsigned pieceIndex;
    uint64_t pieceBB;
    while(pawns != 0){
        uint64_t currentPawnMoves, currentPawnCaptureMoves = 0, enPassantMove;

        pieceIndex = debruijnSerialization(pawns);

        pieceBB = 1uLL << pieceIndex;
        pawns &= ~pieceBB;

        if(this->turn == WHITE){ // pawns going NORTH, so right shift
            currentPawnMoves = pieceBB >> N;
            currentPawnMoves &= ~helpBitboards[OCCUPIED_SQUARES]; // make sure destination square is empty

            if(currentPawnMoves && is2ndRank(pieceBB)){ //check for double pawn move, if single pawn move exists and pawn is on 2nd rank
                currentPawnMoves |= pieceBB >> NN;
                currentPawnMoves &= ~helpBitboards[OCCUPIED_SQUARES]; // again make sure destination square is empty
            }

            // generate capture moves
            if(notAFile(pieceBB)) currentPawnCaptureMoves |= pieceBB >> NW;
            if(notHFile(pieceBB)) currentPawnCaptureMoves |= pieceBB >> NE;
            enPassantMove = currentPawnCaptureMoves & bitboards[BLACK][EN_PASSANT_SQUARES];
            currentPawnCaptureMoves &= bitboards[BLACK][PIECES];


        }
        else{ //black's turn, pawns going south so left shift
            currentPawnMoves = pieceBB << S;
            currentPawnMoves &= ~helpBitboards[OCCUPIED_SQUARES]; // make sure destination square is empty

            if(currentPawnMoves && is7thRank(pieceBB)){ //check for double pawn move, if single pawn move exists and pawn is on 2nd rank
                currentPawnMoves |= pieceBB << SS;
                currentPawnMoves &= ~helpBitboards[OCCUPIED_SQUARES]; // again make sure destination square is empty
            }

            // generate capture moves
            if(notAFile(pieceBB)) currentPawnCaptureMoves |= pieceBB << SW;
            if(notHFile(pieceBB)) currentPawnCaptureMoves |= pieceBB << SE;
            enPassantMove = currentPawnCaptureMoves & bitboards[WHITE][EN_PASSANT_SQUARES];
            currentPawnCaptureMoves &= bitboards[WHITE][PIECES];
        }

        // en passant moves
        if(enPassantMove){
            bitboardsToLegalMovelist(movelist, pieceBB, 0, enPassantMove, false, true);
        }

        // update movelist
        bitboardsToLegalMovelist(movelist, pieceBB, currentPawnMoves, currentPawnCaptureMoves);
    }
}



void Position::GenerateKnightMoves(moveList &movelist) {

    uint64_t knights;
    knights = bitboards[this->turn][KNIGHTS];

    unsigned pieceIndex;
    uint64_t pieceBB, currentKnightMoves, currentKnightCaptures; //pieceBitboard

    while (knights != 0) {

        //grabs the first knight and returns it's index. This knight also gets removed from the knights variable bibboard
        // so in the next loop the next knight will be returned.
        pieceIndex = debruijnSerialization(knights);
        pieceBB = 1uLL << pieceIndex;

        knights &= ~pieceBB; //remove knight from knights bitboard

        currentKnightMoves = knightAttacks(pieceBB); //get the knight attacks (moveGenHelpFunctions.h)


        // use knightmoves AND NOT white pieces to remove blocked squares
        currentKnightMoves &= ~bitboards[this->turn][PIECES];

        // use knightmoves AND opponent pieces to get capture moves (storing capturemoves differently is efficient for the search tree)
        currentKnightCaptures = currentKnightMoves & bitboards[!this->turn][PIECES];


        // remove capturemoves from the normal moves bitboard
        currentKnightMoves  &= ~currentKnightCaptures;

        bitboardsToLegalMovelist(movelist, pieceBB, currentKnightMoves, currentKnightCaptures);

    }
}



void Position::GenerateSliderMoves(moveList &movelist){

    unsigned pieceIndex;
    uint64_t pieceBB, currentPieceMoves, currentPieceCaptures;
    int sliders[] = {BISHOPS, ROOKS, QUEENS}; // sliders to loop over
    for(int currentSlider: sliders) {
        uint64_t currentPiece;
        currentPiece = bitboards[this->turn][currentSlider];
        while (currentPiece != 0) {
            pieceIndex = debruijnSerialization(currentPiece);
            pieceBB = 1uLL << pieceIndex;

            currentPiece &= ~pieceBB; // remove single piece from current pieceBB

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

            bitboardsToLegalMovelist(movelist, pieceBB, currentPieceMoves, currentPieceCaptures);
        }
    }
}

void Position::GenerateKingMoves(moveList &movelist){
    uint64_t king = bitboards[this->turn][KING];

    uint64_t kingMoves = kingAttacks(king);

    kingMoves &= ~bitboards[this->turn][PIECES];
    uint64_t kingCaptureMoves = kingMoves & bitboards[!this->turn][PIECES];
    kingMoves &= ~kingCaptureMoves;

    bitboardsToLegalMovelist(movelist, king, kingMoves, kingCaptureMoves, true);

}


void Position::GenerateCastlingMoves(moveList &movelist) {

    if(this->turn == WHITE) {
        if (castlingRights & WHITE_KINGSIDE_CASTLING_RIGHTS) {
            CastlingToMovelist(movelist, WHITE_KINGSIDE_CASTLING_RIGHTS, WHITE_KINGSIDE_CASTLING_EMPTY_AND_NONATTACKED_SQUARES, WHITE_KINGSIDE_CASTLING_EMPTY_AND_NONATTACKED_SQUARES);
        }
        if (castlingRights & WHITE_QUEENSIDE_CASTLING_RIGHTS) {
            CastlingToMovelist(movelist, WHITE_QUEENSIDE_CASTLING_RIGHTS, WHITE_QUEENSIDE_CASTLING_EMPTY, WHITE_QUEENSIDE_CASTLING_NONATTACKED);
        }
    }
    else{
        if(castlingRights & BLACK_KINGSIDE_CASTLING_RIGHTS){
            CastlingToMovelist(movelist, BLACK_KINGSIDE_CASTLING_RIGHTS, BLACK_KINGSIDE_CASTLING_EMPTY_AND_NONATTACKED_SQUARES, BLACK_KINGSIDE_CASTLING_EMPTY_AND_NONATTACKED_SQUARES);
        }
        if (castlingRights & BLACK_QUEENSIDE_CASTLING_RIGHTS) {
            CastlingToMovelist(movelist, BLACK_QUEENSIDE_CASTLING_RIGHTS, BLACK_QUEENSIDE_CASTLING_EMPTY, BLACK_QUEENSIDE_CASTLING_NONATTACKED);
        }
    }
}

/*
 * Checks legality of castling move and puts into movelist if legal
 */
void Position::CastlingToMovelist(moveList &movelist, unsigned castlingType, uint64_t empty, uint64_t nonattacked){

    if(!(empty & helpBitboards[OCCUPIED_SQUARES])) { // check if the squares are empty
        unsigned nonAttackedInt;
        bool illegalFlag = false;
        unsigned move;
        // loop over the squares that should be non attacked
        while(nonattacked){
            nonAttackedInt = debruijnSerialization(nonattacked);
            uint64_t nonAttackedBB = 1uLL << nonAttackedInt;
            if(squareAttacked(nonAttackedBB, this->turn)){
                illegalFlag = true;
                break; // TODO: change to return and remove illegalflag
            }
            nonattacked &= ~nonAttackedBB;
        }

        if(!illegalFlag) {
            move = CASTLING_FLAG << SPECIAL_MOVE_FLAG_SHIFT;
            move |= castlingType << ORIGIN_SQUARE_SHIFT; // put castling type into origin square bits as they are not used
            movelist.move[movelist.moveLength++] = move;
        }
    }
};

// pinnedOrigin is the location that will be checked for pins (e.g. pinnedOrigin is king for legality check)
// pinned pieces are of colour $colour
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

        blockers &= ~singleBlocker; // remove singleBlocker from blockers bb

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

        blockers &= ~singleBlocker; // remove singleBlocker from blockers bb

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
    if(colour == WHITE){ // pawns going south (because black does attacking), so squareAttacked "attacks" north, so right shift
        if(notAFile(square) && (square >> NW & bitboards[BLACK][PAWNS])) return true;
        if(notHFile(square) && (square >> NE & bitboards[BLACK][PAWNS])) return true;
    }
    else{
        if(notAFile(square) && (square << SW & bitboards[WHITE][PAWNS])) return true;
        if(notHFile(square) && (square << SE & bitboards[WHITE][PAWNS])) return true;
    }

    return false;

}

/* This function converts a origin bitboard and destination bitboard into a movelist.
 * The function assumes the moves are pseudo legal, and performs a legality check to make sure that the king is not
 * left in check after the move has been made.
 *
 * For storing moves the following approach is used: (similar to stockfish)
 * bit 0-5 === origin square;
 * bit 6-11 === destination square;
 * bit 12-13 === promotion piece type (N, B, R, Q)
 * bit 14-15 === special move flag, promotion, en passant, castling
 *
 *
 */

void Position::bitboardsToLegalMovelist(moveList &movelist, uint64_t origin, uint64_t destinations, uint64_t captureDestinations, bool kingMoveFlag, bool enPassantMoveFlag) {
    unsigned originInt, destinationInt, move;
    uint64_t destinationBB;
    originInt = debruijnSerialization(origin);
    origin |= 1uLL << originInt; // restore origin as debruijnSerialization removes this bit by default

    // check if the piece is pinned
    bool pinnedFlag = false;
    if(origin & helpBitboards[PINNED_PIECES]) pinnedFlag = true;

    while(destinations != 0){
        // get integer of destination position
        destinationInt = debruijnSerialization(destinations);


        destinationBB = 1uLL << destinationInt;

        destinations &= ~destinationBB;

        // generate the move
        move = originInt << ORIGIN_SQUARE_SHIFT;
        move |= destinationInt << DESTINATION_SQUARE_SHIFT;

        if(enPassantMoveFlag){
            move |= EN_PASSANT_FLAG << SPECIAL_MOVE_FLAG_SHIFT;
        }

        if(pinnedFlag){
            unsigned kingInt = debruijnSerialization(bitboards[this->turn][KING]);
            // if the piece does not move in the same line as the king, the move is illegal, thus continue with the next move
            if(rayDirectionLookup(kingInt, originInt) != rayDirectionLookup(originInt, destinationInt)) continue;
        }

        if(isIncheck || enPassantMoveFlag || kingMoveFlag){
            doMove(move);
            // check if the king is attacked after this move
            if(squareAttacked(bitboards[!this->turn][KING], !this->turn)){
                undoMove();
                continue;
            }
            undoMove();
        }

        movelist.move[movelist.moveLength++] = move;

    }

    // capture moves
    while(captureDestinations != 0){
        destinationInt = debruijnSerialization(captureDestinations);

        captureDestinations &= ~(1uLL << destinationInt);

        // generate the move
        move = originInt << ORIGIN_SQUARE_SHIFT;
        move |= destinationInt << DESTINATION_SQUARE_SHIFT;

        if(enPassantMoveFlag){
            move |= EN_PASSANT_FLAG << SPECIAL_MOVE_FLAG_SHIFT;
        }

        if(pinnedFlag){
            unsigned kingInt = debruijnSerialization(bitboards[this->turn][KING]);
            // if the piece does not move in the same line as the king, the move is illegal, thus continue with the next move
            if(rayDirectionLookup(kingInt, originInt) != rayDirectionLookup(originInt, destinationInt)) continue;
        }


        if(isIncheck || enPassantMoveFlag || kingMoveFlag){
            doMove(move);
            // check if the king is attacked after this move
            if(squareAttacked(bitboards[!this->turn][KING], !this->turn)){
                undoMove();
                continue;
            }
            undoMove();
        }
        movelist.captureMove[movelist.captureMoveLength++] = move;
    }
}




/*
 * This method checks which piece occupies originInt, and moves that piece to destinationInt. It does NOT check captures.
 * It DOES set the en passant bit in case of double pawn move
 * It also sets removes castling rights in case of king move
 * @param originInt origin bitboard
 * @param destinationInt destination bitboard
 * @param colour specify which colour moves (speeds up the search for the piece)
 */
void Position::MovePiece(uint64_t originBB, uint64_t destinationBB, bool colour) {

    // find the piece that is moved.
    int pieceToMove = -1;
    for (int i = 0; i < 6; i++) {
        if (bitboards[colour][i] & originBB) {
            pieceToMove = i;
            break;
        }
    }

    // TODO: move rook castling right removal to here, using if(rook == originBB | desitinationBB construction)

    // remove castling rights if king is moved
    // note that in case of moving the rook, the removal of castling rights is handled in the doMove function.
    if (pieceToMove == KING) {
        if (colour == WHITE) {
            castlingRights &= ~WHITE_CASTLING_RIGHTS;
        } else {
            castlingRights &= ~BLACK_CASTLING_RIGHTS;
        }
    }

    // set en passant square in case of double pawn move
    if (pieceToMove == PAWNS) {
        if (colour == WHITE) {
            if ((originBB >> NN) & destinationBB) {
                bitboards[WHITE][EN_PASSANT_SQUARES] = originBB >> N;
            }
        } else {
            if ((originBB << SS) & destinationBB) {
                bitboards[BLACK][EN_PASSANT_SQUARES] = originBB << S;
            }
        }
    }

    // remove origin piece
    bitboards[colour][pieceToMove] &= ~originBB;

    // add destination piece
    bitboards[colour][pieceToMove] |= destinationBB;

}

// doMove does the move and stores move information in the previesMoves array
// other than the 15 bits used in the bitboardsToMoveList convention the following bits are used:
// bit 16 capturemoveflag
// bit 17-19 captured piece
// bit 20-25 captured piece index
// bit 26-31 en passant square index
// bit 32-35 castling rights before this move
// in definitions.h the enum can be found for the shifts and the masks

void Position::doMove(unsigned move){
    unsigned originInt, destinationInt, enPassantInt;
    uint64_t originBB, destinationBB;

    // as the previousmoves array uses uint64_t to store a move
    uint64_t moveL = move;

    originInt = (ORIGIN_SQUARE_MASK & moveL) >> ORIGIN_SQUARE_SHIFT;
    destinationInt = (DESTINATION_SQUARE_MASK & moveL) >> DESTINATION_SQUARE_SHIFT;
    originBB = 1uLL << originInt;
    destinationBB = 1uLL << destinationInt;


    // store the castling rights of before the move
    moveL |= uint64_t(castlingRights) << CASTLING_RIGHTS_BEFORE_MOVE_SHIFT;

    switch((SPECIAL_MOVE_FLAG_MASK & moveL) >> SPECIAL_MOVE_FLAG_SHIFT){
        case EN_PASSANT_FLAG:
            if(this->turn == WHITE){ // remove pawn south of destination square, so left shift
                bitboards[BLACK][PAWNS] &= ~(destinationBB << S);
            }
            else{
                bitboards[WHITE][PAWNS] &= ~(destinationBB >> N);
            }
            MovePiece(originBB, destinationBB, this->turn);


            // store destination square in the previousMoveList
            enPassantInt = debruijnSerialization(bitboards[!this->turn][EN_PASSANT_SQUARES]);
            moveL |= enPassantInt << EN_PASSANT_DESTINATION_SQUARE_SHIFT;
            bitboards[!this->turn][EN_PASSANT_SQUARES] = 0;

            break;
        case CASTLING_FLAG:
            if(((ORIGIN_SQUARE_MASK & moveL) >> ORIGIN_SQUARE_SHIFT) & KINGSIDE_CASTLING){ // kingside castling
                doCastlingMove(true); // true is kingside
            }
            else{
                doCastlingMove(false); // false is queenside
            }
            if(this->turn == WHITE){
                castlingRights &= ~WHITE_CASTLING_RIGHTS;
            }
            else{
                castlingRights &= ~BLACK_CASTLING_RIGHTS;
            }
            break;
        default:
            MovePiece(originBB, destinationBB, this->turn);

            // switch to opponent bitboards and remove possible destination piece in case of capture
            // also save this piece in the previousMoves array as it needs to be restored in case of undoMove
            uint64_t capturedPiece;
            for(unsigned i = 0; i < 6; i++){
                capturedPiece = bitboards[!this->turn][i] & destinationBB;
                if(capturedPiece) {
                    bitboards[!this->turn][i] &= ~capturedPiece;

                    // store the capture move specifications
                    moveL |= 1uLL << CAPTURE_MOVE_FLAG_SHIFT;
                    moveL |= i << CAPTURED_PIECE_TYPE_SHIFT;
                    moveL |= debruijnSerialization(capturedPiece) << CAPTURED_PIECE_INDEX_SHIFT;
                    break;
                }
            }
            break;
    }

    // reset castling rights in case the rook has moved or has been captured.
    if(!(bitboards[WHITE][ROOKS] & (1uLL << SQ_H1))){
        castlingRights &= ~WHITE_KINGSIDE_CASTLING_RIGHTS;
    }
    else if(!(bitboards[WHITE][ROOKS] & (1uLL << SQ_A1))){
        castlingRights &= ~WHITE_QUEENSIDE_CASTLING_RIGHTS;
    }
    else if(!(bitboards[BLACK][ROOKS] & (1uLL << SQ_H8))){
        castlingRights &= ~BLACK_KINGSIDE_CASTLING_RIGHTS;
    }
    else if(!(bitboards[BLACK][ROOKS] & (1uLL << SQ_A8))){
        castlingRights &= ~BLACK_QUEENSIDE_CASTLING_RIGHTS;
    }


    // remove en passant destination square
    bitboards[!this->turn][EN_PASSANT_SQUARES] = 0;

    // put into previous moves list
    this->previousMoves[this->halfMoveNumberTotal++] = moveL;

    this->turn = !this->turn;
    generateHelpBitboards();

}

void Position::doMove(char *move) {
    unsigned moveUnsigned = strToMoveNotation(move);

    // in case of castling
    if(this->turn == WHITE){
        if(bitboards[WHITE][KING] & (1uLL << SQ_E1)){
            if(string(move) == "e1g1"){
                moveUnsigned = CASTLING_FLAG << SPECIAL_MOVE_FLAG_SHIFT;
                moveUnsigned |= KINGSIDE_CASTLING << ORIGIN_SQUARE_SHIFT;
            }
            else if(string(move) == "e1c1"){
                moveUnsigned = CASTLING_FLAG << SPECIAL_MOVE_FLAG_SHIFT;
                moveUnsigned |= QUEENSIDE_CASTLING << ORIGIN_SQUARE_SHIFT;
            }
        }
    }
    else{

        if(bitboards[BLACK][KING] & (1uLL << SQ_E8)){
            if(string(move) == "e8g8"){
                moveUnsigned = CASTLING_FLAG << SPECIAL_MOVE_FLAG_SHIFT;
                moveUnsigned |= KINGSIDE_CASTLING << ORIGIN_SQUARE_SHIFT;
            }
            else if(string(move) == "e8c8"){
                moveUnsigned = CASTLING_FLAG << SPECIAL_MOVE_FLAG_SHIFT;
                moveUnsigned |= QUEENSIDE_CASTLING << ORIGIN_SQUARE_SHIFT;
            }
        }
    }

    doMove(moveUnsigned);
}


void Position::doCastlingMove(bool side){
    unsigned shift = 0;
    if(this->turn == WHITE){
        shift = 56; //
    }

    if(side){ // kingside
        bitboards[this->turn][ROOKS] &= ~(1uLL << (7u + shift)); // remove rook
        bitboards[this->turn][ROOKS] |=  (1uLL << (5u + shift)); // add rook
        bitboards[this->turn][KING] = bitboards[this->turn][KING] << 2u; // move king
    }
    else{ // queenside
        bitboards[this->turn][ROOKS] &= ~(1uLL << (0u + shift)); // remove rook
        bitboards[this->turn][ROOKS] |=  (1uLL << (3u + shift)); // add rook
        bitboards[this->turn][KING] = bitboards[this->turn][KING] >> 2u; // move king
    }
}

void Position::undoCastlingMove(bool side) {
    unsigned shift = 0;
    if (!this->turn == WHITE) {
        shift = 56; //
    }

    if (side) { // kingside
        bitboards[!this->turn][ROOKS] |= (1uLL << (7u + shift)); // add rook
        bitboards[!this->turn][ROOKS] &= ~(1uLL << (5u + shift)); // remove rook
        bitboards[!this->turn][KING] = bitboards[!this->turn][KING] >> 2u; // move king
    } else { // queenside
        bitboards[!this->turn][ROOKS] |= (1uLL << (0u + shift)); // add rook
        bitboards[!this->turn][ROOKS] &= ~(1uLL << (3u + shift)); // remove rook
        bitboards[!this->turn][KING] = bitboards[!this->turn][KING] << 2u; // move king
    }
}

// undo the last made move
void Position::undoMove() {
    unsigned originInt, destinationInt, enPassantInt;

    // get the move
    uint64_t move = this->previousMoves[this->halfMoveNumberTotal - 1];
    // revert back to 0
    this->previousMoves[--this->halfMoveNumberTotal] = 0;

    originInt = (move & ORIGIN_SQUARE_MASK) >> ORIGIN_SQUARE_SHIFT;
    destinationInt = (move & DESTINATION_SQUARE_MASK) >> DESTINATION_SQUARE_SHIFT;

    uint64_t originBB = 1uLL << originInt;
    uint64_t destinationBB = 1uLL << destinationInt;



    switch((SPECIAL_MOVE_FLAG_MASK & move) >> SPECIAL_MOVE_FLAG_SHIFT){
        case EN_PASSANT_FLAG:
            if(this->turn == BLACK){ // add pawn south of destination square, so left shift (as we undo whites move if it is blacks turn)
                bitboards[BLACK][PAWNS] |= (destinationBB << S);
            }
            else{
                bitboards[WHITE][PAWNS] |= (destinationBB >> N);
            }
            // move the piece from destination to origin (so a possible wrong order warning is expected)
            MovePiece(destinationBB, originBB, !this->turn);

            // restore en passant destination square
            enPassantInt = (EN_PASSANT_DESTINATION_SQUARE_MASK & move) >> EN_PASSANT_DESTINATION_SQUARE_SHIFT;
            bitboards[this->turn][EN_PASSANT_SQUARES] = 1uLL << enPassantInt;

            // reset en passant destination square of other side
            bitboards[!this->turn][EN_PASSANT_SQUARES] = 0;
            break;

        case CASTLING_FLAG:
            if(((ORIGIN_SQUARE_MASK & move) >> ORIGIN_SQUARE_SHIFT) & KINGSIDE_CASTLING){ // kingside castling
                undoCastlingMove(true); // true is kingside
            }
            else{ // queenside castling
                undoCastlingMove(false); // false is queenside
            }
            break;
        default:
            // move the piece from destination to origin (so a possible wrong order warning is expected)
            MovePiece(destinationBB, originBB, !this->turn);
            break;
    }

    // put the captured piece back
    if(move & CAPTURE_MOVE_FLAG_MASK){
        unsigned pieceIndex = (move & CAPTURED_PIECE_INDEX_MASK) >> CAPTURED_PIECE_INDEX_SHIFT;
        unsigned pieceType = (move & CAPTURED_PIECE_TYPE_MASK) >> CAPTURED_PIECE_TYPE_SHIFT;
        bitboards[this->turn][pieceType] |= 1uLL << pieceIndex;
    }

    // restore castling rights
    castlingRights = (move & CASTLING_RIGHTS_BEFORE_MOVE_MASK) >> CASTLING_RIGHTS_BEFORE_MOVE_SHIFT;

    this->turn = !this->turn;
    generateHelpBitboards();

}


perftCounts Position::PERFT(int depth, bool tree){

    perftCounts pfcount, pfcountTemp;

    definitions::moveList movelist;

    uint64_t totalMoves = 0, captureMoves = 0, normalMoves = 0;

    GenerateMoves(movelist);

    if(depth == 1){

        captureMoves = movelist.captureMoveLength;
        normalMoves = movelist.moveLength;

        pfcount.captures = captureMoves;
        pfcount.normal = normalMoves;
        pfcount.total = normalMoves + captureMoves;

        if(tree){
            for(int i = 0; i < movelist.moveLength; i++){
                cout << moveToStrNotation(movelist.move[i]) << " " << 1 << "\n";
            }
            for(int i = 0; i < movelist.captureMoveLength; i++){
                cout << moveToStrNotation(movelist.captureMove[i]) << " " << 1 << "\n";
            }
        }

        return pfcount;
    }

    for(int i = 0; i < movelist.moveLength; i++){
        doMove(movelist.move[i]);
        pfcountTemp = PERFT(depth - 1, false);
        if(tree) {
            cout << moveToStrNotation(movelist.move[i]) << " " << pfcountTemp.total << "\n";
        }
        captureMoves += pfcountTemp.captures;
        normalMoves += pfcountTemp.normal;
        totalMoves += pfcountTemp.total;
        undoMove();
    }

    for(int i = 0; i < movelist.captureMoveLength; i++){
        doMove(movelist.captureMove[i]);
        pfcountTemp = PERFT(depth - 1, false);
        if(tree) {
            cout << moveToStrNotation(movelist.captureMove[i]) << " " << pfcountTemp.total << "\n";
        }
        captureMoves += pfcountTemp.captures;
        normalMoves += pfcountTemp.normal;
        totalMoves += pfcountTemp.total;
        undoMove();
    }

    pfcount.total = totalMoves;
    pfcount.normal = normalMoves;
    pfcount.captures = captureMoves;

    return pfcount;
}


