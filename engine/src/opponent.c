#include "opponent.h"
#include "board.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "moves.h"
#include <math.h>
#include <string.h>
#include <limits.h>
#include "trans_table.h"
#include <pthread.h>

#define SEARCH_DEPTH 6
#define MAX_THREADS 40

//positive score = current player up material
//negative score = opposing player up material
int scorePosition(BoardState* boardState) {
    int score = 0;
    bool side = boardState->turn;
    bool other_side = boardState->turn ^ 1;

    score += __builtin_popcountll(boardState->pieces[side][KING]) * 50;
    score += __builtin_popcountll(boardState->pieces[side][QUEEN]) * 9;
    score += __builtin_popcountll(boardState->pieces[side][ROOK]) * 5;
    score += __builtin_popcountll(boardState->pieces[side][KNIGHT]) * 3;
    score += __builtin_popcountll(boardState->pieces[side][BISHOP]) * 3;
    score += __builtin_popcountll(boardState->pieces[side][PAWN]);

    score -= __builtin_popcountll(boardState->pieces[other_side][KING]) * 50;
    score -= __builtin_popcountll(boardState->pieces[other_side][QUEEN]) * 9;
    score -= __builtin_popcountll(boardState->pieces[other_side][ROOK]) * 5;
    score -= __builtin_popcountll(boardState->pieces[other_side][KNIGHT]) * 3;
    score -= __builtin_popcountll(boardState->pieces[other_side][BISHOP]) * 3;
    score -= __builtin_popcountll(boardState->pieces[other_side][PAWN]);

    return score;
}


int negamax(BoardState* state, size_t depth, int alpha, int beta) {

    TEntry entry;
    bool found = read(state->hash, &entry);
    if (found && entry.depth >= depth) {
        return entry.score;
    }

    if (depth == 0) {
        int score = scorePosition(state);
        write(state, depth, score);
        return score;
    }

    BoardState saved_state = *state;
    MoveList moves;
    moves.count = 0;
    getAllValidMoves(state, &moves); 

    int max_score = -50;

    for (size_t move = 0; move < moves.count; move++) {
        applyMove(moves.moves[move], state);

        int score = -negamax(state, depth - 1, -beta, -alpha);
        max_score = (score > max_score) ? score : max_score;
        alpha = (alpha > score) ? alpha : score;

        //restore state
        memcpy(state, &saved_state, sizeof(BoardState));

        if (alpha >= beta) break;
    }
    write(state, depth, max_score);
    return max_score;
}

void* negamaxWorker(void* boardState) {
    int* score = malloc(sizeof(int));
    *score = -negamax((BoardState*)boardState, SEARCH_DEPTH - 1, INT_MIN + 1, INT_MAX - 1);
    return score;
}


int makeOptimalMove(BoardState* boardState, int depth) {
    MoveList moves;
    moves.count = 0;
    getAllValidMoves(boardState, &moves);
    Move best;
    int best_score = INT_MIN;

    static pthread_t threads[MAX_THREADS];
    BoardState boards[MAX_THREADS];

    //make the first set of move-boards
    BoardState copy = *boardState;
    assert(moves.count <= MAX_THREADS);

    for (size_t move = 0; move < moves.count; move++) {
        applyMove(moves.moves[move], boardState);
        boards[move] = *boardState;
        *boardState = copy;
    }

    //dispatch threads
    for (size_t thread = 0; thread < moves.count; thread++) {
        pthread_create(&threads[thread], NULL, negamaxWorker, &boards[thread]);
    }

    //join the threads
    for (size_t thread = 0; thread < moves.count; thread++) {
        void* ret;
        int rc = pthread_join(threads[thread], &ret);
        int* score = (int*)ret;
        if (*score > best_score) {
            best_score = *score;
            best = moves.moves[thread];
        }
        free(score);
    }

    applyMove(best, boardState);

    return best_score;
}

int makeOpponentMove(BoardState* boardState) {
    assert(boardState->turn == BLACK);

    return makeOptimalMove(boardState, SEARCH_DEPTH);
}