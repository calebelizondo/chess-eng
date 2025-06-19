#include "board.h"

#ifndef MOVES_H
#define MOVES_H

extern struct BoardState** calcValidMoves(uint64_t position, struct BoardState* boardState);
extern void move(uint64_t from, uint64_t to, struct BoardState* boardState);
extern uint64_t getValidMoves(uint64_t piece_mask, struct BoardState* boardState);
extern void printBinary(uint64_t num);

#endif