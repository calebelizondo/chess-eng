#include "board.h"
#include <stdio.h>
#include <assert.h>

const struct BoardState STARTING_BOARD_STATE = {
    .turn= WHITE,
    .state= ACTIVE,
    .black = {
        .king = 0b0000100000000000000000000000000000000000000000000000000000000000,
        .queens = 0b0001000000000000000000000000000000000000000000000000000000000000,
        .rooks = 0b1000000100000000000000000000000000000000000000000000000000000000,
        .knights = 0b0100001000000000000000000000000000000000000000000000000000000000,
        .bishops = 0b0010010000000000000000000000000000000000000000000000000000000000,
        .pawns = 0b0000000011111111000000000000000000000000000000000000000000000000,
    },
    .white = {
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

        if (board_state->white.king & mask) { board[i] = 'K'; }
        else if (board_state->white.queens & mask) { board[i] = 'Q'; }
        else if (board_state->white.rooks & mask) { board[i] = 'R'; }
        else if (board_state->white.knights & mask) { board[i] = 'N'; }
        else if (board_state->white.bishops & mask) { board[i] = 'B'; }
        else if (board_state->white.pawns & mask) { board[i] = 'P'; }

        else if (board_state->black.king & mask) { board[i] = 'k'; }
        else if (board_state->black.queens & mask) { board[i] = 'q'; }
        else if (board_state->black.rooks & mask) { board[i] = 'r'; }
        else if (board_state->black.knights & mask) { board[i] = 'n'; }
        else if (board_state->black.bishops & mask) { board[i] = 'b'; }
        else if (board_state->black.pawns & mask) { board[i] = 'p'; }
        else board[i] = '.';
    }

    board[64] = '\0';
    return board;
}


void printBoard(struct BoardState* board_state) {
    char* board = boardStateToArray(board_state);

    printf("  +------------------------+\n");
    for (int rank = 7; rank >= 0; rank--) {
        printf("%d |", rank + 1);
        for (int file = 0; file < 8; file++) {
            int index = rank * 8 + file;
            printf(" %c", board[index]);
        }
        printf(" |\n");
    }
    printf("  +------------------------+\n");
    printf("    a b c d e f g h\n");
}


char* moveBitmapToString(uint64_t position) {

    static char move_str[65];

    for (int i = 0; i < 64; i++) {
        if ((position >> i) & 1ULL) {
            move_str[i] = 'x';
        } else {
            move_str[i] = '.';
        }
    }
    return move_str;
}

Position bitmapToPosition(uint64_t position) {
    Position pos; 
    assert(position != 0 && (position & (position - 1)) == 0);

    int index = __builtin_ctzll(position);

    int rowIndex = index / 8;
    int colIndex = index % 8;

    pos.row = rowIndex + 1;
    pos.file = 'a' + colIndex;

    return pos;
    
}
