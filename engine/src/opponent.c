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

#define SEARCH_DEPTH 5
#define USE_TRANS_TABLE true

//positive score = White up material
//negative score = Black up material
int scorePosition(BoardState* boardState) {
    int white_score = 0;
    int black_score = 0;


    white_score += __builtin_popcountll(boardState->p_positions[WHITE][KING]) * 50;
    white_score += __builtin_popcountll(boardState->p_positions[WHITE][QUEEN]) * 8;
    white_score += __builtin_popcountll(boardState->p_positions[WHITE][ROOK]) * 5;
    white_score += __builtin_popcountll(boardState->p_positions[WHITE][KNIGHT]) * 3;
    white_score += __builtin_popcountll(boardState->p_positions[WHITE][BISHOP]) * 3;
    white_score += __builtin_popcountll(boardState->p_positions[WHITE][PAWN]);

    black_score += __builtin_popcountll(boardState->p_positions[BLACK][KING]) * 50;
    black_score += __builtin_popcountll(boardState->p_positions[BLACK][QUEEN]) * 8;
    black_score += __builtin_popcountll(boardState->p_positions[BLACK][ROOK]) * 5;
    black_score += __builtin_popcountll(boardState->p_positions[BLACK][KNIGHT]) * 3;
    black_score += __builtin_popcountll(boardState->p_positions[BLACK][BISHOP]) * 3;
    black_score += __builtin_popcountll(boardState->p_positions[BLACK][PAWN]);

    int material_diff = white_score - black_score;
    
    return (boardState->turn == WHITE) ? material_diff : -material_diff;
}


int negamax(BoardState* state, size_t depth, int alpha, int beta) {

    if (USE_TRANS_TABLE) {
        TEntry entry;
        bool found = read(state, &entry);
        if (found && entry.depth >= depth) {
            return entry.score;
        }
    }

    if (depth == 0) {
        int score = scorePosition(state);
        if (USE_TRANS_TABLE) write(state, depth, score);
        return score;
    }

    BoardState saved_state = *state;
    MoveList moves;
    moves.count = 0;
    getAllValidMoves(state, &moves); 
    if (moves.count <= 0) {
        int score = -50;
        if (USE_TRANS_TABLE) write(state, depth, score);
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


    if (USE_TRANS_TABLE) write(state, depth, max_score);
    return max_score;

}

Move getOptimalMove(BoardState* boardState, int depth) {
    MoveList moves;
    moves.count = 0;
    getAllValidMoves(boardState, &moves);
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