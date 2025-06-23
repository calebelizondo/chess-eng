#include "board.h"
#include <stdio.h>
#include <assert.h>
#include "utils.h"

const uint64_t AFILE = 0b10000000100000001000000010000000100000001000000010000000;
const uint64_t BFILE = 0b01000000010000000100000001000000010000000100000001000000;
const uint64_t HFILE = 0b00000001000000010000000100000001000000010000000100000001;
const uint64_t GFILE = 0b00000010000000100000001000000010000000100000001000000010;


const uint64_t QUEENSIDE_BLACK_CASTLE_MASK = 0b0111000000000000000000000000000000000000000000000000000000000000;
const uint64_t KINGSIDE_BLACK_CASTLE_MASK =  0b0000011000000000000000000000000000000000000000000000000000000000;
const uint64_t KINGSIDE_WHITE_CASTLE_MASK =  0b0000000000000000000000000000000000000000000000000000000000000110;
const uint64_t QUEENSIDE_WHITE_CASTLE_MASK = 0b0000000000000000000000000000000000000000000000000000000001110000;


const BoardState STARTING_BOARD_STATE = {
    .turn= WHITE,
    .black_positions = 0,
    .white_positions = 0,
    .can_castle = WHITE_QUEENSIDE | WHITE_KINGSIDE | BLACK_KINGSIDE | BLACK_QUEENSIDE,
    .valid_enpassant = 0,
    .last_move = 0,
    .black = {
        [KING] = 0b0000100000000000000000000000000000000000000000000000000000000000,
        [QUEEN] = 0b0001000000000000000000000000000000000000000000000000000000000000,
        [ROOK] = 0b1000000100000000000000000000000000000000000000000000000000000000,
        [KNIGHT] = 0b0100001000000000000000000000000000000000000000000000000000000000,
        [BISHOP] = 0b0010010000000000000000000000000000000000000000000000000000000000,
        [PAWN] = 0b0000000011111111000000000000000000000000000000000000000000000000,
    },
    .white = {
        [KING] = 0b0000000000000000000000000000000000000000000000000000000000001000,
        [QUEEN] = 0b00000000000000000000000000000000000000000000000000000000010000,
        [ROOK] = 0b0000000000000000000000000000000000000000000000000000000010000001,
        [KNIGHT] = 0b0000000000000000000000000000000000000000000000000000000001000010,
        [BISHOP] = 0b0000000000000000000000000000000000000000000000000000000000100100,
        [PAWN] = 0b0000000000000000000000000000000000000000000000001111111100000000,
    }
};

void updatePositionBitmap(BoardState* board_state) {
    board_state->black_positions = 
        board_state->black[KING] | 
        board_state->black[QUEEN] |
        board_state->black[PAWN] | 
        board_state->black[ROOK] |
        board_state->black[BISHOP] |
        board_state->black[KNIGHT];

    board_state->white_positions = 
        board_state->white[KING] | 
        board_state->white[QUEEN] |
        board_state->white[PAWN] | 
        board_state->white[ROOK] |
        board_state->white[BISHOP] |
        board_state->white[KNIGHT];
}

char* boardStateToArray(BoardState* board_state) {
    static char board[65];

    for (int i = 0; i < 64; i++) {

        uint64_t mask = 1ULL << (63 - i);

        if (board_state->white[KING] & mask) { board[i] = 'K'; }
        else if (board_state->white[QUEEN] & mask) { board[i] = 'Q'; }
        else if (board_state->white[ROOK] & mask) { board[i] = 'R'; }
        else if (board_state->white[KNIGHT] & mask) { board[i] = 'N'; }
        else if (board_state->white[BISHOP] & mask) { board[i] = 'B'; }
        else if (board_state->white[PAWN] & mask) { board[i] = 'P'; }

        else if (board_state->black[KING] & mask) { board[i] = 'k'; }
        else if (board_state->black[QUEEN] & mask) { board[i] = 'q'; }
        else if (board_state->black[ROOK] & mask) { board[i] = 'r'; }
        else if (board_state->black[KNIGHT] & mask) { board[i] = 'n'; }
        else if (board_state->black[BISHOP] & mask) { board[i] = 'b'; }
        else if (board_state->black[PAWN] & mask) { board[i] = 'p'; }
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