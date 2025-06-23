#import "opponent.h"
#import "board.h"
#import <assert.h>
#include <stdlib.h>
#include <stdio.h>
#import "moves.h"
#include <math.h>
#include <string.h>
#include <limits.h>
#include "trans_table.h"

#define SEARCH_DEPTH 4

typedef struct {
    BoardState* state;
    double score;
} EngineMove;


//positive score = White up material
//negative score = Black up material
int scorePosition(BoardState* boardState) {
    int white_score = 0;
    int black_score = 0;


    white_score += __builtin_popcountll(boardState->white[KING]) * 50;
    white_score += __builtin_popcountll(boardState->white[QUEEN]) * 8;
    white_score += __builtin_popcountll(boardState->white[ROOK]) * 5;
    white_score += __builtin_popcountll(boardState->white[KNIGHT]) * 3;
    white_score += __builtin_popcountll(boardState->white[BISHOP]) * 3;
    white_score += __builtin_popcountll(boardState->white[PAWN]);

    black_score += __builtin_popcountll(boardState->black[KING]) * 50;
    black_score += __builtin_popcountll(boardState->black[QUEEN]) * 8;
    black_score += __builtin_popcountll(boardState->black[ROOK]) * 5;
    black_score += __builtin_popcountll(boardState->black[KNIGHT]) * 3;
    black_score += __builtin_popcountll(boardState->black[BISHOP]) * 3;
    black_score += __builtin_popcountll(boardState->black[PAWN]);

    int material_diff = white_score - black_score;
    
    return (boardState->turn == WHITE) ? material_diff : -material_diff;
}


int negamax(BoardState* state, size_t depth, int alpha, int beta) {

    TEntry entry;
    bool found = read(state, &entry);
    // if (found && entry.depth >= depth) {
    //     return entry.score;
    // }
    // if (found) {
    //     printf("found score:");
    //     printf("%d\n", entry.score);
    // }

    if (depth == 0) {
        int score = scorePosition(state);
        write(state, depth, score);
        return score;
    }

    BoardState saved_state = *state;
    MoveList moves = getAllValidMoves(state); 
    if (moves.count <= 0) {
        int score = -50;
        write(state, depth, score);
        return score; //checkmate!!
    }

    int max_score = INT_MIN;

    for (size_t move = 0; move < moves.count; move++) {
        applyMove(moves.moves[move], state);

        //printBoard(state);

        int score = -negamax(state, depth - 1, -beta, -alpha);
        max_score = (score > max_score) ? score : max_score;
        alpha = (alpha > score) ? alpha : score;

        //restore state
        *state = saved_state;

        if (alpha >= beta) break;
    }


    write(state, depth, max_score);
    return max_score;

}

Move getOptimalMove(BoardState* boardState, int depth) {
    MoveList moves = getAllValidMoves(boardState);
    Move best;
    int best_score = INT_MIN;

    for (size_t i = 0; i < moves.count; ++i) {
        BoardState copy = *boardState;
        applyMove(moves.moves[i], &copy);

        int score = -negamax(&copy, depth - 1, INT_MIN + 1, INT_MAX - 1);

        if (score > best_score) {
            best_score = score;
            best = moves.moves[i];
        }

        // printf("%d\n", score);
        // printBoard(&copy);
    }

    return best;
}

void makeOpponentMove(BoardState* boardState) {
    assert(boardState->turn == BLACK);

    Move move = getOptimalMove(boardState, SEARCH_DEPTH);
    applyMove(move, boardState);
}