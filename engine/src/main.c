#include <stdio.h>
#include <stdlib.h>
#include "board.h"

struct BoardState* current_board_state = NULL;

int main() {

    current_board_state = malloc(sizeof(STARTING_BOARD_STATE));

    if (current_board_state == NULL) {
        return -1;
    }

    *current_board_state = STARTING_BOARD_STATE;

    char* board_str = boardStateToArray(current_board_state);

    printf(board_str);

    printf("board initialized!\n");
    return 0;
}