#include "board.h"

#ifndef MOVES_H
#define MOVES_H

extern struct BoardState** calcValidMoves(uint64_t position, struct BoardState* boardState);
extern void move(int from, int to, struct BoardState* boardState);
extern uint64_t getValidMoves(int idx, struct BoardState* boardState);

#endif