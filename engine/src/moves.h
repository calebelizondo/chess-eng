#include "board.h"

#ifndef MOVES_H
#define MOVES_H

extern struct BoardState** calcValidMoves(uint64_t position, struct BoardState* boardState);

#endif