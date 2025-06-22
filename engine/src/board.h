#include <stdint.h>
#include <stdbool.h>

#ifndef BOARD_H
#define BOARD_H

struct PiecePositions {
    int64_t king;
    int64_t queens;
    int64_t rooks;
    int64_t knights;
    int64_t bishops;
    int64_t pawns;
};

typedef enum { WHITE, BLACK } TURN;

typedef struct {
    char file;
    char row;
} Position;

typedef enum {
    NONE = 0,
    BLACK_QUEENSIDE = 1 << 0,
    WHITE_QUEENSIDE = 1 << 2,
    BLACK_KINGSIDE = 1 << 3,
    WHITE_KINGSIDE = 1 << 4,
} CanCastle;

typedef struct {
    TURN turn;
    struct PiecePositions black;
    struct PiecePositions white;
    uint64_t black_positions;
    uint64_t white_positions;
    CanCastle can_castle;
    uint64_t valid_enpassant;
    uint64_t last_move;
} BoardState;

extern const uint64_t AFILE;
extern const uint64_t BFILE;
extern const uint64_t HFILE;
extern const uint64_t GFILE;

extern const uint64_t QUEENSIDE_BLACK_CASTLE_MASK;
extern const uint64_t QUEENSIDE_WHITE_CASTLE_MASK;
extern const uint64_t KINGSIDE_BLACK_CASTLE_MASK;
extern const uint64_t KINGSIDE_WHITE_CASTLE_MASK;

extern const BoardState STARTING_BOARD_STATE;
extern Position bitmapToPosition(uint64_t position);
extern void printBoard(BoardState* board_state);
extern char* boardStateToArray(BoardState* board_state);
extern char* moveBitmapToString(uint64_t position);
extern uint64_t stringPositionToBitmap(const char* str);
extern void updatePositionBitmap(BoardState* board_state);

#endif