
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

Position::Position(string FEN) {

    string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    if (FEN == "startpos") FEN = startFEN;


    // loop through the FEN string until first space
    unsigned BBindex = 0; // this gets incremented when the FEN contains numbers as well
    char pieceChar = FEN.at(0);
    int i = 0; // initialize here as it is also needed outside of the scope of the for loop

    if (FEN.at(i) == '"') i++; // skip possible leading quotation marks (in case of perft debug)

    for (i; pieceChar != ' '; i++, BBindex++, pieceChar = FEN.at(i)) { // loop for first part of FEN until first space
        if (pieceChar == '/') {
            BBindex--; // account for (incorrect) increment, as it is simply a new row
            continue;
        }
        if (pieceChar <= '9' && pieceChar > '0') {
            BBindex += pieceChar - '1';
            continue;
        }

        bitboards[isupper(pieceChar) != 0][FENpieces.at(tolower(pieceChar))] |=
                1uLL << BBindex; // shift the bits into place
    }

    if (FEN.at(++i) == 'b') {
        this->turn = BLACK;
    } // turn gets initialized as white's turn by default

    //move on to castling rights
    ++i;

    while (FEN.at(++i) != ' ') {
        switch (FEN.at(i)) {
            case 'K':
                this->castlingRights |= WHITE_KINGSIDE_CASTLING_RIGHTS;
                break;
            case 'Q':
                this->castlingRights |= WHITE_QUEENSIDE_CASTLING_RIGHTS;
                break;
            case 'k':
                this->castlingRights |= BLACK_KINGSIDE_CASTLING_RIGHTS;
                break;
            case 'q':
                this->castlingRights |= BLACK_QUEENSIDE_CASTLING_RIGHTS;
                break;
            default:
                break;
        }
    }

    if (FEN.at(++i) != '-') {
        unsigned enPassantInt = FEN.at(i) - 'a';
        enPassantInt += (8 - (FEN.at(++i) - '0')) * 8;
        bitboards[!this->turn][EN_PASSANT_SQUARES] = 1uLL << enPassantInt;
    }

    ++i;

    char halfMove50[3];
    int hmi = 0;
    try {
        while (FEN.at(++i) != ' ') {
            halfMove50[hmi] = FEN.at(i);
        }
        // used to check 50 move rule
        this->halfMoveNumber50 = stoi(halfMove50);

        char fullMove[5];
        int fmi = 0;
        while (++i < FEN.length() && FEN.at(i) != '"') {
            fullMove[fmi] = FEN.at(i);
            fmi++;
        }

        this->fullMoveNumber = stoi(fullMove);

    }
    catch (out_of_range) {
        this->halfMoveNumber50 = 0;
        this->fullMoveNumber = 0;
    }
    generateHelpBitboards();

    sliderAttacks = SliderAttacks();
    sliderAttacks.Initialize(); // initialize slider attacks

    rayDirectionLookupInitialize(); // initialize ray lookup table;

    zobristPieceTableInitialize(); // initialize zobrist hash table

    // calculate the hash of the current position;
    for (int colour = 0; colour < 2; colour++) {
        for (int piece = 0; piece < 6; piece++) {
            int pieceIndex = debruijnSerialization(bitboards[colour][piece]);
            positionHashes[halfMoveNumber] ^= zobristPieceTable[colour][piece][pieceIndex];
        }
    }
    positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
    if(this->turn == BLACK){
        positionHashes[halfMoveNumber] ^= zobristBlackToMove;
    }
    if(bitboards[!this->turn][EN_PASSANT_SQUARES]){
        int enPassantSquare = debruijnSerialization(bitboards[!this->turn][EN_PASSANT_SQUARES]);
        positionHashes[halfMoveNumber] ^= zobristEnPassantFile[enPassantSquare % 8];
    }
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
    if(!isIncheck) {
        GenerateCastlingMoves(movelist);
    }
    GenerateKnightMoves(movelist);
    GenerateSliderMoves(movelist);
    GenerateKingMoves(movelist);
}

void Position::GeneratePawnMoves(moveList &movelist) {
    uint64_t pawns;

    pawns = bitboards[this->turn][PAWNS];

    unsigned pieceIndex;
    uint64_t pieceBB;
    while(pawns != 0){
        uint64_t currentPawnMoves, currentPawnCaptureMoves = 0, enPassantMove;
        bool promotionMoveFlag = false;

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

            // check if the pawn is on the 7th rank, meaning that any move is a promotion move
            if(is7thRank(pieceBB)){ //
                promotionMoveFlag = true;
            }


        }
        else{ //black's turn, pawns going south so left shift
            currentPawnMoves = pieceBB << S;
            currentPawnMoves &= ~helpBitboards[OCCUPIED_SQUARES]; // make sure destination square is empty

            if(currentPawnMoves && is7thRank(pieceBB)){ //check for double pawn move, if single pawn move exists and pawn is on 7th rank
                currentPawnMoves |= pieceBB << SS;
                currentPawnMoves &= ~helpBitboards[OCCUPIED_SQUARES]; // again make sure destination square is empty
            }

            // generate capture moves
            if(notAFile(pieceBB)) currentPawnCaptureMoves |= pieceBB << SW;
            if(notHFile(pieceBB)) currentPawnCaptureMoves |= pieceBB << SE;
            enPassantMove = currentPawnCaptureMoves & bitboards[WHITE][EN_PASSANT_SQUARES];
            currentPawnCaptureMoves &= bitboards[WHITE][PIECES];

            // check if the pawn is on the 2nd rank, meaning that any move is a promotion move
            if(is2ndRank(pieceBB)){ //
                promotionMoveFlag = true;
            }
        }

        // en passant moves
        if(enPassantMove){
            bitboardsToLegalMovelist(movelist, pieceBB, 0, enPassantMove, false, true);
        }

        // update movelist
        bitboardsToLegalMovelist(movelist, pieceBB, currentPawnMoves, currentPawnCaptureMoves, false, false, promotionMoveFlag);
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

            // remove moves blocked by same side pieces
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
                return;
            }
            nonattacked &= ~nonAttackedBB;
        }


        move = CASTLING_FLAG << SPECIAL_MOVE_FLAG_SHIFT;
        move |= castlingType << ORIGIN_SQUARE_SHIFT; // put castling type into origin square bits as they are not used
        movelist.move[movelist.moveLength++] = move;

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

    // check opposing king attacks
    if(kingAttacks(square) & bitboards[!colour][KING]) return true;

    return false;

}

/* This function converts a origin bitboard and destinations bitboard into a movelist (appends to the variable movelist).
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

void Position::bitboardsToLegalMovelist(moveList &movelist, uint64_t origin, uint64_t destinations, uint64_t captureDestinations, bool kingMoveFlag, bool enPassantMoveFlag, bool promotionMoveFlag) {
    unsigned originInt, destinationInt, move;
    uint64_t destinationBB;
    originInt = debruijnSerialization(origin);
    origin |= 1uLL << originInt; // restore origin as debruijnSerialization removes this bit by default

    if(this->castlingRights == 4111){
        cout << "stop";
    }

    // check if the piece is pinned
    bool pinnedFlag = false;
    if(origin & helpBitboards[PINNED_PIECES]) pinnedFlag = true;

    // combine destinations and loop over them (check can be made later to see if it is a capture move)
    uint64_t allDestinations = destinations | captureDestinations;
    while(allDestinations != 0){
        // get integer of destination position
        destinationInt = debruijnSerialization(allDestinations);
        destinationBB = 1uLL << destinationInt;

        allDestinations &= ~destinationBB;

        // generate the move
        move = originInt << ORIGIN_SQUARE_SHIFT;
        move |= destinationInt << DESTINATION_SQUARE_SHIFT;

        if(enPassantMoveFlag){
            move |= EN_PASSANT_FLAG << SPECIAL_MOVE_FLAG_SHIFT;
        }

        // check if piece is pinned and check legality of move.
        if(pinnedFlag){
            unsigned kingInt = debruijnSerialization(bitboards[this->turn][KING]);
            // if the piece does not move in the same line as the king, the move is illegal, thus continue with the next move
            if(rayDirectionLookup(kingInt, originInt) != rayDirectionLookup(originInt, destinationInt)) {
                continue;
            }
        }

        // for these types of moves, simply checking if the piece is pinned does not work, so the move has to be done and
        // subsequently checked if the king is not in check.
        if(isIncheck || enPassantMoveFlag || kingMoveFlag){
            doMove(move);
            // check if the king is attacked after this move
            if(squareAttacked(bitboards[!this->turn][KING], !this->turn)){
                undoMove();
                continue;
            }
            undoMove();
        }

        if(promotionMoveFlag){
            unsigned baseMove = move | (PROMOTION_FLAG << SPECIAL_MOVE_FLAG_SHIFT);
            // cycle over different promotion pieces (N B R Q)
            for(int promotionType = 0; promotionType < 4; promotionType++){
                move = baseMove | promotionType << PROMOTION_TYPE_SHIFT;

                if(destinationBB & captureDestinations){
                    movelist.captureMove[movelist.captureMoveLength++] = move;
                }
                else {
                    movelist.move[movelist.moveLength++] = move;
                }

            }
        }
        else {
            if (destinationBB & captureDestinations) {
                movelist.captureMove[movelist.captureMoveLength++] = move;
            } else {
                movelist.move[movelist.moveLength++] = move;
            }
        }
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

    if(pieceToMove == -1){
        std::cout << "DANGER IN MOVEPIECE\n";
    }

    // TODO: move rook castling right removal to here, using if(rook == originBB | destinationBB construction)

    // remove castling rights if king is moved and update hash
    // note that in case of moving the rook, the removal of castling rights is handled in the doMove function.
    if (pieceToMove == KING) {
        if (colour == WHITE) {
            positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
            castlingRights &= ~WHITE_CASTLING_RIGHTS;
            positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
        } else {
            positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
            castlingRights &= ~BLACK_CASTLING_RIGHTS;
            positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
        }
    }

    // set en passant square in case of double pawn move and update positionHashes[halfMoveNumber]
    if (pieceToMove == PAWNS) {
        if (colour == WHITE) {
            if ((originBB >> NN) & destinationBB) {
                bitboards[WHITE][EN_PASSANT_SQUARES] = originBB >> N;
                // update hash
                positionHashes[halfMoveNumber] ^= zobristEnPassantFile[debruijnSerialization(originBB >> N) % 8];
            }
        } else {
            if ((originBB << SS) & destinationBB) {
                bitboards[BLACK][EN_PASSANT_SQUARES] = originBB << S;
                // update hash
                positionHashes[halfMoveNumber] = zobristEnPassantFile[debruijnSerialization(originBB << S) % 8];
            }
        }
    }

    // remove origin piece
    bitboards[colour][pieceToMove] &= ~originBB;
    // update hash by removing origin piece
    positionHashes[halfMoveNumber] ^= zobristPieceTable[colour][pieceToMove][debruijnSerialization(originBB)];

    // add destination piece
    bitboards[colour][pieceToMove] |= destinationBB;
    // update hash by adding destination piece
    positionHashes[halfMoveNumber] ^= zobristPieceTable[colour][pieceToMove][debruijnSerialization(destinationBB)];
}

// doMove does the move and stores move information in the previesMoves array
// other than the 15 bits used in the bitboardsToMoveList convention the following bits are used:
// bit 16 capturemoveflag
// bit 17-19 captured piece
// bit 20-25 captured piece index
// bit 26-31 en passant square index
// bit 32-35 castling rights before this move
// bit 36-42 halfmove number for the 50 move rule (gets reset in case of pawn or capture move)
// bit 43-48 halfmoves since the last time an irreversible move occured
// in definitions.h the enum can be found for the shifts and the masks

void Position::doMove(unsigned move){
    unsigned originInt, destinationInt, enPassantInt;
    uint64_t originBB, destinationBB;

    // as the previousmoves array uses uint64_t to store a move. So moveL(arge) is the variable for previousmoves list
    uint64_t moveL = move;

    // store halfmovenumbers for 50 move repetition rule
    uint64_t temp = halfMoveNumber50;
    moveL |= temp << HALFMOVENUMBER_BEFORE_MOVE_SHIFT;

    // store halfmovenumbers since the last irreversible move (for 3fold repetion check)
    temp = halfMovesSinceIrrepr;
    moveL |= temp << HALFMOVENUMBER_SINCE_IRREVERSIBLE_MOVE_SHIFT;

    // increment halfmovenumber counters
    halfMoveNumber50++;
    halfMoveNumber++;
    halfMovesSinceIrrepr++;

    positionHashes[halfMoveNumber] = positionHashes[halfMoveNumber - 1];

    originInt = (ORIGIN_SQUARE_MASK & moveL) >> ORIGIN_SQUARE_SHIFT;
    destinationInt = (DESTINATION_SQUARE_MASK & moveL) >> DESTINATION_SQUARE_SHIFT;
    originBB = 1uLL << originInt;
    destinationBB = 1uLL << destinationInt;

    // store the castling rights of before the move
    moveL |= uint64_t(castlingRights) << CASTLING_RIGHTS_BEFORE_MOVE_SHIFT;

    // used for the promotion special move case
    unsigned promotionType;
    unsigned promotionIndices[] = {KNIGHTS, BISHOPS, ROOKS, QUEENS};

    uint64_t capturedPiece;
    switch((SPECIAL_MOVE_FLAG_MASK & moveL) >> SPECIAL_MOVE_FLAG_SHIFT){
        case EN_PASSANT_FLAG:
            halfMovesSinceIrrepr = 0; // reset for 3fold repetition
            halfMoveNumber50 = 0; // reset for 50 move rule
            if(this->turn == WHITE){ // remove pawn south of destination square, so left shift
                bitboards[BLACK][PAWNS] &= ~(destinationBB << S);

                // remove pawn from zobrist hash
                unsigned EPcapture = debruijnSerialization(destinationBB << S);
                positionHashes[halfMoveNumber] ^= zobristPieceTable[BLACK][PAWNS][EPcapture];
            }
            else{
                bitboards[WHITE][PAWNS] &= ~(destinationBB >> N);

                // remove pawn from zobrist hash
                unsigned EPcapture = debruijnSerialization(destinationBB >> N);
                positionHashes[halfMoveNumber] ^= zobristPieceTable[WHITE][PAWNS][EPcapture];
            }
            MovePiece(originBB, destinationBB, this->turn);
            break;
        case CASTLING_FLAG:
            halfMovesSinceIrrepr = 0; // reset for 3fold repetition
            if(((ORIGIN_SQUARE_MASK & moveL) >> ORIGIN_SQUARE_SHIFT) & KINGSIDE_CASTLING){ // kingside castling
                doCastlingMove(true); // true is kingside
            }
            else{
                doCastlingMove(false); // false is queenside
            }
            if(this->turn == WHITE){
                // update hash and castling rights
                positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
                castlingRights &= ~WHITE_CASTLING_RIGHTS;
                positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
            }
            else{
                positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
                castlingRights &= ~BLACK_CASTLING_RIGHTS;
                positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
            }
            break;
        case PROMOTION_FLAG:
            halfMovesSinceIrrepr = 0; // reset for 3fold repetition
            halfMoveNumber50 = 0; // reset for 50 move rule
            // remove the pawn and update hash
            bitboards[this->turn][PAWNS] &= ~originBB;
            positionHashes[halfMoveNumber] ^= zobristPieceTable[this->turn][PAWNS][originInt];

            // place promoted piece and update positionHashes[halfMoveNumber]
            promotionType = promotionIndices[(move & PROMOTION_TYPE_MASK) >> PROMOTION_TYPE_SHIFT];
            bitboards[this->turn][promotionType] |= destinationBB;
            positionHashes[halfMoveNumber] ^= zobristPieceTable[this->turn][promotionType][destinationInt];

            // switch to opponent bitboards and remove possible destination piece in case of capture
            // also save this piece in the previousMoves array as it needs to be restored in case of undoMove
            for(unsigned i = 0; i < 6; i++){
                capturedPiece = bitboards[!this->turn][i] & destinationBB;
                if(capturedPiece) {
                    bitboards[!this->turn][i] &= ~capturedPiece;

                    //update hash
                    positionHashes[halfMoveNumber] ^= zobristPieceTable[!this->turn][i][destinationInt];

                    // store the capture move specifications
                    moveL |= 1uLL << CAPTURE_MOVE_FLAG_SHIFT;
                    moveL |= i << CAPTURED_PIECE_TYPE_SHIFT;
                    moveL |= debruijnSerialization(capturedPiece) << CAPTURED_PIECE_INDEX_SHIFT;
                    break;
                }
            }
            break;
        default:
            MovePiece(originBB, destinationBB, this->turn);

            // switch to opponent bitboards and remove possible destination piece in case of capture
            // also save this piece in the previousMoves array as it needs to be restored in case of undoMove
            for(unsigned i = 0; i < 6; i++){
                capturedPiece = bitboards[!this->turn][i] & destinationBB;
                if(capturedPiece) {
                    halfMovesSinceIrrepr = 0; // reset for 3fold repetition
                    halfMoveNumber50 = 0; // reset for 50 move rule
                    bitboards[!this->turn][i] &= ~capturedPiece;

                    //update hash
                    positionHashes[halfMoveNumber] ^= zobristPieceTable[!this->turn][i][destinationInt];

                    // store the capture move specifications
                    moveL |= 1uLL << CAPTURE_MOVE_FLAG_SHIFT;
                    moveL |= i << CAPTURED_PIECE_TYPE_SHIFT;
                    moveL |= debruijnSerialization(capturedPiece) << CAPTURED_PIECE_INDEX_SHIFT;
                    break;
                }
            }
            break;
    }

    // reset castling rights in case the rook has moved or has been captured. (and update hash)
    if(!(bitboards[WHITE][ROOKS] & (1uLL << SQ_H1))){
        positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
        castlingRights &= ~WHITE_KINGSIDE_CASTLING_RIGHTS;
        positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
    }
    if(!(bitboards[WHITE][ROOKS] & (1uLL << SQ_A1))){
        positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
        castlingRights &= ~WHITE_QUEENSIDE_CASTLING_RIGHTS;
        positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
    }
    if(!(bitboards[BLACK][ROOKS] & (1uLL << SQ_H8))){
        positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
        castlingRights &= ~BLACK_KINGSIDE_CASTLING_RIGHTS;
        positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
    }
    if(!(bitboards[BLACK][ROOKS] & (1uLL << SQ_A8))){
        positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
        castlingRights &= ~BLACK_QUEENSIDE_CASTLING_RIGHTS;
        positionHashes[halfMoveNumber] ^= zobristCastlingRightsTable[castlingRights];
    }


    // store possible en passant destination information in the previousMoveList
    enPassantInt = debruijnSerialization(bitboards[!this->turn][EN_PASSANT_SQUARES]);
    moveL |= enPassantInt << EN_PASSANT_DESTINATION_SQUARE_SHIFT;

    // update hash if there was an en passant square
    if(bitboards[!this->turn][EN_PASSANT_SQUARES]) {
        positionHashes[halfMoveNumber] ^= zobristEnPassantFile[enPassantInt % 8];
    }

    // reset possible en passant destination
    bitboards[!this->turn][EN_PASSANT_SQUARES] = 0;

    // put into previous moves list
    this->previousMoves[this->halfMoveNumber - 1] = moveL;

    // change turn and update hash
    this->turn = !this->turn;
    positionHashes[halfMoveNumber] ^= zobristBlackToMove;

    generateHelpBitboards();
}

void Position::doMove(string move) {
    // do a move where the move is formatted as for example e2e4, with promotion type appended if necessary.
    unsigned moveUnsigned = strToMoveNotation(move);

    // in case of castling
    if(this->turn == WHITE){ //
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

    // check for en passant move (as in case of do_move from external source this is not known)
    unsigned destinationInt = (moveUnsigned & DESTINATION_SQUARE_MASK) >> DESTINATION_SQUARE_SHIFT;
    if(bitboards[!this->turn][EN_PASSANT_SQUARES] & (1ull << destinationInt )){
        moveUnsigned |= EN_PASSANT_FLAG << SPECIAL_MOVE_FLAG_SHIFT;
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
        positionHashes[halfMoveNumber] ^= zobristPieceTable[this->turn][ROOKS][7u + shift];

        bitboards[this->turn][ROOKS] |=  (1uLL << (5u + shift)); // add rook
        positionHashes[halfMoveNumber] ^= zobristPieceTable[this->turn][ROOKS][5u + shift];

        positionHashes[halfMoveNumber] ^= zobristPieceTable[this->turn][KING][debruijnSerialization(bitboards[this->turn][KING])]; // remove king from hash
        bitboards[this->turn][KING] = bitboards[this->turn][KING] << 2u; // move king
        positionHashes[halfMoveNumber] ^= zobristPieceTable[this->turn][KING][debruijnSerialization(bitboards[this->turn][KING] << 2u)]; // add king to hash

    }
    else{ // queenside
        bitboards[this->turn][ROOKS] &= ~(1uLL << (0u + shift)); // remove rook
        positionHashes[halfMoveNumber] ^= zobristPieceTable[this->turn][ROOKS][0u + shift];

        bitboards[this->turn][ROOKS] |=  (1uLL << (3u + shift)); // add rook
        positionHashes[halfMoveNumber] ^= zobristPieceTable[this->turn][ROOKS][3u + shift];

        positionHashes[halfMoveNumber] ^= zobristPieceTable[this->turn][KING][debruijnSerialization(bitboards[this->turn][KING])]; // remove king from hash
        bitboards[this->turn][KING] = bitboards[this->turn][KING] >> 2u; // move king
        positionHashes[halfMoveNumber] ^= zobristPieceTable[this->turn][KING][debruijnSerialization(bitboards[this->turn][KING]) >> 2u]; // add king to hash
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
    // hash does not have to be updated manually, as it is stored for previous positions. But, as the
    // movePiece function changes the current hash, the final hash update has to be done at the end of this function
    // ______
    unsigned originInt, destinationInt, enPassantInt;

    // get the move
    uint64_t move = this->previousMoves[this->halfMoveNumber - 1];
    // revert back to 0
    this->previousMoves[this->halfMoveNumber - 1] = 0;

    originInt = (move & ORIGIN_SQUARE_MASK) >> ORIGIN_SQUARE_SHIFT;
    destinationInt = (move & DESTINATION_SQUARE_MASK) >> DESTINATION_SQUARE_SHIFT;

    uint64_t originBB = 1uLL << originInt;
    uint64_t destinationBB = 1uLL << destinationInt;


    // used for the promotion special move case
    unsigned promotionType;
    unsigned promotionIndices[] = {KNIGHTS, BISHOPS, ROOKS, QUEENS};

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
            break;

        case CASTLING_FLAG:
            if(((ORIGIN_SQUARE_MASK & move) >> ORIGIN_SQUARE_SHIFT) & KINGSIDE_CASTLING){ // kingside castling
                undoCastlingMove(true); // true is kingside
            }
            else{ // queenside castling
                undoCastlingMove(false); // false is queenside
            }
            break;
        case PROMOTION_FLAG:
            // find out promoted piece type
            promotionType = promotionIndices[(move & PROMOTION_TYPE_MASK) >> PROMOTION_TYPE_SHIFT];
            // remove promoted piece
            bitboards[!this->turn][promotionType] &= ~destinationBB;
            // add pawn
            bitboards[!this->turn][PAWNS] |= originBB;
            // put the captured piece back in case of capture
            if(move & CAPTURE_MOVE_FLAG_MASK){
                unsigned pieceIndex = (move & CAPTURED_PIECE_INDEX_MASK) >> CAPTURED_PIECE_INDEX_SHIFT;
                unsigned pieceType = (move & CAPTURED_PIECE_TYPE_MASK) >> CAPTURED_PIECE_TYPE_SHIFT;
                bitboards[this->turn][pieceType] |= 1uLL << pieceIndex;
            }
            break;
        default:
            // move the piece from destination to origin (so a possible wrong order warning is expected)
            MovePiece(destinationBB, originBB, !this->turn);

            // put the captured piece back
            if(move & CAPTURE_MOVE_FLAG_MASK){
                unsigned pieceIndex = (move & CAPTURED_PIECE_INDEX_MASK) >> CAPTURED_PIECE_INDEX_SHIFT;
                unsigned pieceType = (move & CAPTURED_PIECE_TYPE_MASK) >> CAPTURED_PIECE_TYPE_SHIFT;
                bitboards[this->turn][pieceType] |= 1uLL << pieceIndex;
            }
            break;
    }


    // restore en passant destination square
    enPassantInt = (EN_PASSANT_DESTINATION_SQUARE_MASK & move) >> EN_PASSANT_DESTINATION_SQUARE_SHIFT;
    if(enPassantInt) {
        bitboards[this->turn][EN_PASSANT_SQUARES] = 1uLL << enPassantInt;
    }

    // reset en passant destination square of other side
    bitboards[!this->turn][EN_PASSANT_SQUARES] = 0;

    // restore castling rights
    castlingRights = (move & CASTLING_RIGHTS_BEFORE_MOVE_MASK) >> CASTLING_RIGHTS_BEFORE_MOVE_SHIFT;

    // restore halfmove number for 50 move rule;
    halfMoveNumber50 = (move & HALFMOVENUMBER_BEFORE_MOVE_MASK) >> HALFMOVENUMBER_BEFORE_MOVE_SHIFT;

    // restore halfmove halfmoves since last time irreversible move (for 3fold repetion checks)
    halfMovesSinceIrrepr = (move & HALFMOVENUMBER_SINCE_IRREVERSIBLE_MOVE_MASK) >> HALFMOVENUMBER_SINCE_IRREVERSIBLE_MOVE_SHIFT;

    halfMoveNumber--;

    this->turn = !this->turn;
    generateHelpBitboards();

    // reset hash of undone position
    positionHashes[halfMoveNumber + 1] = 0;
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




const int PIECEWEIGHTS[6] = {
        // pawn, knight, bishop, rook, queen, king
        100, 320, 330, 500, 900, 20000
};

int PAWNPST[64] = {
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 25, 25, 10,  5,  5,
        0,  0,  0, 22, 22,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        5, 10, 10,-20,-30, 10, 10,  5,
        0,  0,  0,  0,  0,  0,  0,  0
};

int KNIGHTPST[64] = {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-30,-30,-30,-30,-30,-30,-50,
};


int BISHOPPST[64] = {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
};

int ROOKPST[64] = {
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,  0,  0,  5,  5,  0,  0,  0

};

int QUEENSPST[64] = {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,  0,  5,  5,  5,  5,  0, -5,
        0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, 5,  -10,-10,-10,-20
};

int KINGPSTMIDGAME[64] = {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
        20, 20, -10,-20,-20,-20, 20, 20,
        20, 30, -10,-20,  0,-20, 30, 20
};


int KINGPSTENDGAME[64] = {
        -50,-40,-30,-20,-20,-30,-40,-50,
        -30,-20,-10,  0,  0,-10,-20,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-30,  0,  0,  0,  0,-30,-30,
        -50,-30,-30,-30,-30,-30,-30,-50
};


constexpr int *PSTsMID[6] = {PAWNPST, KNIGHTPST, BISHOPPST, ROOKPST, QUEENSPST, KINGPSTMIDGAME};
constexpr int *PSTsEND[6] = {PAWNPST, KNIGHTPST, BISHOPPST, ROOKPST, QUEENSPST, KINGPSTENDGAME};

constexpr int INVERT[64] = {
        56, 57, 58, 59, 60, 61, 62, 63,
        48, 49, 50, 51, 52, 53, 54, 55,
        40, 41, 42, 43, 44, 45, 46, 47,
        32, 33, 34, 35, 36, 37, 38, 39,
        24, 25, 26, 27, 28, 29, 30, 31,
        16, 17, 18, 19, 20, 21, 22, 23,
        8,  9,  10, 11, 12, 13, 14, 15,
        0,  1,  2,  3,  4,  5,  6,  7
};

int Position::Evaluate() {
    // score in centipawns (positive is good for white, negative good for black)
    int score = 0;

    //start with white
    for(int pieceIndex = 0; pieceIndex < 6; pieceIndex++){
        uint64_t piece = bitboards[WHITE][pieceIndex];
        // calculate score for all pieces of pieceIndex type
        while(piece){
            // get index of first piece
            int pieceInt = debruijnSerialization(piece);

            // remove piece from remaining pieces
            piece &= ~(1uLL << pieceInt);

            // calculate score
            score += PIECEWEIGHTS[pieceIndex];

            if(isEndGame){
                score += PSTsEND[pieceIndex][pieceInt];
            }
            else{
                score += PSTsMID[pieceIndex][pieceInt];
            }
        }
    }

    // for black
    for(int pieceIndex = 0; pieceIndex < 6; pieceIndex++){
        uint64_t piece = bitboards[BLACK][pieceIndex];
        // calculate score for all pieces of pieceIndex type
        while(piece){
            // get index of first piece
            int pieceInt = debruijnSerialization(piece);

            // remove piece from remaining pieces
            piece &= ~(1uLL << pieceInt);

            pieceInt = INVERT[pieceInt]; // invert for black

            // calculate score
            score -= PIECEWEIGHTS[pieceIndex];

            if(isEndGame){
                score -= PSTsEND[pieceIndex][pieceInt];
            }
            else{
                score -= PSTsMID[pieceIndex][pieceInt];
            }
        }

        // for negamax, the score should always be maximizing for whomevers turn it is
    }

    if(this->turn){
        return score;
    }
    else{
        return -score;
    }
}

bool Position::isDraw() {
    if(halfMoveNumber50 >= 100){
        return true;
    }

    // check three fold repetition rule
    if(halfMovesSinceIrrepr >= 8){
        int threefoldRepetitionCount = 0;
        for(int halfmove = 2; halfmove <= halfMovesSinceIrrepr; halfmove += 2){
            // check same colour positions (so 2 halfmoves per check).
            if(positionHashes[halfMoveNumber] == positionHashes[halfMoveNumber - halfmove]){
                threefoldRepetitionCount++;
            }
        }
        if(threefoldRepetitionCount >= 2){
            return true;
        }
    }
    return false;
}


