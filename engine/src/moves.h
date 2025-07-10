#ifndef MOVES_H
#define MOVES_H

#include "board.h"
#include <string.h>

typedef enum {
    FLAG_NONE        = 0,
    FLAG_EN_PASSANT  = 1 << 0,
    FLAG_CASTLE_KINGSIDE = 1 << 1,
    FLAG_CASTLE_QUEENSIDE = 1 << 2,
    FLAG_CAPTURE     = 1 << 3, 
    FLAG_PAWN_MOVE_TWO = 1 << 4, 
    FLAG_PROMOTION = 1 << 5,
} MoveFlags;

typedef struct {
    uint64_t from;
    uint64_t to;
    MoveFlags flags;
    PieceType piece;
} Move;

typedef struct {
    Move moves[256];
    size_t count;
} MoveList;

extern void getAllValidMoves(const BoardState* const boardState, MoveList* buffer);
extern void applyMove(Move move, BoardState* boardState);
extern bool isInCheck(SIDE side, const BoardState* const boardState);
extern void getValidMoves(uint64_t piece_mask, const BoardState* const boardState, MoveList* buffer);
extern void printBinary(uint64_t num);
extern void initMoveMaps();
extern void initAttackMaps(BoardState* boardState);

#endif