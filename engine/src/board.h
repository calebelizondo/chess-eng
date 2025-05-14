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

struct BoardState {
    TURN turn;
    GAME_STATE state;
    struct PiecePositions black;
    struct PiecePositions white;
};

extern const struct BoardState STARTING_BOARD_STATE;
extern char* boardStateToArray(struct BoardState* board_state);

#endif