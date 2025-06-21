#include "board.h"

#ifndef MOVES_H
#define MOVES_H


typedef struct {
    //where are the valid spots to move?
    uint64_t move_bitmap;
    //new states that would be triggered by each possible moves
    BoardState* boards;
    //the amount of possible moves
    size_t count;
} Moves;


// extern BoardState** calcValidMoves(uint64_t position, BoardState* boardState);
extern Moves getAllValidMoves(BoardState* boardState);
extern void move(uint64_t from, uint64_t to, BoardState* boardState);
extern bool isInCheck(TURN side, BoardState* boardState);
extern Moves getValidMoves(uint64_t piece_mask, BoardState* boardState);
extern void printBinary(uint64_t num);

#endif