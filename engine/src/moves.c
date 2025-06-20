#include <stdlib.h>
#include <stdbool.h>
#include "moves.h"
#include <assert.h>
#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <string.h>


Moves calc_pawn_moves(uint64_t position, BoardState* boardState);
Moves calc_king_moves(uint64_t position, BoardState* boardState);
Moves calc_knight_moves(uint64_t position, BoardState* boardState);
Moves calc_bishop_moves(uint64_t position, BoardState* boardState);
Moves calc_rook_moves(uint64_t position, BoardState* boardState);
Moves calc_queen_moves(uint64_t position, BoardState* boardState);


Moves getValidMoves(uint64_t piece_mask, BoardState* boardState) {

    const uint64_t friendly_positions = (boardState->turn == WHITE) ? boardState->white_positions : boardState->black_positions;
    const uint64_t enemy_positions = (boardState->turn == WHITE) ? boardState->black_positions : boardState->white_positions;
    
    if ((boardState->white.pawns & piece_mask) || (boardState->black.pawns & piece_mask)) {
        return calc_pawn_moves(piece_mask, boardState);
    } else if ((boardState->black.knights & piece_mask) || (boardState->white.knights & piece_mask)) {
        return calc_knight_moves(piece_mask, boardState);
    } else if ((boardState->black.king & piece_mask) || (boardState->white.king & piece_mask)) {
        return calc_king_moves(piece_mask, boardState);
    } else if ((boardState->black.bishops & piece_mask) || (boardState->white.bishops & piece_mask)) {
        return calc_bishop_moves(piece_mask, boardState);
    } else if ((boardState->black.rooks & piece_mask) || (boardState->white.rooks & piece_mask)) {
        return calc_rook_moves(piece_mask, boardState);
    } else if ((boardState->black.queens & piece_mask) || (boardState->white.queens & piece_mask)) {
        return calc_queen_moves(piece_mask, boardState);
    }
    

    assert(false);
}


void move(uint64_t from, uint64_t to, BoardState* boardState) {

    uint64_t from_mask = from;
    uint64_t to_mask = to;
    
    if ((boardState->black.king & from_mask) != 0) boardState->black.king |= to_mask;
    boardState->black.king &= ~from_mask;
    if ((boardState->black.queens & from_mask) != 0) boardState->black.queens |= to_mask;
    boardState->black.queens &= ~from_mask;
    if ((boardState->black.rooks & from_mask) != 0) boardState->black.rooks |= to_mask;
    boardState->black.rooks &= ~from_mask;
    if ((boardState->black.bishops & from_mask) != 0) boardState->black.bishops |= to_mask;
    boardState->black.bishops &= ~from_mask;
    if ((boardState->black.knights & from_mask) != 0) boardState->black.knights |= to_mask;
    boardState->black.knights &= ~from_mask;
    if ((boardState->black.pawns & from_mask) != 0) boardState->black.pawns |= to_mask;
    boardState->black.pawns &= ~from_mask;

    if ((boardState->white.king & from_mask) != 0) boardState->white.king |= to_mask;
    boardState->white.king &= ~from_mask;
    if ((boardState->white.queens & from_mask) != 0) boardState->white.queens |= to_mask;
    boardState->white.queens &= ~from_mask;
    if ((boardState->white.rooks & from_mask) != 0) boardState->white.rooks |= to_mask;
    boardState->white.rooks &= ~from_mask;
    if ((boardState->white.bishops & from_mask) != 0) boardState->white.bishops |= to_mask;
    boardState->white.bishops &= ~from_mask;
    if ((boardState->white.knights & from_mask) != 0) boardState->white.knights |= to_mask;
    boardState->white.knights &= ~from_mask;
    if ((boardState->white.pawns & from_mask) != 0) boardState->white.pawns |= to_mask;
    boardState->white.pawns &= ~from_mask;

    if (boardState->turn == WHITE) boardState->turn = BLACK;
    else boardState->turn = WHITE;

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
    
        //swap turns
        newBoardStates[i].turn = (boardState->turn == WHITE) ? BLACK : WHITE;
        //move king to new position + remove new position from opponent's positions
        if (boardState->turn == WHITE) {
            newBoardStates[i].white.king &= ~new_position;
            newBoardStates[i].white.king |= new_position;

            newBoardStates[i].black.queens &= ~new_position;
            newBoardStates[i].black.king &= ~new_position;
            newBoardStates[i].black.rooks &= ~new_position;
            newBoardStates[i].black.bishops &= ~new_position;
            newBoardStates[i].black.knights &= ~new_position;
            newBoardStates[i].black.pawns &= ~new_position;
        } else {
            newBoardStates[i].black.king &= ~new_position;
            newBoardStates[i].black.king |= new_position;

            newBoardStates[i].white.queens &= ~new_position;
            newBoardStates[i].white.king &= ~new_position;
            newBoardStates[i].white.rooks &= ~new_position;
            newBoardStates[i].white.bishops &= ~new_position;
            newBoardStates[i].white.knights &= ~new_position;
            newBoardStates[i].white.pawns &= ~new_position;
        }
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
    
        //swap turns
        newBoardStates[i].turn = (boardState->turn == WHITE) ? BLACK : WHITE;
        //move rook to new position + remove new position from opponent's positions
        if (boardState->turn == WHITE) {
            newBoardStates[i].white.bishops &= ~new_position;
            newBoardStates[i].white.bishops |= new_position;

            newBoardStates[i].black.queens &= ~new_position;
            newBoardStates[i].black.king &= ~new_position;
            newBoardStates[i].black.rooks &= ~new_position;
            newBoardStates[i].black.bishops &= ~new_position;
            newBoardStates[i].black.knights &= ~new_position;
            newBoardStates[i].black.pawns &= ~new_position;
        } else {
            newBoardStates[i].black.bishops &= ~new_position;
            newBoardStates[i].black.bishops |= new_position;

            newBoardStates[i].white.queens &= ~new_position;
            newBoardStates[i].white.king &= ~new_position;
            newBoardStates[i].white.rooks &= ~new_position;
            newBoardStates[i].white.bishops &= ~new_position;
            newBoardStates[i].white.knights &= ~new_position;
            newBoardStates[i].white.pawns &= ~new_position;
        }
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
    
        //swap turns
        newBoardStates[i].turn = (boardState->turn == WHITE) ? BLACK : WHITE;
        //move rook to new position + remove new position from opponent's positions
        if (boardState->turn == WHITE) {
            newBoardStates[i].white.rooks &= ~new_position;
            newBoardStates[i].white.rooks |= new_position;

            newBoardStates[i].black.queens &= ~new_position;
            newBoardStates[i].black.king &= ~new_position;
            newBoardStates[i].black.rooks &= ~new_position;
            newBoardStates[i].black.bishops &= ~new_position;
            newBoardStates[i].black.knights &= ~new_position;
            newBoardStates[i].black.pawns &= ~new_position;
        } else {
            newBoardStates[i].black.rooks &= ~new_position;
            newBoardStates[i].black.rooks |= new_position;

            newBoardStates[i].white.queens &= ~new_position;
            newBoardStates[i].white.king &= ~new_position;
            newBoardStates[i].white.rooks &= ~new_position;
            newBoardStates[i].white.bishops &= ~new_position;
            newBoardStates[i].white.knights &= ~new_position;
            newBoardStates[i].white.pawns &= ~new_position;
        }
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

    free(bish_moves.boards);
    free(rook_moves.boards);

    //create board states
    for (size_t i = 0; i < queen_moves.count; i++) {
        uint64_t new_position = extract_nth_set_bit(queen_moves.move_bitmap, i);
        queen_moves.boards[i] = *boardState;
    
        //swap turns
        queen_moves.boards[i].turn = (boardState->turn == WHITE) ? BLACK : WHITE;
        //move rook to new position + remove new position from opponent's positions
        if (boardState->turn == WHITE) {
            queen_moves.boards[i].white.queens &= ~new_position;
            queen_moves.boards[i].white.queens |= new_position;

            queen_moves.boards[i].black.queens &= ~new_position;
            queen_moves.boards[i].black.king &= ~new_position;
            queen_moves.boards[i].black.rooks &= ~new_position;
            queen_moves.boards[i].black.bishops &= ~new_position;
            queen_moves.boards[i].black.knights &= ~new_position;
            queen_moves.boards[i].black.pawns &= ~new_position;
        } else {
            queen_moves.boards[i].black.queens &= ~new_position;
            queen_moves.boards[i].black.queens |= new_position;

            queen_moves.boards[i].white.queens &= ~new_position;
            queen_moves.boards[i].white.king &= ~new_position;
            queen_moves.boards[i].white.rooks &= ~new_position;
            queen_moves.boards[i].white.bishops &= ~new_position;
            queen_moves.boards[i].white.knights &= ~new_position;
            queen_moves.boards[i].white.pawns &= ~new_position;
        }
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
    
        //swap turns
        newBoardStates[i].turn = (boardState->turn == WHITE) ? BLACK : WHITE;
        //move knight to new position + remove new position from opponent's positions
        if (boardState->turn == WHITE) {
            newBoardStates[i].white.knights &= ~new_position;
            newBoardStates[i].white.knights |= new_position;

            newBoardStates[i].black.queens &= ~new_position;
            newBoardStates[i].black.king &= ~new_position;
            newBoardStates[i].black.rooks &= ~new_position;
            newBoardStates[i].black.bishops &= ~new_position;
            newBoardStates[i].black.knights &= ~new_position;
            newBoardStates[i].black.pawns &= ~new_position;
        } else {
            newBoardStates[i].black.knights &= ~new_position;
            newBoardStates[i].black.knights |= new_position;

            newBoardStates[i].white.queens &= ~new_position;
            newBoardStates[i].white.king &= ~new_position;
            newBoardStates[i].white.rooks &= ~new_position;
            newBoardStates[i].white.bishops &= ~new_position;
            newBoardStates[i].white.knights &= ~new_position;
            newBoardStates[i].white.pawns &= ~new_position;
        }
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
    
        //swap turns
        newBoardStates[i].turn = (boardState->turn == WHITE) ? BLACK : WHITE;
        //move knight to new position + remove new position from opponent's positions
        if (boardState->turn == WHITE) {
            newBoardStates[i].white.pawns &= ~new_position;
            newBoardStates[i].white.pawns |= new_position;

            newBoardStates[i].black.queens &= ~new_position;
            newBoardStates[i].black.king &= ~new_position;
            newBoardStates[i].black.rooks &= ~new_position;
            newBoardStates[i].black.bishops &= ~new_position;
            newBoardStates[i].black.knights &= ~new_position;
            newBoardStates[i].black.pawns &= ~new_position;
        } else {
            newBoardStates[i].black.pawns &= ~new_position;
            newBoardStates[i].black.pawns |= new_position;

            newBoardStates[i].white.queens &= ~new_position;
            newBoardStates[i].white.king &= ~new_position;
            newBoardStates[i].white.rooks &= ~new_position;
            newBoardStates[i].white.bishops &= ~new_position;
            newBoardStates[i].white.knights &= ~new_position;
            newBoardStates[i].white.pawns &= ~new_position;
        }
    }

    moves.boards = newBoardStates;

    return moves;
}