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


void calc_pawn_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer);
void calc_king_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer);
void calc_knight_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer);
void calc_bishop_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer);
void calc_rook_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer);
void calc_queen_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer);


uint64_t king_bitmap(uint64_t position, const BoardState* const boardState, SIDE side);
uint64_t pawn_bitmap(uint64_t position, const BoardState* const boardState, SIDE side);
uint64_t rook_bitmap(uint64_t position, const BoardState* const boardState, SIDE side);
uint64_t bishop_bitmap(uint64_t position, const BoardState* const boardState, SIDE side);
uint64_t knight_bitmap(uint64_t position, const BoardState* const boardState, SIDE side);

typedef void (*PseudoLegalMoveGenerator)(uint64_t, const BoardState*, MoveList*);
PseudoLegalMoveGenerator psuedoMoveGenerator[PIECE_TYPE_COUNT] = {
    [PAWN]  = calc_pawn_moves,
    [KNIGHT] = calc_knight_moves,
    [BISHOP] = calc_bishop_moves,
    [ROOK]  = calc_rook_moves,
    [QUEEN] = calc_queen_moves,
    [KING]  = calc_king_moves
};

void getAllValidMoves(const BoardState* const boardState, MoveList* buffer) {
    buffer->count = 0;

    const uint64_t friendly_positions = boardState->positions[boardState->turn];
    const uint64_t piece_count = __builtin_popcountll(friendly_positions);

    for (size_t i = 0; i < piece_count; ++i) {
        uint64_t piece_mask = extract_nth_set_bit(friendly_positions, i);
        getValidMoves(piece_mask, boardState, buffer);
    }
}


//TODO: 
//  Validate other piece could actually capture the king
//  (main edge case is if piece is pinned)
bool isInCheck(SIDE side, const BoardState* const boardState) {

    const uint64_t friendly_king = boardState->p_positions[side][KING];
    const uint64_t friendly_positions = boardState->positions[side];
    const uint64_t enemy_positions = boardState->positions[side ^ 1];

    //pretend king is a different piece
    //if it can capture it's counterpart on the other side, that one
    //could also (maybe) capture it
    //thus it is in check 

    if (friendly_king == 0) return true;

    uint64_t capture_bitmap = 0;
    capture_bitmap = king_bitmap(friendly_king, boardState, side);
    if ((capture_bitmap & boardState->p_positions[side ^ 1][KING]) != 0) return true;

    capture_bitmap = knight_bitmap(friendly_king, boardState, side);
    if ((capture_bitmap & boardState->p_positions[side ^ 1][KNIGHT]) != 0) return true;  

    capture_bitmap = pawn_bitmap(friendly_king, boardState, side);
    if ((capture_bitmap & boardState->p_positions[side ^ 1][PAWN]) != 0) return true;

    capture_bitmap = bishop_bitmap(friendly_king, boardState, side);
    if ((capture_bitmap & boardState->p_positions[side ^ 1][BISHOP]) != 0) return true;
    if ((capture_bitmap & boardState->p_positions[side ^ 1][QUEEN]) != 0) return true;

    capture_bitmap = rook_bitmap(friendly_king, boardState, side);
    if ((capture_bitmap & boardState->p_positions[side ^ 1][ROOK]) != 0) return true;
    if ((capture_bitmap & boardState->p_positions[side ^ 1][QUEEN]) != 0) return true;  

    return false;
}

//Assumption: If a move allows a response that could capture the King, it is illegal: 
void getValidMoves(uint64_t piece_mask, const BoardState* const boardState, MoveList* buffer) {

    PieceType pt = getPieceType(piece_mask, boardState);

    //pointer to psuedo-legal moves start/legal move end:
    size_t psuedoMovePointer = buffer->count;
    BoardState applied_move;

    psuedoMoveGenerator[pt](piece_mask, boardState, buffer);

    for (size_t possible_move = psuedoMovePointer; possible_move < buffer->count; possible_move++) {
        applied_move = *boardState;
        applyMove(buffer->moves[possible_move], &applied_move);
        bool move_is_legal = !isInCheck(boardState->turn, &applied_move);
        if (move_is_legal) {
            buffer->moves[psuedoMovePointer] = buffer->moves[possible_move];
            psuedoMovePointer++;
        }
    }

    buffer->count = psuedoMovePointer;

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
            boardState->p_positions[WHITE][KING] = 1ULL << 5;
            boardState->p_positions[WHITE][ROOK] &= ~(1ULL << 7);
            boardState->p_positions[WHITE][ROOK] |= 1ULL << 4;
        } else {
            boardState->p_positions[BLACK][KING] = 1ULL << 61;
            boardState->p_positions[BLACK][ROOK] &= ~(1ULL << 63);
            boardState->p_positions[BLACK][ROOK] |= 1ULL << 60;
        }

        removeCastleFlags(boardState);
    } else if (move.flags & FLAG_CASTLE_KINGSIDE) {
        if (boardState->turn == WHITE) {
            boardState->p_positions[WHITE][KING] = (1ULL << 1);
            boardState->p_positions[WHITE][ROOK] &= ~(1ULL << 0);
            boardState->p_positions[WHITE][ROOK] |= 1ULL << 2;
        } else {
            boardState->p_positions[BLACK][KING] = 1ULL << 57;
            boardState->p_positions[BLACK][ROOK] &= ~(1ULL << 56); 
            boardState->p_positions[BLACK][ROOK] |= (1ULL << 58);
        }
        removeCastleFlags(boardState);
    } //general case, turn = WHITE
    else if (boardState->turn == WHITE) {

        //move friendly piece
        if ((boardState->p_positions[WHITE][KING] & from_mask) != 0) {
            removeCastleFlags(boardState);
            boardState->p_positions[WHITE][KING] |= to_mask;
        }
        if ((boardState->p_positions[WHITE][QUEEN] & from_mask) != 0) boardState->p_positions[WHITE][QUEEN] |= to_mask;
        if ((boardState->p_positions[WHITE][ROOK] & from_mask) != 0) boardState->p_positions[WHITE][ROOK] |= to_mask;
        if ((boardState->p_positions[WHITE][BISHOP] & from_mask) != 0) boardState->p_positions[WHITE][BISHOP] |= to_mask;
        if ((boardState->p_positions[WHITE][KNIGHT] & from_mask) != 0) boardState->p_positions[WHITE][KNIGHT] |= to_mask;
        if ((boardState->p_positions[WHITE][PAWN] & from_mask) != 0) boardState->p_positions[WHITE][PAWN] |= to_mask;

        boardState->p_positions[WHITE][KING] &= ~from_mask;
        boardState->p_positions[WHITE][QUEEN] &= ~from_mask;
        boardState->p_positions[WHITE][ROOK] &= ~from_mask;
        boardState->p_positions[WHITE][BISHOP] &= ~from_mask;
        boardState->p_positions[WHITE][KNIGHT] &= ~from_mask;
        boardState->p_positions[WHITE][PAWN] &= ~from_mask;


        boardState->p_positions[BLACK][KING] &= ~to_mask;
        boardState->p_positions[BLACK][QUEEN] &= ~to_mask;
        boardState->p_positions[BLACK][ROOK] &= ~to_mask;
        boardState->p_positions[BLACK][BISHOP] &= ~to_mask;
        boardState->p_positions[BLACK][KNIGHT] &= ~to_mask;
        boardState->p_positions[BLACK][PAWN] &= ~to_mask;
    } else {

        if ((boardState->p_positions[BLACK][KING] & from_mask) != 0) {
            removeCastleFlags(boardState);
            boardState->p_positions[BLACK][KING] |= to_mask;
        }
        if ((boardState->p_positions[BLACK][QUEEN] & from_mask) != 0) boardState->p_positions[BLACK][QUEEN] |= to_mask;
        if ((boardState->p_positions[BLACK][ROOK] & from_mask) != 0) boardState->p_positions[BLACK][ROOK] |= to_mask;
        if ((boardState->p_positions[BLACK][BISHOP] & from_mask) != 0) boardState->p_positions[BLACK][BISHOP] |= to_mask;
        if ((boardState->p_positions[BLACK][KNIGHT] & from_mask) != 0) boardState->p_positions[BLACK][KNIGHT] |= to_mask;
        if ((boardState->p_positions[BLACK][PAWN] & from_mask) != 0) boardState->p_positions[BLACK][PAWN] |= to_mask;

        boardState->p_positions[BLACK][KING] &= ~from_mask;
        boardState->p_positions[BLACK][QUEEN] &= ~from_mask;
        boardState->p_positions[BLACK][ROOK] &= ~from_mask;
        boardState->p_positions[BLACK][BISHOP] &= ~from_mask;
        boardState->p_positions[BLACK][KNIGHT] &= ~from_mask;
        boardState->p_positions[BLACK][PAWN] &= ~from_mask;


        boardState->p_positions[WHITE][KING] &= ~to_mask;
        boardState->p_positions[WHITE][QUEEN] &= ~to_mask;
        boardState->p_positions[WHITE][ROOK] &= ~to_mask;
        boardState->p_positions[WHITE][BISHOP] &= ~to_mask;
        boardState->p_positions[WHITE][KNIGHT] &= ~to_mask;
        boardState->p_positions[WHITE][PAWN] &= ~to_mask;

    }


    boardState->turn ^= 1;

    boardState->last_move = to_mask;
    updatePositionBitmap(boardState);

}

uint64_t king_bitmap(uint64_t position, const BoardState* const boardState, SIDE side) {

    
    uint64_t friendly_positions = boardState->positions[boardState->turn];
    uint64_t enemy_positions = boardState->positions[boardState->turn ^ 1];

    
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

    return candidate_positions;

}

void calc_king_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer) {

    uint64_t all_positions = boardState->positions[WHITE] | boardState->positions[BLACK];
    uint64_t enemy_positions = boardState->positions[boardState->turn ^ 1];
    uint64_t friendly_queenside_castle_mask = (boardState->turn == WHITE) ? QUEENSIDE_WHITE_CASTLE_MASK : QUEENSIDE_BLACK_CASTLE_MASK;
    uint64_t friendly_kingside_castle_mask = (boardState->turn == WHITE) ? KINGSIDE_WHITE_CASTLE_MASK : KINGSIDE_BLACK_CASTLE_MASK;
    uint64_t can_castle_queenside = ((boardState->turn == WHITE) ? (boardState->can_castle & WHITE_QUEENSIDE) :
        (boardState->can_castle & BLACK_QUEENSIDE)) && ((all_positions & friendly_queenside_castle_mask) == 0);
    uint64_t can_castle_kingside = ((boardState->turn == WHITE) ? (boardState->can_castle & WHITE_KINGSIDE) :
        (boardState->can_castle & BLACK_KINGSIDE)) && ((all_positions & friendly_kingside_castle_mask) == 0);

    const uint64_t move_bitmap = king_bitmap(position, boardState, boardState->turn);
    const size_t new_move_c = ((move_bitmap != 0) ? __builtin_popcountll(move_bitmap) : 0);

    for (size_t i = 0; i < new_move_c; i++) {
        uint64_t new_position = extract_nth_set_bit(move_bitmap, i);
        Move move; 
        move.from = position;
        move.to = new_position;
        move.flags = (new_position & enemy_positions) ? FLAG_CAPTURE : FLAG_NONE;
        move.promotion = NO_PROMOTION;

        buffer->moves[buffer->count] = move;
        buffer->count++;
    }

    if (can_castle_kingside) {
        Move kingside_castle;
        kingside_castle.from = position;
        kingside_castle.to = (boardState->turn == WHITE) ? (1ULL << 1) : (1ULL << 57);
        kingside_castle.flags = FLAG_NONE | FLAG_CASTLE_KINGSIDE;
        kingside_castle.promotion = NO_PROMOTION;

        buffer->moves[buffer->count] = kingside_castle;
        buffer->count++;
    }

    if (can_castle_queenside) {
        Move queenside_castle;
        queenside_castle.from = position;
        queenside_castle.to = (boardState->turn == WHITE) ? (1ULL << 5) : (1ULL << 61);
        queenside_castle.flags = FLAG_NONE | FLAG_CASTLE_QUEENSIDE;
        queenside_castle.promotion = NO_PROMOTION;

        buffer->moves[buffer->count] = queenside_castle;
        buffer->count++;
    }
}

uint64_t bishop_bitmap(uint64_t position, const BoardState* const boardState, SIDE turn) {
    uint64_t candidate_positions = 0;
    int square = __builtin_ctzll(position); 
    uint64_t friendly_positions = boardState->positions[boardState->turn];
    uint64_t enemy_positions = boardState->positions[boardState->turn ^ 1];

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

    return candidate_positions;

}

void calc_bishop_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer) {
    uint64_t enemy_positions = boardState->positions[boardState->turn ^ 1];
    const uint64_t move_bitmap = bishop_bitmap(position, boardState, boardState->turn);
    const size_t new_move_c = ((move_bitmap != 0) ? __builtin_popcountll(move_bitmap) : 0);

    for (size_t i = 0; i < new_move_c; i++) {
        uint64_t new_position = extract_nth_set_bit(move_bitmap, i);
        Move move; 
        move.from = position;
        move.to = new_position;
        move.flags = (new_position & enemy_positions) ? FLAG_CAPTURE : FLAG_NONE;
        move.promotion = NO_PROMOTION;
        buffer->moves[buffer->count] = move;
        buffer->count++;
    }
}

uint64_t rook_bitmap(uint64_t position, const BoardState* const boardState, SIDE side) {
    uint64_t candidate_positions = 0;
    int square = __builtin_ctzll(position);
    uint64_t friendly_positions = boardState->positions[side];
    uint64_t enemy_positions = boardState->positions[side ^ 1];

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

    return candidate_positions;
}

void calc_rook_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer) {
    
    uint64_t enemy_positions = boardState->positions[boardState->turn ^ 1];
    const uint64_t move_bitmap = rook_bitmap(position, boardState, boardState->turn);
    const size_t new_move_c = ((move_bitmap != 0) ? __builtin_popcountll(move_bitmap) : 0);

    for (size_t i = 0; i < new_move_c; i++) {
        uint64_t new_position = extract_nth_set_bit(move_bitmap, i);
        Move move; 
        move.from = position;
        move.to = new_position;
        move.flags = (new_position & enemy_positions) ? FLAG_CAPTURE : FLAG_NONE;
        move.promotion = NO_PROMOTION;
        buffer->moves[buffer->count] = move;
        buffer->count++;
    }

}


void calc_queen_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer) {
    calc_bishop_moves(position, boardState, buffer);
    calc_rook_moves(position, boardState, buffer);
}

uint64_t knight_bitmap(uint64_t position, const BoardState* const boardState, SIDE turn) {

    uint64_t candidate_positions = 0;
    char file = bitmapToPosition(position).file;
    uint64_t enemy_positions = boardState->positions[turn ^ 1];
    uint64_t friendly_positions = boardState->positions[turn];
    
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
    return candidate_positions;

}

void calc_knight_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer) {

    uint64_t enemy_positions = boardState->positions[boardState->turn ^ 1];
    const uint64_t move_bitmap = knight_bitmap(position, boardState, boardState->turn);
    const size_t new_move_c = (move_bitmap != 0) ? __builtin_popcountll(move_bitmap) : 0;

    for (size_t i = 0; i < new_move_c; i++) {
        uint64_t new_position = extract_nth_set_bit(move_bitmap, i);
        Move move; 
        move.from = position;
        move.to = new_position;
        move.flags = (new_position & enemy_positions) ? FLAG_CAPTURE : FLAG_NONE;
        move.promotion = NO_PROMOTION;
        buffer->moves[buffer->count] = move;
        buffer->count++;
    }
}

uint64_t pawn_bitmap(uint64_t position, const BoardState* boardState, SIDE turn) {
    uint64_t friendly_positions = boardState->positions[turn];
    uint64_t enemy_positions = boardState->positions[(turn) ^ 1];

    const uint64_t occupied_positions = friendly_positions | enemy_positions;
    const char file = bitmapToPosition(position).file;
    
    
    bool is_on_starting_row = ((turn == WHITE)
        ? (position & 0x000000000000FF00)
        : (position & 0x00FF000000000000)) != 0;

    //mark space in front of pawn as a valid move
    uint64_t candidate_positions = (turn == WHITE) ? position << 8 : position >> 8;
    //remove any occupied spaces
    candidate_positions &= ~occupied_positions;
    //if on starting space, 2 spaces in front is also valid if 1 space in front is valid
    if (is_on_starting_row) {
        //shift spaces 'up' and add to candidates
        candidate_positions |= (turn == WHITE) ? candidate_positions << 8 : candidate_positions >> 8;
        //remove any occupied spaces
        candidate_positions &= ~occupied_positions;
    }

    //if an enemy is on diagonal, consider move valid
    candidate_positions |= (turn == WHITE) ? 
        (((position << 7)) | ((position << 9))) & enemy_positions : 
        (((position >> 7)) | ((position >> 9))) & enemy_positions;

    //remove any moves from a-h file and vice-versa
    if (file == 'a') {
        candidate_positions &= ~HFILE;
    } else if (file == 'h') {
        candidate_positions &= ~AFILE;
    }

    return candidate_positions;
}

//TODO: promotion and en-passant
void calc_pawn_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer) {

    uint64_t enemy_positions = boardState->positions[boardState->turn ^ 1];
    const uint64_t move_bitmap = pawn_bitmap(position, boardState, boardState->turn);
    const size_t new_move_c = ((move_bitmap != 0) ? __builtin_popcountll(move_bitmap) : 0);

    for (size_t i = 0; i < new_move_c; i++) {
        uint64_t new_position = extract_nth_set_bit(move_bitmap, i);
        Move move; 
        move.from = position;
        move.to = new_position;
        move.flags = (new_position & enemy_positions) ? FLAG_CAPTURE : FLAG_NONE;
        move.promotion = NO_PROMOTION;
        buffer->moves[buffer->count] = move;
        buffer->count++;
    }
}