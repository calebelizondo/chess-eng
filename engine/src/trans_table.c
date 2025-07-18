#include "board.h"
#include "moves.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "trans_table.h"
#include <stdio.h>

#define TABLE_SIZE ((1024 * 1024) /2)
#define ZORBIST_SEED 42
#define ZORBIST_SIZE ((PIECE_TYPE_COUNT * 2) + 3)


TEntry* trans_table;
uint64_t zobrist_table[ZORBIST_SIZE][64];

void initZorbistTable() {
    srand(ZORBIST_SEED);
    for (int piece = 0; piece < ZORBIST_SIZE; ++piece)
        for (int square = 0; square < 64; ++square) {
            zobrist_table[piece][square] = ((uint64_t)rand() << 32) | rand();
        }
}

void initTransTable() {
    trans_table = malloc(sizeof(TEntry) * TABLE_SIZE);
    assert(trans_table != NULL);
    initZorbistTable();
}


void XORPiece(BoardState* state, int idx, PieceType pt, SIDE side) {
    ZTABLE_KEY key = (pt + (PIECE_TYPE_COUNT * side));
    uint64_t new_hash = state->hash ^ zobrist_table[key][idx]; 
    state->hash = new_hash;
}

void XOR(BoardState* state, int idx, ZTABLE_KEY key) {
    state->hash ^= zobrist_table[key][idx];
}

uint64_t hash(const BoardState* const boardState) {
    uint64_t h = 0;

    //hash white pieces
    for (size_t i = 0; i < PIECE_TYPE_COUNT; i++) {
        uint64_t bb = boardState->pieces[WHITE][i];
        while (bb) {
            int sq = __builtin_ctzll(bb);
            h ^= zobrist_table[i][sq];
            bb &= bb - 1;
        }
    }

    //hash black pieces
    for (size_t i = PIECE_TYPE_COUNT; i < PIECE_TYPE_COUNT * 2; i++) {
        uint64_t bb = boardState->pieces[BLACK][i - PIECE_TYPE_COUNT];
        while (bb) {
            int sq = __builtin_ctzll(bb);
            h ^= zobrist_table[i][sq];
            bb &= bb - 1;
        }
    }

    //hash en-passant locs
    uint64_t en_ps_bb = boardState->valid_enpassant;
    while (en_ps_bb) {
        int sq = __builtin_ctzll(en_ps_bb);
        h ^= zobrist_table[EN_PASSANT][sq];
        en_ps_bb &= en_ps_bb - 1;
    }

    //hash castling rights
    h ^= zobrist_table[CASTLE_RIGHTS][boardState->can_castle];

    //hash turn
    h ^= zobrist_table[TURN][boardState->turn];
    
    return h;
}


bool read(uint64_t zhash, TEntry* buffer) {
    uint64_t index = zhash % (uint64_t) TABLE_SIZE;

    if (trans_table[index].key == zhash) {

        // *buffer = trans_table[index];

        memcpy(buffer, &trans_table[index], sizeof(TEntry));
        return true;
    }

    return false;
} 

void write(const BoardState* const boardState, int depth, int score) {

    uint64_t zhash = boardState->hash;
    uint64_t index = zhash % (uint64_t) TABLE_SIZE;

    trans_table[index] = (TEntry) {
        .key=zhash,
        .depth=depth,
        .score=score,
    };
}