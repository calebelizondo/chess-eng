#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "moves.h"
#include <emscripten.h>
#include <stdbool.h>

BoardState* current_board_state = NULL;

int main() {

    current_board_state = malloc(sizeof(STARTING_BOARD_STATE));
    updatePositionBitmap(current_board_state);

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
char* movePiece(char* from, char* to, bool isCastle, bool isEnpassant, bool isPromotion, char* promoteTo) {
    move(stringPositionToBitmap(from), stringPositionToBitmap(to), current_board_state);
    updatePositionBitmap(current_board_state);
    return getCurrentBoardState();
}

EMSCRIPTEN_KEEPALIVE
char* getValidPieceMoves(char* piece) {

    printf("%d\n", 1);
    //temp, black doesnt move yet. 
    current_board_state->turn = WHITE;
    updatePositionBitmap(current_board_state);
    Moves moves = getValidMoves(stringPositionToBitmap(piece), current_board_state);

    //temp,for now black doesn't move
    free(moves.boards);
    return moveBitmapToString(moves.move_bitmap);
}

EMSCRIPTEN_KEEPALIVE 
bool isEnemyInCheck() {
    return isInCheck(BLACK, current_board_state);
}

EMSCRIPTEN_KEEPALIVE 
bool isPlayerInCheck() {
    return isInCheck(WHITE, current_board_state);
}