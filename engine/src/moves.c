#include <stdlib.h>
#include <stdbool.h>
#include "moves.h"
#include <assert.h>
#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <string.h>

/*
    TODO: 

    - Kings shouldn't be able to castle out of check or if rook has previously moved
    - Pawns need to be able to promote
    - Pawns need to be able to en passante

*/


MoveList calc_pawn_moves(uint64_t position, const BoardState* const boardState);
MoveList calc_king_moves(uint64_t position, const BoardState* const boardState);
MoveList calc_knight_moves(uint64_t position, const BoardState* const boardState);
MoveList calc_bishop_moves(uint64_t position, const BoardState* const boardState);
MoveList calc_rook_moves(uint64_t position, const BoardState* const boardState);
MoveList calc_queen_moves(uint64_t position, const BoardState* const boardState);


MoveList getAllValidMoves(const BoardState* const boardState) {

    //initial array has set capacity
    MoveList validMoves;
    validMoves.count = 0;

    const uint64_t friendly_positions = (boardState->turn == WHITE)
        ? boardState->white_positions
        : boardState->black_positions;

    const uint64_t piece_count = __builtin_popcountll(friendly_positions);

    for (size_t i = 0; i < piece_count; ++i) {
        uint64_t piece_mask = extract_nth_set_bit(friendly_positions, i);
        MoveList piece_moves = getValidMoves(piece_mask, boardState);

        const size_t new_count = validMoves.count + piece_moves.count;
        assert(new_count < 256);

        for (size_t move = validMoves.count; move < new_count; move++) {
            validMoves.moves[move] = piece_moves.moves[move - validMoves.count];
        }

        validMoves.count += piece_moves.count;
    }

    return validMoves;
}
//Assumes move does not put own king in check
MoveList getPsuedoLegalMoves(uint64_t piece_mask, const BoardState* const boardState) {

    MoveList moves;
    moves.count = -1;
    
    //get all presumably valid moves
    if ((boardState->white[PAWN] & piece_mask) || (boardState->black[PAWN] & piece_mask)) {
        moves = calc_pawn_moves(piece_mask, boardState);
    } else if ((boardState->black[KNIGHT] & piece_mask) || (boardState->white[KNIGHT] & piece_mask)) {
        moves = calc_knight_moves(piece_mask, boardState);
    } else if ((boardState->black[KING] & piece_mask) || (boardState->white[KING] & piece_mask)) {
        moves = calc_king_moves(piece_mask, boardState);
    } else if ((boardState->black[BISHOP] & piece_mask) || (boardState->white[BISHOP] & piece_mask)) {
        moves = calc_bishop_moves(piece_mask, boardState);
    } else if ((boardState->black[ROOK] & piece_mask) || (boardState->white[ROOK] & piece_mask)) {
        moves = calc_rook_moves(piece_mask, boardState);
    } else if ((boardState->black[QUEEN] & piece_mask) || (boardState->white[QUEEN] & piece_mask)) {
        moves = calc_queen_moves(piece_mask, boardState);
    }    

    return moves;
}

bool isInCheck(TURN side, const BoardState* const boardState) {

    const uint64_t enemy_positions = (side == WHITE) ? boardState->black_positions : boardState->white_positions; 
    const uint64_t friendly_king = (side == WHITE) ? boardState->white[KING] : boardState->black[KING];
    const size_t enemy_piece_total = __builtin_popcountll(enemy_positions);

    for (size_t enemy_piece = 0; enemy_piece < enemy_piece_total; enemy_piece++) {
        const uint64_t enemy_piece_position = extract_nth_set_bit(enemy_positions, enemy_piece);
        const MoveList psuedoLegalResponses = getPsuedoLegalMoves(enemy_piece_position, boardState);

        for (size_t response = 0; response < psuedoLegalResponses.count; response++) {

            //if enemy can "capture" king, check
            if ((friendly_king & psuedoLegalResponses.moves[response].to) != 0) {
                return true;
            }
        }
    }
    return false;
}

//Assumption: If a move allows a response that could capture the King, it is illegal: 
MoveList getValidMoves(uint64_t piece_mask, const BoardState* const boardState) {
    MoveList pseudoLegalMoves = getPsuedoLegalMoves(piece_mask, boardState);
    MoveList legalMoves;
    legalMoves.count = 0;

    BoardState applied_move;

    for (size_t possible_move = 0; possible_move < pseudoLegalMoves.count; possible_move++) {
        applied_move = *boardState;
        applyMove(pseudoLegalMoves.moves[possible_move], &applied_move);
        bool move_is_legal = !isInCheck(boardState->turn, &applied_move);
        if (move_is_legal) {
            legalMoves.moves[legalMoves.count] = pseudoLegalMoves.moves[possible_move];
            legalMoves.count++;
        }
    }

    return legalMoves;
}

void removeCastleFlags(BoardState* boardState) {

    if (boardState->turn == WHITE) {
        boardState->can_castle &= ~WHITE_KINGSIDE;
        boardState->can_castle &= ~WHITE_QUEENSIDE;
    } else {
        boardState->can_castle &= ~BLACK_KINGSIDE;
        boardState->can_castle &= ~BLACK_QUEENSIDE; 
    }
}

void resetEnPassantSquares(BoardState* boardState) {
    boardState->valid_enpassant = 0;
}


/*
    TODO: properly handle flags (castle, promotion, en-passante)
    set valid en-passant bitmap
*/

void applyMove(Move move, BoardState* boardState) {

    uint64_t from_mask = move.from;
    uint64_t to_mask = move.to;

    //handle castle
    if (move.flags & FLAG_CASTLE_QUEENSIDE) {
        if (boardState->turn == WHITE) {
            boardState->white[KING] = 1ULL << 5;
            boardState->white[ROOK] &= ~(1ULL << 7);
            boardState->white[ROOK] |= 1ULL << 4;
        } else {
            boardState->black[KING] = 1ULL << 61;
            boardState->black[ROOK] &= ~(1ULL << 63);
            boardState->black[ROOK] |= 1ULL << 60;
        }

        removeCastleFlags(boardState);
    } else if (move.flags & FLAG_CASTLE_KINGSIDE) {
        if (boardState->turn == WHITE) {
            boardState->white[KING] = (1ULL << 1);
            boardState->white[ROOK] &= ~(1ULL << 0);
            boardState->white[ROOK] |= 1ULL << 2;
        } else {
            boardState->black[KING] = 1ULL << 57;
            boardState->black[ROOK] &= ~(1ULL << 56); 
            boardState->black[ROOK] |= (1ULL << 58);
        }
        removeCastleFlags(boardState);
    } //general case, turn = WHITE
    else if (boardState->turn == WHITE) {

        //move friendly piece
        if ((boardState->white[KING] & from_mask) != 0) {
            removeCastleFlags(boardState);
            boardState->white[KING] |= to_mask;
        }
        if ((boardState->white[QUEEN] & from_mask) != 0) boardState->white[QUEEN] |= to_mask;
        if ((boardState->white[ROOK] & from_mask) != 0) boardState->white[ROOK] |= to_mask;
        if ((boardState->white[BISHOP] & from_mask) != 0) boardState->white[BISHOP] |= to_mask;
        if ((boardState->white[KNIGHT] & from_mask) != 0) boardState->white[KNIGHT] |= to_mask;
        if ((boardState->white[PAWN] & from_mask) != 0) boardState->white[PAWN] |= to_mask;

        boardState->white[KING] &= ~from_mask;
        boardState->white[QUEEN] &= ~from_mask;
        boardState->white[ROOK] &= ~from_mask;
        boardState->white[BISHOP] &= ~from_mask;
        boardState->white[KNIGHT] &= ~from_mask;
        boardState->white[PAWN] &= ~from_mask;


        boardState->black[KING] &= ~to_mask;
        boardState->black[QUEEN] &= ~to_mask;
        boardState->black[ROOK] &= ~to_mask;
        boardState->black[BISHOP] &= ~to_mask;
        boardState->black[KNIGHT] &= ~to_mask;
        boardState->black[PAWN] &= ~to_mask;
    } else {

        if ((boardState->black[KING] & from_mask) != 0) {
            removeCastleFlags(boardState);
            boardState->black[KING] |= to_mask;
        }
        if ((boardState->black[QUEEN] & from_mask) != 0) boardState->black[QUEEN] |= to_mask;
        if ((boardState->black[ROOK] & from_mask) != 0) boardState->black[ROOK] |= to_mask;
        if ((boardState->black[BISHOP] & from_mask) != 0) boardState->black[BISHOP] |= to_mask;
        if ((boardState->black[KNIGHT] & from_mask) != 0) boardState->black[KNIGHT] |= to_mask;
        if ((boardState->black[PAWN] & from_mask) != 0) boardState->black[PAWN] |= to_mask;

        boardState->black[KING] &= ~from_mask;
        boardState->black[QUEEN] &= ~from_mask;
        boardState->black[ROOK] &= ~from_mask;
        boardState->black[BISHOP] &= ~from_mask;
        boardState->black[KNIGHT] &= ~from_mask;
        boardState->black[PAWN] &= ~from_mask;


        boardState->white[KING] &= ~to_mask;
        boardState->white[QUEEN] &= ~to_mask;
        boardState->white[ROOK] &= ~to_mask;
        boardState->white[BISHOP] &= ~to_mask;
        boardState->white[KNIGHT] &= ~to_mask;
        boardState->white[PAWN] &= ~to_mask;

    }


    if (boardState->turn == WHITE) boardState->turn = BLACK;
    else boardState->turn = WHITE;

    boardState->last_move = to_mask;
    updatePositionBitmap(boardState);

}

MoveList calc_king_moves(uint64_t position, const BoardState* const boardState) {

    uint64_t friendly_positions = (boardState->turn == WHITE) ? boardState->white_positions : boardState->black_positions;
    uint64_t enemy_positions = (boardState->turn == WHITE) ? boardState->black_positions : boardState->white_positions;
    uint64_t all_positions = friendly_positions | enemy_positions;
    uint64_t friendly_queenside_castle_mask = (boardState->turn == WHITE) ? QUEENSIDE_WHITE_CASTLE_MASK : QUEENSIDE_BLACK_CASTLE_MASK;
    uint64_t friendly_kingside_castle_mask = (boardState->turn == WHITE) ? KINGSIDE_WHITE_CASTLE_MASK : KINGSIDE_BLACK_CASTLE_MASK;
    uint64_t can_castle_queenside = ((boardState->turn == WHITE) ? (boardState->can_castle & WHITE_QUEENSIDE) :
        (boardState->can_castle & BLACK_QUEENSIDE)) && ((all_positions & friendly_queenside_castle_mask) == 0);
    uint64_t can_castle_kingside = ((boardState->turn == WHITE) ? (boardState->can_castle & WHITE_KINGSIDE) :
        (boardState->can_castle & BLACK_KINGSIDE)) && ((all_positions & friendly_kingside_castle_mask) == 0);
    
    uint64_t candidate_positions = 0;
    const char file = bitmapToPosition(position).file; 
    candidate_positions |= position >> 8;
    candidate_positions |= position << 8;
    candidate_positions |= position >> 1;
    candidate_positions |= position << 1;
    candidate_positions |= position >> 9;
    candidate_positions |= position << 9;
    candidate_positions |= position >> 7;
    candidate_positions |= position << 7;

    //remove friendly positions
    candidate_positions &= ~friendly_positions;

    //prevent moving from a-h file and vice-versa
    if (file == 'a') {
        candidate_positions &= ~HFILE;
    } else if (file == 'h') {
        candidate_positions &= ~AFILE;
    }

    const uint64_t move_bitmap = candidate_positions;

    MoveList moves;
    moves.count =  __builtin_popcountll(candidate_positions);

    for (size_t i = 0; i < moves.count; i++) {
        uint64_t new_position = extract_nth_set_bit(move_bitmap, i);
        Move move; 
        move.from = position;
        move.to = new_position;
        move.flags = (new_position & enemy_positions) ? FLAG_CAPTURE : FLAG_NONE;
        move.promotion = NO_PROMOTION;

        moves.moves[i] = move;
    }

    if (can_castle_kingside) {
        Move kingside_castle;
        kingside_castle.from = position;
        kingside_castle.to = (boardState->turn == WHITE) ? (1ULL << 1) : (1ULL << 57);
        kingside_castle.flags = FLAG_NONE | FLAG_CASTLE_KINGSIDE;
        kingside_castle.promotion = NO_PROMOTION;

        moves.moves[moves.count] = kingside_castle;
        moves.count++;
    }

    if (can_castle_queenside) {
        Move queenside_castle;
        queenside_castle.from = position;
        queenside_castle.to = (boardState->turn == WHITE) ? (1ULL << 5) : (1ULL << 61);
        queenside_castle.flags = FLAG_NONE | FLAG_CASTLE_QUEENSIDE;
        queenside_castle.promotion = NO_PROMOTION;

        moves.moves[moves.count] = queenside_castle;
        moves.count++;
    }

    return moves;
}

MoveList calc_bishop_moves(uint64_t position, const BoardState* const boardState) {
    uint64_t candidate_positions = 0;
    int square = __builtin_ctzll(position); 
    uint64_t friendly_positions = (boardState->turn == WHITE) ? boardState->white_positions : boardState->black_positions;
    uint64_t enemy_positions = (boardState->turn == WHITE) ? boardState->black_positions : boardState->white_positions;

    // direction vectors: NE, NW, SE, SW
    const int directions[4] = { 9, 7, -7, -9 };

    for (int d = 0; d < 4; ++d) {
        int step = directions[d];
        int s = square;

        while (1) {
            s += step;

            if (s < 0 || s >= 64) break;

            // prevent file wraparound
            int file_prev = (s - step) % 8;
            int file_now = s % 8;
            if (abs(file_now - file_prev) != 1) break;

            uint64_t mask = 1ULL << s;
            if (mask & friendly_positions) break;
            candidate_positions |= mask;
            if (mask & enemy_positions) break;
        }
    }

    const uint64_t move_bitmap = candidate_positions;

    MoveList moves;
    moves.count =  __builtin_popcountll(candidate_positions);

    for (size_t i = 0; i < moves.count; i++) {
        uint64_t new_position = extract_nth_set_bit(move_bitmap, i);
        Move move; 
        move.from = position;
        move.to = new_position;
        move.flags = (new_position & enemy_positions) ? FLAG_CAPTURE : FLAG_NONE;
        move.promotion = NO_PROMOTION;
        moves.moves[i] = move;
    }


    return moves;
}


MoveList calc_rook_moves(uint64_t position, const BoardState* const boardState) {
    uint64_t candidate_positions = 0;
    int square = __builtin_ctzll(position);
    uint64_t friendly_positions = (boardState->turn == WHITE) ? boardState->white_positions : boardState->black_positions;
    uint64_t enemy_positions = (boardState->turn == WHITE) ? boardState->black_positions : boardState->white_positions;

    // up (north)
    for (int s = square + 8; s < 64; s += 8) {
        uint64_t mask = 1ULL << s;
        if (mask & friendly_positions) break;
        candidate_positions |= mask;
        if (mask & enemy_positions) break;
    }

    // down (south)
    for (int s = square - 8; s >= 0; s -= 8) {
        uint64_t mask = 1ULL << s;
        if (mask & friendly_positions) break;
        candidate_positions |= mask;
        if (mask & enemy_positions) break;
    }

    // right (east)
    for (int s = square + 1; s % 8 != 0; ++s) {
        uint64_t mask = 1ULL << s;
        if (mask & friendly_positions) break;
        candidate_positions |= mask;
        if (mask & enemy_positions) break;
    }

    // left (west)
    for (int s = square - 1; s % 8 != 7 && s >= 0; --s) {
        uint64_t mask = 1ULL << s;
        if (mask & friendly_positions) break;
        candidate_positions |= mask;
        if (mask & enemy_positions) break;
    }

    const uint64_t move_bitmap = candidate_positions;

    MoveList moves;
    moves.count =  __builtin_popcountll(candidate_positions);

    for (size_t i = 0; i < moves.count; i++) {
        uint64_t new_position = extract_nth_set_bit(move_bitmap, i);
        Move move; 
        move.from = position;
        move.to = new_position;
        move.flags = (new_position & enemy_positions) ? FLAG_CAPTURE : FLAG_NONE;
        move.promotion = NO_PROMOTION;
        moves.moves[i] = move;
    }


    return moves;

}


MoveList calc_queen_moves(uint64_t position, const BoardState* const boardState) {
    
    MoveList bish_moves = calc_bishop_moves(position, boardState);
    MoveList rook_moves = calc_rook_moves(position, boardState);

    MoveList queen_moves;
    queen_moves.count = rook_moves.count + bish_moves.count;


    for (size_t i = 0; i < rook_moves.count; i++) {
        queen_moves.moves[i] = rook_moves.moves[i];
    }

    for (size_t i = rook_moves.count; i < queen_moves.count; i++) {
        queen_moves.moves[i] = bish_moves.moves[i - rook_moves.count];
    } 

    return queen_moves;
}

MoveList calc_knight_moves(uint64_t position, const BoardState* const boardState) {

    uint64_t candidate_positions = 0;
    char file = bitmapToPosition(position).file;
    uint64_t enemy_positions = (boardState->turn == WHITE) ? boardState->black_positions : boardState->white_positions;
    uint64_t friendly_positions = (boardState->turn == WHITE) ? boardState->white_positions : boardState->black_positions;
    
    if (file > 'a') {
        candidate_positions |= position >> 15;
        candidate_positions |= position << 17;
    }

    if (file > 'b') {
        candidate_positions |= position << 10;
        candidate_positions |= position >> 6;
    }

    if (file < 'g') {
        candidate_positions |= position << 6;
        candidate_positions |= position >> 10;

    }

    if (file < 'h') {
        candidate_positions |= position << 15;
        candidate_positions |= position >> 17;
    }

    candidate_positions &= ~friendly_positions;

    const uint64_t move_bitmap = candidate_positions;

    MoveList moves;
    moves.count =  __builtin_popcountll(candidate_positions);

    for (size_t i = 0; i < moves.count; i++) {
        uint64_t new_position = extract_nth_set_bit(move_bitmap, i);
        Move move; 
        move.from = position;
        move.to = new_position;
        move.flags = (new_position & enemy_positions) ? FLAG_CAPTURE : FLAG_NONE;
        move.promotion = NO_PROMOTION;
        moves.moves[i] = move;
    }


    return moves;

}


//TODO: promotion and en-passant
MoveList calc_pawn_moves(uint64_t position, const BoardState* const boardState) {

    uint64_t friendly_positions = (boardState->turn == WHITE) ? boardState->white_positions : boardState->black_positions;
    uint64_t enemy_positions = (boardState->turn == WHITE) ? boardState->black_positions : boardState->white_positions;
    const uint64_t occupied_positions = friendly_positions | enemy_positions;
    const char file = bitmapToPosition(position).file;
    
    
    bool is_on_starting_row = ((boardState->turn == WHITE)
        ? (position & 0x000000000000FF00)
        : (position & 0x00FF000000000000)) != 0;

    //mark space in front of pawn as a valid move
    uint64_t candidate_positions = (boardState->turn == WHITE) ? position << 8 : position >> 8;
    //remove any occupied spaces
    candidate_positions &= ~occupied_positions;
    //if on starting space, 2 spaces in front is also valid if 1 space in front is valid
    if (is_on_starting_row) {
        //shift spaces 'up' and add to candidates
        candidate_positions |= (boardState->turn == WHITE) ? candidate_positions << 8 : candidate_positions >> 8;
        //remove any occupied spaces
        candidate_positions &= ~occupied_positions;
    }

    //if an enemy is on diagonal, consider move valid
    candidate_positions |= (boardState->turn == WHITE) ? 
        (((position << 7)) | ((position << 9))) & enemy_positions : 
        (((position >> 7)) | ((position >> 9))) & enemy_positions;

    //remove any moves from a-h file and vice-versa
    if (file == 'a') {
        candidate_positions &= ~HFILE;
    } else if (file == 'h') {
        candidate_positions &= ~AFILE;
    }

    const uint64_t move_bitmap = candidate_positions;

    MoveList moves;
    moves.count =  __builtin_popcountll(candidate_positions);

    for (size_t i = 0; i < moves.count; i++) {
        uint64_t new_position = extract_nth_set_bit(move_bitmap, i);
        Move move; 
        move.from = position;
        move.to = new_position;
        move.flags = (new_position & enemy_positions) ? FLAG_CAPTURE : FLAG_NONE;
        move.promotion = NO_PROMOTION;
        moves.moves[i] = move;
    }


    return moves;
}