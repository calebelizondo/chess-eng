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
char* getCurrentBoardState() {
    return boardStateToArray(current_board_state);
}