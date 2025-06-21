#import "opponent.h"
#import "board.h"
#import <assert.h>
#include <stdlib.h>
#include <stdio.h>
#import "moves.h"
#include <math.h>
#include <string.h>

typedef struct {
    BoardState* state;
    double score;
} EngineMove;


//positive score = White up material
//negative score = Black up material
double scorePosition(BoardState* boardState) {
    double white_score = 0;
    double black_score = 0;


    white_score += __builtin_popcountll(boardState->white.king) * 50;
    white_score += __builtin_popcountll(boardState->white.queens) * 8;
    white_score += __builtin_popcountll(boardState->white.rooks) * 5;
    white_score += __builtin_popcountll(boardState->white.knights) * 3;
    white_score += __builtin_popcountll(boardState->white.bishops) * 3;
    white_score += __builtin_popcountll(boardState->white.pawns);

    black_score += __builtin_popcountll(boardState->black.king) * 50;
    black_score += __builtin_popcountll(boardState->black.queens) * 8;
    black_score += __builtin_popcountll(boardState->black.rooks) * 5;
    black_score += __builtin_popcountll(boardState->black.knights) * 3;
    black_score += __builtin_popcountll(boardState->black.bishops) * 3;
    black_score += __builtin_popcountll(boardState->black.pawns);

    return white_score - black_score;
}


EngineMove getOptimalMove(BoardState* boardState, size_t maxDepth) {


    if (maxDepth == 0) {
        EngineMove tree_end;
        tree_end.state = malloc(sizeof(BoardState));
        memcpy(tree_end.state, boardState, sizeof(BoardState));
        tree_end.score = 0;
        return tree_end;
    }
    
    //no available moves, this is checkmate!
    Moves all_moves = getAllValidMoves(boardState);
    if (all_moves.count == 0) {
        EngineMove checkmate;
        checkmate.state=boardState;
        checkmate.score=(boardState->turn == WHITE) ? 100 : -100;
    }


    BoardState* optimalState;
    //if opponent is black, invert score
    int score_multiple = (boardState->turn == WHITE) ? -1 : 1;
    double smallest_response_score = INFINITY;
    

    //find the move that for which the best response has the smallest score
    for (size_t move = 0; move < all_moves.count; move++) {

        const EngineMove response = getOptimalMove(&all_moves.boards[move], maxDepth - 1);
        const BoardState* responseState = response.state;
        const double side_corrected_score = response.score * score_multiple;

        if (smallest_response_score > side_corrected_score) {
            smallest_response_score = side_corrected_score;
            optimalState = &all_moves.boards[move];
        }

        free(responseState);
    }

    EngineMove optimalMove;
    
    optimalMove.state = malloc(sizeof(BoardState));
    optimalMove.score = scorePosition(optimalMove.state) - smallest_response_score;
    memcpy(optimalMove.state, optimalState, sizeof(BoardState));

    free(all_moves.boards);

    return optimalMove;
}

void makeOpponentMove(BoardState* boardState) {
    assert(boardState->turn == BLACK);
    *boardState = *getOptimalMove(boardState, 4).state;
}


// void makeOpponentMove(BoardState* boardState, size_t maxDepth ) {
//     assert(boardState->turn == BLACK);

// }