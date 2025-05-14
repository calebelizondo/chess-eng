#include "board.h"
#include <stdio.h>

const struct BoardState STARTING_BOARD_STATE = {
    .turn= WHITE,
    .state= ACTIVE,
    .white = {
        .king = 0b0000100000000000000000000000000000000000000000000000000000000000,
        .queens = 0b0001000000000000000000000000000000000000000000000000000000000000,
        .rooks = 0b1000000100000000000000000000000000000000000000000000000000000000,
        .knights = 0b0100001000000000000000000000000000000000000000000000000000000000,
        .bishops = 0b0010010000000000000000000000000000000000000000000000000000000000,
        .pawns = 0b0000000011111111000000000000000000000000000000000000000000000000,
    },
    .black = {
        .king = 0b0000000000000000000000000000000000000000000000000000000000001000,
        .queens = 0b000000000000000000000000000000000000000000000000000000000010000,
        .rooks = 0b0000000000000000000000000000000000000000000000000000000010000001,
        .knights = 0b0000000000000000000000000000000000000000000000000000000001000010,
        .bishops = 0b0000000000000000000000000000000000000000000000000000000000100100,
        .pawns = 0b0000000000000000000000000000000000000000000000001111111100000000,
    }
};

char* boardStateToArray(struct BoardState* board_state) {
    static char board[65];

    for (int i = 0; i < 64; i++) {

        uint64_t mask = 1ULL << i;

        if (board_state->white.king & mask) { board[i] = 'K'; continue; }
        if (board_state->white.queens & mask) { board[i] = 'Q'; continue; }
        if (board_state->white.rooks & mask) { board[i] = 'R'; continue; }
        if (board_state->white.knights & mask) { board[i] = 'N'; continue; }
        if (board_state->white.bishops & mask) { board[i] = 'B'; continue; }
        if (board_state->white.pawns & mask) { board[i] = 'P'; continue; }

        if (board_state->black.king & mask) { board[i] = 'k'; continue; }
        if (board_state->black.queens & mask) { board[i] = 'q'; continue; }
        if (board_state->black.rooks & mask) { board[i] = 'r'; continue; }
        if (board_state->black.knights & mask) { board[i] = 'n'; continue; }
        if (board_state->black.bishops & mask) { board[i] = 'b'; continue; }
        if (board_state->black.pawns & mask) { board[i] = 'p'; continue; }
        board[i] = '.';
    }

    board[64] = '\0';
    return board;
}
