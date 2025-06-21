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

    - Kings need to be able to castle
        - needs to make sure no pieces are in the way
        - cannot castle out of check
        - cannot castle if either rook or king has previously moved
    - Pawns need to be able to promote
    - Pawns need to be able to en passante

*/


Moves calc_pawn_moves(uint64_t position, BoardState* boardState);
Moves calc_king_moves(uint64_t position, BoardState* boardState);
Moves calc_knight_moves(uint64_t position, BoardState* boardState);
Moves calc_bishop_moves(uint64_t position, BoardState* boardState);
Moves calc_rook_moves(uint64_t position, BoardState* boardState);
Moves calc_queen_moves(uint64_t position, BoardState* boardState);


#define INITIAL_BUFFER_SIZE 50
#define GROWTH_FACTOR 1.75

Moves getAllValidMoves(BoardState* boardState) {
    size_t capacity = INITIAL_BUFFER_SIZE;


    //initial array has set capacity
    Moves validMoves;
    validMoves.count = 0;
    validMoves.move_bitmap = 0;
    validMoves.boards = malloc(sizeof(BoardState) * capacity);
    assert(validMoves.boards);

    const uint64_t friendly_positions = (boardState->turn == WHITE)
        ? boardState->white_positions
        : boardState->black_positions;

    const uint64_t piece_count = __builtin_popcountll(friendly_positions);

    for (size_t i = 0; i < piece_count; ++i) {
        uint64_t piece_mask = extract_nth_set_bit(friendly_positions, i);
        Moves piece_moves = getValidMoves(piece_mask, boardState);

        if (validMoves.count + piece_moves.count > capacity) {
            while (validMoves.count + piece_moves.count > capacity) {
                capacity = (size_t)(capacity * GROWTH_FACTOR + 1);
            }
            validMoves.boards = realloc(validMoves.boards, sizeof(BoardState) * capacity);
            assert(validMoves.boards);
        }

        memcpy(&validMoves.boards[validMoves.count],
               piece_moves.boards,
               sizeof(BoardState) * piece_moves.count);

        validMoves.count += piece_moves.count;
        free(piece_moves.boards);
    }

    if (capacity > validMoves.count) {
        validMoves.boards = realloc(validMoves.boards, sizeof(BoardState) * validMoves.count);
    }

    return validMoves;
}
//Assumes move does not put own king in check
Moves getPsuedoLegalMoves(uint64_t piece_mask, BoardState* boardState) {

    Moves moves;
    
    //get all presumably valid moves
    if ((boardState->white.pawns & piece_mask) || (boardState->black.pawns & piece_mask)) {
        moves = calc_pawn_moves(piece_mask, boardState);
    } else if ((boardState->black.knights & piece_mask) || (boardState->white.knights & piece_mask)) {
        moves = calc_knight_moves(piece_mask, boardState);
    } else if ((boardState->black.king & piece_mask) || (boardState->white.king & piece_mask)) {
        moves = calc_king_moves(piece_mask, boardState);
    } else if ((boardState->black.bishops & piece_mask) || (boardState->white.bishops & piece_mask)) {
        moves = calc_bishop_moves(piece_mask, boardState);
    } else if ((boardState->black.rooks & piece_mask) || (boardState->white.rooks & piece_mask)) {
        moves = calc_rook_moves(piece_mask, boardState);
    } else if ((boardState->black.queens & piece_mask) || (boardState->white.queens & piece_mask)) {
        moves = calc_queen_moves(piece_mask, boardState);
    }    

    return moves;
}

bool isInCheck(TURN side, BoardState* boardState) {

    const uint64_t enemy_positions = (side == WHITE) ? boardState->black_positions : boardState->white_positions; 
    const size_t enemy_piece_total = __builtin_popcountll(enemy_positions);

    for (size_t enemy_piece = 0; enemy_piece < enemy_piece_total; enemy_piece++) {
        const uint64_t enemy_piece_position = extract_nth_set_bit(enemy_positions, enemy_piece);
        const Moves psuedoLegalResponses = getPsuedoLegalMoves(enemy_piece_position, boardState);

        for (size_t response = 0; response < psuedoLegalResponses.count; response++) {

            const uint64_t king = (side == WHITE) 
                ? psuedoLegalResponses.boards[response].white.king
                : psuedoLegalResponses.boards[response].black.king;

            if (king == 0) {
                free(psuedoLegalResponses.boards);
                return true;
            }
        }

        free(psuedoLegalResponses.boards);
    }

    return false;
}

//Assumption: If a move allows a response that could capture the King, it is illegal: 
Moves getValidMoves(uint64_t piece_mask, BoardState* boardState) {
    Moves pseudoLegalMoves = getPsuedoLegalMoves(piece_mask, boardState);
    Moves legalMoves;
    legalMoves.boards = malloc(sizeof(BoardState) * pseudoLegalMoves.count);
    legalMoves.count = 0;
    legalMoves.move_bitmap = 0;

    for (size_t possible_move = 0; possible_move < pseudoLegalMoves.count; possible_move++) {

        bool move_is_legal = !isInCheck(boardState->turn, &pseudoLegalMoves.boards[possible_move]);

        if (move_is_legal) {
            memcpy(&legalMoves.boards[legalMoves.count], &pseudoLegalMoves.boards[possible_move], sizeof(BoardState));
            legalMoves.count++;
            legalMoves.move_bitmap |= pseudoLegalMoves.boards[possible_move].last_move;
        }
    }

    free(pseudoLegalMoves.boards);
    return legalMoves;
}

//used for simple moves, move from one space to another
//more complex moves (castling etc) handled by individual piece functions
void move(uint64_t from, uint64_t to, BoardState* boardState) {

    uint64_t from_mask = from;
    uint64_t to_mask = to;
    
    if (boardState->turn == WHITE) {

        //move friendly piece
        if ((boardState->white.king & from_mask) != 0) boardState->white.king |= to_mask;
        if ((boardState->white.queens & from_mask) != 0) boardState->white.queens |= to_mask;
        if ((boardState->white.rooks & from_mask) != 0) boardState->white.rooks |= to_mask;
        if ((boardState->white.bishops & from_mask) != 0) boardState->white.bishops |= to_mask;
        if ((boardState->white.knights & from_mask) != 0) boardState->white.knights |= to_mask;
        if ((boardState->white.pawns & from_mask) != 0) boardState->white.pawns |= to_mask;

        boardState->white.king &= ~from_mask;
        boardState->white.queens &= ~from_mask;
        boardState->white.rooks &= ~from_mask;
        boardState->white.bishops &= ~from_mask;
        boardState->white.knights &= ~from_mask;
        boardState->white.pawns &= ~from_mask;


        boardState->black.king &= ~to_mask;
        boardState->black.queens &= ~to_mask;
        boardState->black.rooks &= ~to_mask;
        boardState->black.bishops &= ~to_mask;
        boardState->black.knights &= ~to_mask;
        boardState->black.pawns &= ~to_mask;
    } else {

        if ((boardState->black.king & from_mask) != 0) boardState->black.king |= to_mask;
        if ((boardState->black.queens & from_mask) != 0) boardState->black.queens |= to_mask;
        if ((boardState->black.rooks & from_mask) != 0) boardState->black.rooks |= to_mask;
        if ((boardState->black.bishops & from_mask) != 0) boardState->black.bishops |= to_mask;
        if ((boardState->black.knights & from_mask) != 0) boardState->black.knights |= to_mask;
        if ((boardState->black.pawns & from_mask) != 0) boardState->black.pawns |= to_mask;

        boardState->black.king &= ~from_mask;
        boardState->black.queens &= ~from_mask;
        boardState->black.rooks &= ~from_mask;
        boardState->black.bishops &= ~from_mask;
        boardState->black.knights &= ~from_mask;
        boardState->black.pawns &= ~from_mask;


        boardState->white.king &= ~to_mask;
        boardState->white.queens &= ~to_mask;
        boardState->white.rooks &= ~to_mask;
        boardState->white.bishops &= ~to_mask;
        boardState->white.knights &= ~to_mask;
        boardState->white.pawns &= ~to_mask;

    }
    if (boardState->turn == WHITE) boardState->turn = BLACK;
    else boardState->turn = WHITE;

    boardState->last_move = to;
    updatePositionBitmap(boardState);

}

//TODO: castle
Moves calc_king_moves(uint64_t position, BoardState* boardState) {

    uint64_t friendly_positions = (boardState->turn == WHITE) ? boardState->white_positions : boardState->black_positions;
    bool can_castle = (boardState->turn == WHITE) ? boardState->white_can_castle : boardState->black_can_castle;

    if (!can_castle) {

    }
    
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

    Moves moves;


    moves.move_bitmap = candidate_positions;
    moves.count =  __builtin_popcountll(candidate_positions);

    BoardState* newBoardStates = malloc(sizeof(BoardState) * moves.count);

    for (size_t i = 0; i < moves.count; i++) {
        uint64_t new_position = extract_nth_set_bit(moves.move_bitmap, i);
        newBoardStates[i] = *boardState;
        move(position, new_position, &newBoardStates[i]);
    }

    moves.boards = newBoardStates;

    return moves;
}

Moves calc_bishop_moves(uint64_t position, BoardState* boardState) {
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

    Moves moves;
    moves.move_bitmap = candidate_positions;
    moves.count = __builtin_popcountll(candidate_positions);
    BoardState* newBoardStates = malloc(sizeof(BoardState) * moves.count);

    //create board states
    for (size_t i = 0; i < moves.count; i++) {
        uint64_t new_position = extract_nth_set_bit(moves.move_bitmap, i);
        newBoardStates[i] = *boardState;
        move(position, new_position, &newBoardStates[i]);
    }

    moves.boards = newBoardStates;

    return moves;
}


Moves calc_rook_moves(uint64_t position, BoardState* boardState) {
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

    Moves moves;
    moves.move_bitmap = candidate_positions;
    moves.count = __builtin_popcountll(candidate_positions);
    BoardState* newBoardStates = malloc(sizeof(BoardState) * moves.count);

    //create board states
    for (size_t i = 0; i < moves.count; i++) {
        uint64_t new_position = extract_nth_set_bit(moves.move_bitmap, i);
        newBoardStates[i] = *boardState;
        move(position, new_position, &newBoardStates[i]);
    }

    moves.boards = newBoardStates;

    return moves;

}


Moves calc_queen_moves(uint64_t position, BoardState* boardState) {
    
    Moves bish_moves = calc_bishop_moves(position, boardState);
    Moves rook_moves = calc_rook_moves(position, boardState);


    Moves queen_moves;
    queen_moves.count = bish_moves.count + rook_moves.count;
    queen_moves.move_bitmap = bish_moves.move_bitmap | rook_moves.move_bitmap;
    queen_moves.boards = malloc(sizeof(BoardState) * queen_moves.count);

    memcpy(queen_moves.boards, bish_moves.boards, sizeof(BoardState) * bish_moves.count);
    memcpy(queen_moves.boards + bish_moves.count, rook_moves.boards, sizeof(BoardState) * rook_moves.count);

    free(bish_moves.boards);
    free(rook_moves.boards);

    //create board states
    for (size_t i = 0; i < queen_moves.count; i++) {
        uint64_t new_position = extract_nth_set_bit(queen_moves.move_bitmap, i);
        move(position, new_position, &queen_moves.boards[i]);
        queen_moves.boards[i].turn = (boardState->turn == WHITE) ? BLACK : WHITE;
    }


    return queen_moves;
}

Moves calc_knight_moves(uint64_t position, BoardState* boardState) {

    Moves moves;

    uint64_t candidate_positions = 0;
    char file = bitmapToPosition(position).file;
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

    //count all the possible moves
    int count = __builtin_popcountll(candidate_positions);
    moves.count = count;
    moves.move_bitmap = candidate_positions;

    //allocate array for each possible board state
    BoardState* newBoardStates = malloc(sizeof(BoardState) * count);

    //create board states
    for (size_t i = 0; i < count; i++) {
        uint64_t new_position = extract_nth_set_bit(moves.move_bitmap, i);
        newBoardStates[i] = *boardState;
        move(position, new_position, &newBoardStates[i]);
    }

    moves.boards = newBoardStates;

    return moves;

}


//TODO: promotion and en-passant
Moves calc_pawn_moves(uint64_t position, BoardState* boardState) {

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

    int count = __builtin_popcountll(candidate_positions);
    Moves moves;
    moves.count = count;
    moves.move_bitmap = candidate_positions;

    //allocate array for each possible board state
    BoardState* newBoardStates = malloc(sizeof(BoardState) * count);

    //create board states
    for (size_t i = 0; i < count; i++) {
        uint64_t new_position = extract_nth_set_bit(moves.move_bitmap, i);
        newBoardStates[i] = *boardState;
        move(position, new_position, &newBoardStates[i]);
    }

    moves.boards = newBoardStates;

    return moves;
}