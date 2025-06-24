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
    .positions = {0b0000000000000000000000000000000000000000000000001111111111111111, 0b1111111111111111000000000000000000000000000000000000000000000000, },
    .can_castle = WHITE_QUEENSIDE | WHITE_KINGSIDE | BLACK_KINGSIDE | BLACK_QUEENSIDE,
    .valid_enpassant = 0,
    .last_move = 0,
    .p_positions = { 
        [BLACK] = {
            [KING] = 0b0000100000000000000000000000000000000000000000000000000000000000,
            [QUEEN] = 0b0001000000000000000000000000000000000000000000000000000000000000,
            [ROOK] = 0b1000000100000000000000000000000000000000000000000000000000000000,
            [KNIGHT] = 0b0100001000000000000000000000000000000000000000000000000000000000,
            [BISHOP] = 0b0010010000000000000000000000000000000000000000000000000000000000,
            [PAWN] = 0b0000000011111111000000000000000000000000000000000000000000000000,
        },
        [WHITE] = {
            [KING] = 0b0000000000000000000000000000000000000000000000000000000000001000,
            [QUEEN] = 0b00000000000000000000000000000000000000000000000000000000010000,
            [ROOK] = 0b0000000000000000000000000000000000000000000000000000000010000001,
            [KNIGHT] = 0b0000000000000000000000000000000000000000000000000000000001000010,
            [BISHOP] = 0b0000000000000000000000000000000000000000000000000000000000100100,
            [PAWN] = 0b0000000000000000000000000000000000000000000000001111111100000000,
        }
    }
};

void updatePositionBitmap(BoardState* board_state) {
    board_state->positions[BLACK] = 
        board_state->p_positions[BLACK][KING] | 
        board_state->p_positions[BLACK][QUEEN] |
        board_state->p_positions[BLACK][PAWN] | 
        board_state->p_positions[BLACK][ROOK] |
        board_state->p_positions[BLACK][BISHOP] |
        board_state->p_positions[BLACK][KNIGHT];

    board_state->positions[WHITE] = 
        board_state->p_positions[WHITE][KING] | 
        board_state->p_positions[WHITE][QUEEN] |
        board_state->p_positions[WHITE][PAWN] | 
        board_state->p_positions[WHITE][ROOK] |
        board_state->p_positions[WHITE][BISHOP] |
        board_state->p_positions[WHITE][KNIGHT];
}

char* boardStateToArray(BoardState* board_state) {
    static char board[65];

    for (int i = 0; i < 64; i++) {

        uint64_t mask = 1ULL << (63 - i);

        if (board_state->p_positions[WHITE][KING] & mask) { board[i] = 'K'; }
        else if (board_state->p_positions[WHITE][QUEEN] & mask) { board[i] = 'Q'; }
        else if (board_state->p_positions[WHITE][ROOK] & mask) { board[i] = 'R'; }
        else if (board_state->p_positions[WHITE][KNIGHT] & mask) { board[i] = 'N'; }
        else if (board_state->p_positions[WHITE][BISHOP] & mask) { board[i] = 'B'; }
        else if (board_state->p_positions[WHITE][PAWN] & mask) { board[i] = 'P'; }

        else if (board_state->p_positions[BLACK][KING] & mask) { board[i] = 'k'; }
        else if (board_state->p_positions[BLACK][QUEEN] & mask) { board[i] = 'q'; }
        else if (board_state->p_positions[BLACK][ROOK] & mask) { board[i] = 'r'; }
        else if (board_state->p_positions[BLACK][KNIGHT] & mask) { board[i] = 'n'; }
        else if (board_state->p_positions[BLACK][BISHOP] & mask) { board[i] = 'b'; }
        else if (board_state->p_positions[BLACK][PAWN] & mask) { board[i] = 'p'; }
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
    assert(position != 0);

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

PieceType getPieceType(uint64_t piece_mask, const BoardState* const boardState) {
    for (int pt = 0; pt < PIECE_TYPE_COUNT; pt++) {
        if ((boardState->p_positions[WHITE][pt] | boardState->p_positions[BLACK][pt]) & piece_mask) {
            return (PieceType)pt;
        }
    }
    assert(false);
}