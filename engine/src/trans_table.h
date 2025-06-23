#ifndef TRANS_TABLE_H
#define TRANS_TABLE_H

// typedef enum {
//     EXACT,
//     LOWERBOUND,
//     UPPERBOUND
// } BoundType;

typedef struct {
    uint64_t key;
    int depth;
    int score;
} TEntry;

extern void initTransTable();

extern bool read(const BoardState* const boardState, TEntry* buffer);
extern void write(const BoardState* const boardState, int depth, int score) ;

#endif