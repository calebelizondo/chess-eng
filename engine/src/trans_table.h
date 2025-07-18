#ifndef TRANS_TABLE_H
#define TRANS_TABLE_H


typedef struct {
    uint64_t key;
    int depth;
    int score;
} TEntry;

typedef enum {
    W_PAWN, 
    W_KING, 
    W_QUEEN, 
    W_BISHOP, 
    W_KNIGHT, 
    W_ROOK,
    B_PAWN, 
    B_KING, 
    B_QUEEN, 
    B_BISHOP, 
    B_KNIGHT, 
    B_ROOK,
    EN_PASSANT,
    CASTLE_RIGHTS,
    TURN
} ZTABLE_KEY;


extern void initTransTable();

extern bool read(uint64_t zhash, TEntry* buffer);
extern void write(const BoardState* const boardState, int depth, int score);
extern uint64_t hash(const BoardState* const boardState);
extern void XORPiece(BoardState* state, int idx, PieceType pt, SIDE side);
extern void XOR(BoardState* state, int idx, ZTABLE_KEY key);

#endif