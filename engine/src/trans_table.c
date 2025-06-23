#include "board.h"
#include "moves.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TABLE_SIZE (1024 * 1024) 
#define ZORBIST_SEED 42

typedef enum {
    EXACT,
    LOWERBOUND,
    UPPERBOUND
} BoundType;

typedef struct {
    uint64_t key;
    int depth;
    int score;
    Move best_move;
    BoundType bound;
} TEntry;


TEntry* trans_table;
uint64_t zobrist_table[15][64];

void initZorbistTable() {
    srand(ZORBIST_SEED);
    for (int piece = 0; piece < 15; ++piece)
        for (int square = 0; square < 64; ++square)
            zobrist_table[piece][square] = ((uint64_t)rand() << 32) | rand();
}

void initTransTable() {
    trans_table = malloc(sizeof(TEntry) * TABLE_SIZE);
    assert(trans_table != NULL);
    initZorbistTable();
}

uint64_t hash(const BoardState* const boardState) {
    uint64_t h = 0;

    // for (size_t i = 0; i < PIECE)
    
    return h;
}