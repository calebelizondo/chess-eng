#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "moves.h"
#include <emscripten.h>

struct BoardState* current_board_state = NULL;

int main() {

    current_board_state = malloc(sizeof(STARTING_BOARD_STATE));

    if (current_board_state == NULL) {
        return -1;
    }

    *current_board_state = STARTING_BOARD_STATE;
    
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
char* movePiece(int from, int to) {
    move(from, to, current_board_state);
    return getCurrentBoardState();
}

EMSCRIPTEN_KEEPALIVE
char* getValidPieceMoves(int idx) {
    //temp, black doesnt move yet. 
    current_board_state->turn = WHITE;
    return moveBitmapToString(getValidMoves(idx, current_board_state));
}