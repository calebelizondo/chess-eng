#include "board.h"
#include <stdio.h>
#include <assert.h>
#include "utils.h"

const uint64_t AFILE = 0b10000000100000001000000010000000100000001000000010000000;
const uint64_t BFILE = 0b01000000010000000100000001000000010000000100000001000000;
const uint64_t HFILE = 0b00000001000000010000000100000001000000010000000100000001;
const uint64_t GFILE = 0b00000010000000100000001000000010000000100000001000000010;

const BoardState STARTING_BOARD_STATE = {
    .turn= WHITE,
    .state= ACTIVE,
    .black_positions = 0,
    .white_positions = 0,
    .white_can_castle = true,
    .black_can_castle = true,
    .last_move = 0,
    .black = {
        .king = 0b0000100000000000000000000000000000000000000000000000000000000000,
        .queens = 0b0001000000000000000000000000000000000000000000000000000000000000,
        .rooks = 0b1000000100000000000000000000010000000000000000000000000000000000,
        .knights = 0b0100001000000000000000000000000000000000000000000000000000000000,
        .bishops = 0b0010010000000000000000000000000000000000000000000000000000000000,
        .pawns = 0b0000000011111111000000000000000000000000000000000000000000000000,
    },
    .white = {
        .king = 0b0000000000000000000000000000000000000000000000000000000000001000,
        .queens = 0b00000000000000000000000000000000000000000000000000000000010000,
        .rooks = 0b0000000000000000000000000000000000000000000000000000000010000001,
        .knights = 0b0000000000000000000000000000000000000000000000000000000001000010,
        .bishops = 0b0000000000000000000000000000000000000000000000000000000000100100,
        .pawns = 0b0000000000000000000000000000000000000000000000001111111100000000,
    }
};

void updatePositionBitmap(BoardState* board_state) {
    board_state->black_positions = 
        board_state->black.king | 
        board_state->black.queens |
        board_state->black.pawns | 
        board_state->black.rooks |
        board_state->black.bishops |
        board_state->black.knights;

    board_state->white_positions = 
        board_state->white.king | 
        board_state->white.queens |
        board_state->white.pawns | 
        board_state->white.rooks |
        board_state->white.bishops |
        board_state->white.knights;
}

char* boardStateToArray(BoardState* board_state) {
    static char board[65];

    for (int i = 0; i < 64; i++) {

        uint64_t mask = 1ULL << (63 - i);

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


void printBoard(BoardState* board_state) {
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


char* moveBitmapToString(uint64_t moves) {
    static char board[65];

    for (int i = 0; i < 64; i++) {

        uint64_t mask = 1ULL << i;

        if (moves & mask) { board[i] = 'x'; }
        else board[i] = '.';
    }

    board[64] = '\0';
    return board;
}

Position bitmapToPosition(uint64_t position) {
    Position pos; 
    assert(position != 0 && (position & (position - 1)) == 0);

    int index = __builtin_ctzll(position);

    int rowIndex = index / 8;
    int colIndex = index % 8;

    pos.row = '1' + rowIndex;
    pos.file = 'a' + (7 - colIndex);

    return pos;
    
}


uint64_t stringPositionToBitmap(const char* str) {
    assert(str[0] >= 'a' && str[0] <= 'h');
    assert(str[1] >= '1' && str[1] <= '8');

    int file = str[0] - 'a';
    int rank = str[1] - '1';

    int index = (rank) * 8 + (7 - file);

    uint64_t bin = 1ULL << index;

    return bin;
}