#include <stdlib.h>
#include <stdbool.h>
#include "moves.h"
#include <assert.h>
#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <string.h>
#include "maps.h"
#include "trans_table.h"


void calc_pawn_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer);
void calc_king_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer);
void calc_knight_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer);
void calc_bishop_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer);
void calc_rook_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer);
void calc_queen_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer);


uint64_t king_bitmap(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions, uint64_t valid_enpassant, SIDE turn);
uint64_t queen_bitmap(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions, uint64_t valid_enpassant, SIDE turn);
uint64_t pawn_bitmap(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions, uint64_t valid_enpassant, SIDE turn);
uint64_t rook_bitmap(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions, uint64_t valid_enpassante, SIDE turn);
uint64_t bishop_bitmap(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions, uint64_t valid_enpassant, SIDE turn);
uint64_t knight_bitmap(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions, uint64_t valid_enpassant, SIDE turn);

typedef void (*PseudoLegalMoveGenerator)(uint64_t, const BoardState*, MoveList*);
PseudoLegalMoveGenerator psuedoMoveGenerator[PIECE_TYPE_COUNT] = {
    [PAWN]  = calc_pawn_moves,
    [KNIGHT] = calc_knight_moves,
    [BISHOP] = calc_bishop_moves,
    [ROOK]  = calc_rook_moves,
    [QUEEN] = calc_queen_moves,
    [KING]  = calc_king_moves
};

typedef uint64_t (*BitmapGenerator)(uint64_t, uint64_t, uint64_t, uint64_t, SIDE);
BitmapGenerator bitmapGenerator[PIECE_TYPE_COUNT] = {
    [PAWN] = pawn_bitmap, 
    [KNIGHT] = knight_bitmap,
    [BISHOP] = bishop_bitmap,
    [ROOK] = rook_bitmap,
    [QUEEN] = queen_bitmap, 
    [KING] = king_bitmap
};

void clearAttacks(uint64_t position, BoardState* boardState) {
    for (int k = 0; k < 64; k++) {
        boardState->attacked_by[1][k] &= ~position;
        boardState->attacked_by[0][k] &= ~position;
    }
}

void addAttacks(uint64_t mask, PieceType piece_type, BoardState* boardState, SIDE side) {

    //compute bitmap
    uint64_t attacks = bitmapGenerator[piece_type](
        mask,
        boardState->positions[side],
        boardState->positions[side^1],
        boardState->valid_enpassant,
        side
    );

    int king_sq = __builtin_ctzll(boardState->pieces[side^1][KING]);
    int attacking_sq = __builtin_ctzll(mask);
    //if attacks make check, compute check mask
    if (attacks & boardState->pieces[side^1][KING]) {
        //include place attacking from
        boardState->check_mask[side^1] |= (RAYS[attacking_sq][king_sq] | mask);
    }

    //check for and update pins
    if (piece_type == ROOK || piece_type == QUEEN) {
        uint64_t overlap = (RAYS[king_sq][attacking_sq] & ROOK_RAY_MAP[king_sq]) & boardState->positions[side^1];
        if (__builtin_ctzll(overlap) == 1)
            boardState->pinned[side^1] |= overlap;
    }
    if (piece_type == BISHOP || piece_type == QUEEN) {
        uint64_t overlap = (RAYS[king_sq][attacking_sq] & BISHOP_RAY_MAP[king_sq]) & boardState->positions[side^1];
        if (__builtin_ctzll(overlap) == 1)
            boardState->pinned[side^1] |= overlap;
    }

    while (attacks) {
        uint64_t a_mask = attacks & -attacks;
        attacks &= attacks - 1;
        int to_sq = __builtin_ctzll(a_mask);
        boardState->attacked_by[side^1][to_sq] |= mask;
    }
}


void updateRayAttackMaps(uint64_t position, BoardState* boardState) {
    int sq = __builtin_ctzll(position);
    uint64_t positions_to_update = boardState->attacked_by[WHITE][sq] | boardState->attacked_by[BLACK][sq];

    // mask out non-ray attackers
    positions_to_update &= ~(boardState->pieces[WHITE][KING] |
                             boardState->pieces[WHITE][PAWN] |
                             boardState->pieces[WHITE][KNIGHT] |
                             boardState->pieces[BLACK][KING] |
                             boardState->pieces[BLACK][PAWN] |
                             boardState->pieces[BLACK][KNIGHT]);

    
    if (!positions_to_update) return;

    for (int side = 0; side < 2; side++) {
        for (int j = 0; j < 3; j++) {
            PieceType pt = (PieceType[]){ROOK, BISHOP, QUEEN}[j];
            uint64_t piece_mask = positions_to_update & boardState->pieces[side][pt];

            while (piece_mask) {
                uint64_t mask = piece_mask & -piece_mask;
                piece_mask &= piece_mask - 1;
                int from_sq = __builtin_ctzll(mask);

                clearAttacks(mask, boardState);
                addAttacks(mask, pt, boardState, side);
            }
        }
    }
}



void updateAttackMaps(uint64_t position, PieceType piece_type, BoardState* boardState) {
    //clear existing attacks
    clearAttacks(position, boardState);
    //add new ones
    addAttacks(position, piece_type, boardState, boardState->turn);
    //update any discover attacks
    updateRayAttackMaps(position, boardState);
}

void initAttackMaps(BoardState* boardState) {
    for (int side = 0; side < 2; side++) {
        for (int pt = 0; pt < PIECE_TYPE_COUNT; pt++) {
            uint64_t piece_mask = boardState->pieces[side][pt];

            while (piece_mask) {
                uint64_t mask = piece_mask & -piece_mask;
                piece_mask &= piece_mask - 1;
                int from_sq = __builtin_ctzll(mask);

                clearAttacks(mask, boardState);
                addAttacks(mask, pt, boardState, side);
            }
        }
    }
}

bool isInCheck(SIDE side, const BoardState* const boardState) {
    int sq = __builtin_ctzll(boardState->pieces[side][KING]);
    return boardState->attacked_by[side][sq] != 0;
}


void getAllValidMoves(const BoardState* const boardState, MoveList* buffer) {
    buffer->count = 0;

    const uint64_t friendly_positions = boardState->positions[boardState->turn];
    const uint64_t piece_count = __builtin_popcountll(friendly_positions);

    for (size_t i = 0; i < piece_count; ++i) {
        uint64_t piece_mask = extract_nth_set_bit(friendly_positions, i);
        getValidMoves(piece_mask, boardState, buffer);
    }
}

//Assumption: There are 4 types of psuedo-legal moves that are illegal: 
/*
    1. Castling out of check
    2. King moves into attacked square
    3. Pinned piece moves
    4. When in check, the moving piece needs to either move onto the check mask or king needs to make out of the way (take or block the checking piece)
*/
void getValidMoves(uint64_t piece_mask, const BoardState* const boardState, MoveList* buffer) {
    PieceType pt = getPieceType(piece_mask, boardState);

    //pointer to psuedo-legal moves start/legal move end:
    size_t psuedoMovePointer = buffer->count;
    bool side = boardState->turn;
    bool start_in_check = isInCheck(boardState->turn, boardState);
    size_t checking_pieces_c = 
        __builtin_popcountll(boardState->attacked_by[side][__builtin_ctzll(boardState->pieces[side][KING])]);

    //cache friendly position map
    psuedoMoveGenerator[pt](piece_mask, boardState, buffer);
    BoardState temp;

    for (size_t possible_move = psuedoMovePointer; possible_move < buffer->count; possible_move++) {
        Move* move = &buffer->moves[possible_move];

        //king cannot move into attacked square
        if ((move->piece == KING) && (boardState->attacked_by[side][__builtin_ctzll(move->to)] != 0))
            continue;

        //pinned pieces cannot move
        if (move->from & boardState->pinned[side]) continue;      

        if (start_in_check) {
            //if there's more than 1 checking piece, have to move king
            if ((checking_pieces_c != 1) && (move->piece != KING)) continue;
            //cannot castle out of check
            if (move->flags & FLAG_CASTLE_KINGSIDE || move->flags & FLAG_CASTLE_QUEENSIDE) continue;
            //if not king, needs to be on check mask (take or block the checking piece)
            if ((move->piece != KING) && !(move->to & boardState->check_mask[side])) continue;
            //if the king moves, it must be either to a safe space (handled earlier) or must take attacker
        }

        buffer->moves[psuedoMovePointer] = buffer->moves[possible_move];
        psuedoMovePointer++;
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

//TODO: REMOVE CASTLE RIGHTS WHEN ROOK MOVES
void applyMove(Move move, BoardState* boardState) {

    uint64_t from_mask = move.from;
    int from_square = __builtin_ctzll(move.from); 
    uint64_t to_mask = move.to;
    int to_square = __builtin_ctzll(move.to);
    bool side = boardState->turn;
    bool other_side = boardState->turn ^ 1;
    uint64_t friendly_positions = boardState->positions[side];
    uint64_t enemy_positions = boardState->positions[other_side];

    //get rid of stale attack map values
    clearAttacks(move.from, boardState);
    boardState->check_mask[side] = 0;
    boardState->check_mask[other_side] = 0;
    boardState->pinned[side] = 0;
    boardState->pinned[other_side] = 0;

    //reset en-passant squares
    boardState->valid_enpassant = 0;

    //regardless of case, need to remove piece from prev loc
    boardState->pieces[side][move.piece]  &= ~from_mask;
    //"remove" piece from hash
    XORPiece(boardState, from_square, move.piece, side);

    //move piece to new spot (queen if promotion, otherwise the piece that moved)
    PieceType piece = __builtin_expect(move.flags & FLAG_PROMOTION, false) ? QUEEN : move.piece;
    boardState->pieces[side][piece] |= to_mask;
    //add piece to hash
    XORPiece(boardState, to_square, piece, side);
    if (piece == KING) removeCastleFlags(boardState);

    //if we capture, we need to remove enemy position from new position
    if (__builtin_expect(move.flags & FLAG_CAPTURE, false)) {
        // boardState->pieces[other_side][KING] &= ~to_mask;
        // boardState->pieces[other_side][QUEEN] &= ~to_mask;
        // boardState->pieces[other_side][ROOK] &= ~to_mask;
        // boardState->pieces[other_side][BISHOP] &= ~to_mask;
        // boardState->pieces[other_side][KNIGHT] &= ~to_mask;
        // boardState->pieces[other_side][PAWN] &= ~to_mask; 

        for (int pt = 0; pt < PIECE_TYPE_COUNT; pt++) {
            if (boardState->pieces[other_side][pt] & to_mask) {
                boardState->pieces[other_side][pt] &= ~to_mask;
                XORPiece(boardState, to_square, pt, other_side);
                break;
            }
        }
    } 

    //SPECIAL CASES: 
    //if en passant, take from en-passant square
    if (__builtin_expect(move.flags & FLAG_EN_PASSANT, false)) {
        uint64_t taken_mask = (boardState->turn == WHITE)
            ? (to_mask >> 8)
            : (to_mask << 8);
        boardState->pieces[other_side][PAWN] &= ~taken_mask;
        XORPiece(boardState, __builtin_ctzll(taken_mask), PAWN, other_side);
    }
    //if we moved a pawn up two spaces, need to update en-passant squares
    else if (__builtin_expect(move.flags & FLAG_PAWN_MOVE_TWO, false)) {
        boardState->valid_enpassant |= (boardState->turn == WHITE)
            ? (from_mask << 8)
            : (from_mask >> 8);
    }
    //if castle, we need to move the rook as well
    else if (__builtin_expect(move.flags & FLAG_CASTLE_KINGSIDE, false)) {
        if (boardState->turn == WHITE) {
            boardState->pieces[WHITE][ROOK] &= ~(1ULL << 0);
            boardState->pieces[WHITE][ROOK] |= 1ULL << 2;
        } else {
            boardState->pieces[BLACK][ROOK] &= ~(1ULL << 56); 
            boardState->pieces[BLACK][ROOK] |= (1ULL << 58);
        }
        removeCastleFlags(boardState);
    }
    else if (__builtin_expect(move.flags & FLAG_CASTLE_QUEENSIDE, false)) {
        if (boardState->turn == WHITE) {
            boardState->pieces[WHITE][ROOK] &= ~(1ULL << 7);
            boardState->pieces[WHITE][ROOK] |= 1ULL << 4;
        } else {
            boardState->pieces[BLACK][ROOK] &= ~(1ULL << 63);
            boardState->pieces[BLACK][ROOK] |= 1ULL << 60;
        }

        removeCastleFlags(boardState);
    }

    updatePositions(boardState);

    //update attack map
    updateAttackMaps(move.to, move.piece, boardState);
    updateAttackMaps(move.from, move.piece, boardState);
    
    //swap turns
    boardState->turn ^= 1;

    boardState->last_move = to_mask;
}

uint64_t king_bitmap(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions, uint64_t valid_enpassant, SIDE turn) {
    
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
    uint64_t friendly_positions = boardState->positions[boardState->turn];
    uint64_t enemy_positions = boardState->positions[boardState->turn ^ 1];
    uint64_t friendly_queenside_castle_mask = (boardState->turn == WHITE) ? QUEENSIDE_WHITE_CASTLE_MASK : QUEENSIDE_BLACK_CASTLE_MASK;
    uint64_t friendly_kingside_castle_mask = (boardState->turn == WHITE) ? KINGSIDE_WHITE_CASTLE_MASK : KINGSIDE_BLACK_CASTLE_MASK;
    uint64_t can_castle_queenside = ((boardState->turn == WHITE) ? (boardState->can_castle & WHITE_QUEENSIDE) :
        (boardState->can_castle & BLACK_QUEENSIDE)) && ((all_positions & friendly_queenside_castle_mask) == 0);
    uint64_t can_castle_kingside = ((boardState->turn == WHITE) ? (boardState->can_castle & WHITE_KINGSIDE) :
        (boardState->can_castle & BLACK_KINGSIDE)) && ((all_positions & friendly_kingside_castle_mask) == 0);

    const uint64_t move_bitmap = king_bitmap(position, friendly_positions, enemy_positions, boardState->valid_enpassant, boardState->turn);
    const size_t new_move_c = ((move_bitmap != 0) ? __builtin_popcountll(move_bitmap) : 0);

    for (size_t i = 0; i < new_move_c; i++) {
        uint64_t new_position = extract_nth_set_bit(move_bitmap, i);
        Move move; 
        move.from = position;
        move.to = new_position;
        move.flags = (new_position & enemy_positions) ? FLAG_CAPTURE : FLAG_NONE;
        move.piece = KING;

        buffer->moves[buffer->count] = move;
        buffer->count++;
    }

    if (can_castle_kingside) {
        Move kingside_castle;
        kingside_castle.from = position;
        kingside_castle.to = (boardState->turn == WHITE) ? (1ULL << 1) : (1ULL << 57);
        kingside_castle.flags = FLAG_NONE | FLAG_CASTLE_KINGSIDE;
        kingside_castle.piece = KING;
        buffer->moves[buffer->count] = kingside_castle;
        buffer->count++;
    }

    if (can_castle_queenside) {
        Move queenside_castle;
        queenside_castle.from = position;
        queenside_castle.to = (boardState->turn == WHITE) ? (1ULL << 5) : (1ULL << 61);
        queenside_castle.flags = FLAG_NONE | FLAG_CASTLE_QUEENSIDE;
        queenside_castle.piece = KING;
        buffer->moves[buffer->count] = queenside_castle;
        buffer->count++;
    }
}

uint64_t bishop_bitmap(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions, uint64_t valid_enpassant, SIDE turn) {
    uint64_t candidate_positions = 0;
    int square = __builtin_ctzll(position); 
    for (size_t dir = 0; dir < 4; dir++) {
        for (size_t i = 0; i < 8; i++) {
            uint64_t possible_move = BISHOP_MOVE_MAP[square][dir][i];
            if (possible_move == 0) break;
            if (possible_move & friendly_positions) break;
            candidate_positions |= BISHOP_MOVE_MAP[square][dir][i];
            if (possible_move & enemy_positions) break;     
        }
    }
    return candidate_positions;
}

void calc_bishop_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer) {
    uint64_t friendly_positions = boardState->positions[boardState->turn];
    uint64_t enemy_positions = boardState->positions[boardState->turn ^ 1];
    const uint64_t move_bitmap = bishop_bitmap(position, friendly_positions, enemy_positions, boardState->valid_enpassant, boardState->turn);
    const size_t new_move_c = ((move_bitmap != 0) ? __builtin_popcountll(move_bitmap) : 0);

    for (size_t i = 0; i < new_move_c; i++) {
        uint64_t new_position = extract_nth_set_bit(move_bitmap, i);
        Move move; 
        move.from = position;
        move.to = new_position;
        move.flags = (new_position & enemy_positions) ? FLAG_CAPTURE : FLAG_NONE;
        move.piece = BISHOP;
        buffer->moves[buffer->count] = move;
        buffer->count++;
    }
}

uint64_t rook_bitmap(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions, uint64_t valid_enpassant, SIDE turn) {
    uint64_t candidate_positions = 0;
    int square = __builtin_ctzll(position); 
    for (size_t dir = 0; dir < 4; dir++) {
        for (size_t i = 0; i < 8; i++) {
            uint64_t possible_move = ROOK_MOVE_MAP[square][dir][i];
            if (possible_move == 0) break;
            if (possible_move & friendly_positions) break;
            candidate_positions |= ROOK_MOVE_MAP[square][dir][i];
            if (possible_move & enemy_positions) break;     
        }
    }
    return candidate_positions;
}

void calc_rook_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer) {
    uint64_t friendly_positions = boardState->positions[boardState->turn];
    uint64_t enemy_positions = boardState->positions[boardState->turn ^ 1];
    const uint64_t move_bitmap = rook_bitmap(position, friendly_positions, enemy_positions, boardState->valid_enpassant, boardState->turn);
    const size_t new_move_c = ((move_bitmap != 0) ? __builtin_popcountll(move_bitmap) : 0);

    for (size_t i = 0; i < new_move_c; i++) {
        uint64_t new_position = extract_nth_set_bit(move_bitmap, i);
        Move move; 
        move.from = position;
        move.to = new_position;
        move.flags = (new_position & enemy_positions) ? FLAG_CAPTURE : FLAG_NONE;
        move.piece = ROOK;
        buffer->moves[buffer->count] = move;
        buffer->count++;
    }

}

uint64_t queen_bitmap(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions, uint64_t valid_enpassant, SIDE turn) {
    return rook_bitmap(position, friendly_positions, enemy_positions, valid_enpassant, turn) 
        | bishop_bitmap(position, friendly_positions, enemy_positions, valid_enpassant, turn);
}


void calc_queen_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer) {

    size_t ptr = buffer->count;

    calc_bishop_moves(position, boardState, buffer);
    calc_rook_moves(position, boardState, buffer);

    for ( ; ptr < buffer->count; ptr++){
        buffer->moves[ptr].piece = QUEEN;
    }
}

uint64_t knight_bitmap(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions, uint64_t valid_enpassant, SIDE turn) {

    uint64_t candidate_positions = 0;
    int square = __builtin_ctzll(position); 
    for (size_t i = 0; i < 8; i++) {
        uint64_t possible_move = KNIGHT_MOVE_MAP[square][i];
        if (possible_move & friendly_positions) continue;
        candidate_positions |= possible_move; 
    }
    return candidate_positions;

}

void calc_knight_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer) {

    uint64_t friendly_positions = boardState->positions[boardState->turn];
    uint64_t enemy_positions = boardState->positions[boardState->turn ^ 1];
    const uint64_t move_bitmap = knight_bitmap(position, friendly_positions, enemy_positions, boardState->valid_enpassant, boardState->turn);
    const size_t new_move_c = (move_bitmap != 0) ? __builtin_popcountll(move_bitmap) : 0;

    for (size_t i = 0; i < new_move_c; i++) {
        uint64_t new_position = extract_nth_set_bit(move_bitmap, i);
        Move move; 
        move.from = position;
        move.to = new_position;
        move.flags = (new_position & enemy_positions) ? FLAG_CAPTURE : FLAG_NONE;
        move.piece = KNIGHT;
        buffer->moves[buffer->count] = move;
        buffer->count++;
    }
}

uint64_t pawn_bitmap(uint64_t position,  uint64_t friendly_positions, uint64_t enemy_positions, uint64_t valid_enpassant, SIDE turn) {

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
        (((position << 7)) | ((position << 9))) & (enemy_positions | valid_enpassant) : 
        (((position >> 7)) | ((position >> 9))) & (enemy_positions | valid_enpassant);

    //remove any moves from a-h file and vice-versa
    if (file == 'a') {
        candidate_positions &= ~HFILE;
    } else if (file == 'h') {
        candidate_positions &= ~AFILE;
    }

    return candidate_positions;
}

void calc_pawn_moves(uint64_t position, const BoardState* const boardState, MoveList* buffer) {
    uint64_t friendly_positions = boardState->positions[boardState->turn];
    uint64_t enemy_positions = boardState->positions[boardState->turn ^ 1];
    const uint64_t move_bitmap = pawn_bitmap(position, friendly_positions, enemy_positions, boardState->valid_enpassant, boardState->turn);
    const size_t new_move_c = ((move_bitmap != 0) ? __builtin_popcountll(move_bitmap) : 0);
    static const uint64_t promotion_map = 0xFF000000000000FF;

    for (size_t i = 0; i < new_move_c; i++) {
        uint64_t new_position = extract_nth_set_bit(move_bitmap, i);
        Move move; 
        move.from = position;
        move.to = new_position;
        move.flags = (new_position & enemy_positions) ? FLAG_CAPTURE : FLAG_NONE;
        move.flags |= (((new_position << 16) == position) || ((new_position >> 16) == position)) ? FLAG_PAWN_MOVE_TWO : FLAG_NONE;
        move.flags |= (new_position & boardState->valid_enpassant) ? FLAG_EN_PASSANT : FLAG_NONE;
        move.flags |= (new_position & promotion_map) ? FLAG_PROMOTION : FLAG_NONE;
        move.piece = PAWN;
        buffer->moves[buffer->count] = move;
        buffer->count++;
    }
}