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
typedef enum { ACTIVE, INACTIVE } GAME_STATE;

typedef struct {
    char file;
    char row;
} Position;

typedef struct {
    TURN turn;
    GAME_STATE state;
    struct PiecePositions black;
    struct PiecePositions white;
    uint64_t black_positions;
    uint64_t white_positions;
    bool black_can_castle;
    bool white_can_castle;
} BoardState;

extern const uint64_t AFILE;
extern const uint64_t BFILE;
extern const uint64_t HFILE;
extern const uint64_t GFILE;

extern const BoardState STARTING_BOARD_STATE;
extern Position bitmapToPosition(uint64_t position);
extern void printBoard(BoardState* board_state);
extern char* boardStateToArray(BoardState* board_state);
extern char* moveBitmapToString(uint64_t position);
extern uint64_t stringPositionToBitmap(const char* str);
extern void updatePositionBitmap(BoardState* board_state);

#endif