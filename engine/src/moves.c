#include <stdlib.h>
#include <stdbool.h>
#include "moves.h"
#include <assert.h>
#include "board.h"
#include <stdio.h>
#include <stdlib.h>

uint64_t calc_pawn_moves(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions, TURN turn);
uint64_t calc_king_moves(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions);
uint64_t calc_knight_moves(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions);
uint64_t calc_bishop_moves(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions);
uint64_t calc_rook_moves(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions);
uint64_t calc_queen_moves(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions);


uint64_t getValidMoves(uint64_t piece_mask, struct BoardState* boardState) {

    printBinary(piece_mask);

    const uint64_t black_positions = 
        boardState->black.king | boardState->black.queens | boardState->black.rooks | boardState->black.knights | boardState->black.bishops | boardState->black.pawns;
    const uint64_t white_positions = 
        boardState->white.king | boardState->white.queens | boardState->white.rooks | boardState->white.knights | boardState->white.bishops | boardState->white.pawns;
    
    const uint64_t friendly_positions = (boardState->turn == WHITE) ? white_positions : black_positions;
    const uint64_t enemy_positions = (boardState->turn == WHITE) ? black_positions : white_positions;
    
    if ((boardState->white.pawns & piece_mask) || (boardState->black.pawns & piece_mask)) {
        return calc_pawn_moves(piece_mask, friendly_positions, enemy_positions, boardState->turn);
    } else if ((boardState->black.knights & piece_mask) || (boardState->white.knights & piece_mask)) {
        return calc_knight_moves(piece_mask, friendly_positions, enemy_positions);
    } else if ((boardState->black.king & piece_mask) || (boardState->white.king & piece_mask)) {
        return calc_king_moves(piece_mask, friendly_positions, enemy_positions);
    } else if ((boardState->black.bishops & piece_mask) || (boardState->white.bishops & piece_mask)) {
        return calc_bishop_moves(piece_mask, friendly_positions, enemy_positions);
    } else if ((boardState->black.rooks & piece_mask) || (boardState->white.rooks & piece_mask)) {
        return calc_rook_moves(piece_mask, friendly_positions, enemy_positions);
    } else if ((boardState->black.queens & piece_mask) || (boardState->white.queens & piece_mask)) {
        return calc_queen_moves(piece_mask, friendly_positions, enemy_positions);
    }
    //placeholder
    else return 0;
}


void move(uint64_t from, uint64_t to, struct BoardState* boardState) {

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

struct BoardState** calcValidMoves(uint64_t position, struct BoardState* boardState) {
    //set up function args
    const uint64_t black_positions = 
        boardState->black.king | boardState->black.queens | boardState->black.rooks | boardState->black.knights | boardState->black.bishops | boardState->black.pawns;
    const uint64_t white_positions = 
        boardState->white.king | boardState->white.queens | boardState->white.rooks | boardState->white.knights | boardState->white.bishops | boardState->white.pawns;
    
    const uint64_t friendly_positions = (boardState->turn == WHITE) ? white_positions : black_positions;
    const uint64_t enemy_positions = (boardState->turn == WHITE) ? black_positions : white_positions;
    
    //figure out the piece type and record candidate positions
    uint64_t candidate_positions = 0;
    const struct PiecePositions friendly_pieces = (boardState->turn == WHITE) ? boardState->white : boardState->black; 
    const struct PiecePositions enemy_pieces = (boardState->turn == WHITE) ? boardState->black : boardState->white;

    if ((position & friendly_pieces.pawns) != 0) {
        candidate_positions = calc_pawn_moves(position, friendly_positions, enemy_positions, boardState->turn);
    }//TODO add other piece logic

    //parse candidate positions and construct valid BoardState for each candidate
    for (size_t idx = 0; idx < 64; idx++) {

        if ((candidate_positions >> idx) & 1) continue; //if no candidate at current index, move on

        const uint64_t new_position = 1ULL << idx;
        struct PiecePositions new_friendly_pieces = friendly_pieces;
        struct PiecePositions new_enemy_pieces = enemy_pieces;

        new_friendly_pieces.king = ((new_friendly_pieces.king & position) != 0) ? (new_friendly_pieces.king & ~position) | new_position : new_friendly_pieces.king;
        new_friendly_pieces.queens = ((new_friendly_pieces.queens & position) != 0) ? (new_friendly_pieces.queens & ~position) | new_position : new_friendly_pieces.queens;
        new_friendly_pieces.rooks = ((new_friendly_pieces.rooks & position) != 0) ? (new_friendly_pieces.rooks & ~position) | new_position : new_friendly_pieces.rooks;
        new_friendly_pieces.bishops = ((new_friendly_pieces.bishops & position) != 0) ? (new_friendly_pieces.bishops & ~position) | new_position : new_friendly_pieces.bishops;
        new_friendly_pieces.knights = ((new_friendly_pieces.knights & position) != 0) ? (new_friendly_pieces.knights & ~position) | new_position : new_friendly_pieces.knights;
        new_friendly_pieces.pawns = ((new_friendly_pieces.pawns & position) != 0) ? (new_friendly_pieces.pawns & ~position) | new_position : new_friendly_pieces.pawns;

        new_enemy_pieces.queens &= ~new_position;
        new_enemy_pieces.rooks &= ~new_position;
        new_enemy_pieces.bishops &= ~new_position;
        new_enemy_pieces.knights &= ~new_position;
        new_enemy_pieces.pawns &= ~new_position;

        //make new board state
        struct BoardState* newBoard = malloc(sizeof(struct BoardState));
        *newBoard = (struct BoardState){
            .turn = (boardState->turn == WHITE) ? BLACK : WHITE,
            .state = ACTIVE,
            .white = (boardState->turn == WHITE) ? new_friendly_pieces : new_enemy_pieces,
            .black = (boardState->turn == WHITE) ? new_enemy_pieces : new_friendly_pieces,
        };
    }

    return NULL;

}

uint64_t calc_king_moves(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions) {
    
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

    if (file == 'a') {
        candidate_positions &= ~HFILE;
    } else if (file == 'h') {
        candidate_positions &= ~AFILE;
    }

    return candidate_positions;
}

uint64_t calc_bishop_moves(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions) {
    uint64_t candidate_positions = 0;


    return candidate_positions;
}

uint64_t calc_rook_moves(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions) {
    
    //get all positions in same row + column
    uint64_t candidate_positions = 0;
    //remove current position
    candidate_positions &= ~position; 
    //remove friendly positions
    candidate_positions &= ~friendly_positions;

    return candidate_positions;   
}

uint64_t calc_queen_moves(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions) {
    return calc_bishop_moves(position, friendly_positions, enemy_positions) | 
        calc_rook_moves(position, friendly_positions, enemy_positions);
}

uint64_t calc_knight_moves(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions) {

    uint64_t candidate_positions = 0;
    char file = bitmapToPosition(position).file;
    
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
        candidate_positions |= position >> 17;

    }

    if (file < 'h') {
        candidate_positions |= position << 15;
        candidate_positions |= position >> 10;
    }

    candidate_positions &= ~friendly_positions;

    return candidate_positions;

}


//TODO: promotion and en-passant
uint64_t calc_pawn_moves(uint64_t position, uint64_t friendly_positions, uint64_t enemy_positions, TURN turn) {

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