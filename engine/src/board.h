#include <stdint.h>
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

struct BoardState {
    TURN turn;
    GAME_STATE state;
    struct PiecePositions black;
    struct PiecePositions white;
};

extern const uint64_t AFILE;
extern const uint64_t BFILE;
extern const uint64_t HFILE;
extern const uint64_t GFILE;

extern const struct BoardState STARTING_BOARD_STATE;
extern Position bitmapToPosition(uint64_t position);
extern void printBoard(struct BoardState* board_state);
extern char* boardStateToArray(struct BoardState* board_state);
extern char* moveBitmapToString(uint64_t position);
extern uint64_t stringPositionToBitmap(const char* str);

#endif