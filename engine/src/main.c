#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "moves.h"
#include <emscripten.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "opponent.h"
#include "trans_table.h"

BoardState* current_board_state = NULL;
MoveList* available_moves = NULL;
int score = 0;

int main() {

    initTransTable();

    current_board_state = malloc(sizeof(STARTING_BOARD_STATE));
    available_moves = malloc(sizeof(MoveList));
    updatePositions(current_board_state);
    initAttackMaps(current_board_state);
    uint64_t starting_hash = hash(current_board_state);
    current_board_state->hash = starting_hash;


    if (current_board_state == NULL || available_moves == NULL) {
        return -1;
    }

    *current_board_state = STARTING_BOARD_STATE;
    available_moves->count = 0;
    initMoveMaps();
    
    return 0;
}

EMSCRIPTEN_KEEPALIVE
void printCurrentBoardState() {
    printBoard(current_board_state);
}

EMSCRIPTEN_KEEPALIVE
char* resetGame() {
    *current_board_state = STARTING_BOARD_STATE;
    return boardStateToArray(current_board_state);
}

EMSCRIPTEN_KEEPALIVE
char* getCurrentBoardState() {
    return boardStateToArray(current_board_state);
}

EMSCRIPTEN_KEEPALIVE
char* movePiece(char* from, char* to, bool isCastle, bool isEnpassant, bool isPromotion, char* promoteTo) {
    assert(available_moves != NULL);

    //find the move that matches our to, from move
    uint64_t from_bitmap = stringPositionToBitmap(from);
    uint64_t to_bitmap = stringPositionToBitmap(to);
    bool move_found = false;
    for (size_t move = 0; move < available_moves->count; move++) {
        Move cur_move = available_moves->moves[move];

        if ((cur_move.to == to_bitmap) && (cur_move.from == from_bitmap)) {
            applyMove(cur_move, current_board_state);
            move_found = true;
            break;
        } 
    }

    assert(move_found);
    available_moves->count = 0;
    //-opponent's score = player's score
    score = -makeOpponentMove(current_board_state);

    return getCurrentBoardState();
}

EMSCRIPTEN_KEEPALIVE
char* getValidPieceMoves(char* piece) {

    //assert(current_board_state->turn == WHITE);
    available_moves->count = 0;
    getValidMoves(stringPositionToBitmap(piece), current_board_state, available_moves);

    uint64_t moves_bitmap = 0;
    for (size_t i = 0; i < available_moves->count; i++) {
        moves_bitmap |= available_moves->moves[i].to;
    }

    return moveBitmapToString(moves_bitmap);
}


EMSCRIPTEN_KEEPALIVE 
bool isEnemyInCheck() {
    return isInCheck(BLACK, current_board_state);
}

EMSCRIPTEN_KEEPALIVE 
bool isPlayerInCheck() {
    return isInCheck(WHITE, current_board_state);
}

EMSCRIPTEN_KEEPALIVE
int getScore() {
    return score;
}

EMSCRIPTEN_KEEPALIVE
bool isWhitesTurn() {
    return current_board_state->turn;
}