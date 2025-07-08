#include "maps.h"
#include "board.h"

/*
    index 0: position on board
    index 1: direction (up, down etc)
    index 2: possible new position (or 0 if no more)
*/
uint64_t ROOK_MOVE_MAP[64][4][8];
uint64_t BISHOP_MOVE_MAP[64][4][8];


/*
    index 0: position on board
    index 1: possible new position (or 0 if no more)
*/
uint64_t KNIGHT_MOVE_MAP[64][8];

/*
    index 0: position on board -> all places that the piece could move to
    (used to check if we need to recompute attack maps i.e. identify discover attacks)
*/
uint64_t ROOK_RAY_MAP[64];
uint64_t BISHOP_RAY_MAP[64];


//from -> to
uint64_t RAYS[64][64];

void initRays() {
    for (int from = 0; from < 64; from++) {
        for (int to = 0; to < 64; to++) {
            if (from == to) {
                RAYS[from][to] = 0;
                continue;
            }

            int dr = (to / 8) - (from / 8);
            int df = (to % 8) - (from % 8);

            int step = 0;

            if (dr == 0) step = (df > 0) ? 1 : -1; // horizontal
            else if (df == 0) step = (dr > 0) ? 8 : -8; // vertical
            else if (dr == df) step = (dr > 0) ? 9 : -9; // diagonal //
            else if (dr == -df) step = (dr > 0) ? 7 : -7; // diagonal /
            else {
                RAYS[from][to] = 0;
                continue; 
            }

            uint64_t ray = 0;
            int sq = from + step;
            while (sq != to) {
                ray |= 1ULL << sq;
                sq += step;
            }

            RAYS[from][to] = ray;
        }
    }
}
void initMoveMaps() {

    //init rook map
    for (size_t i = 0; i < 64; i++) {
        ROOK_RAY_MAP[i] = 0;
        uint64_t position = 1ULL << i;
        //used to prevent wrap-around
        uint64_t row_mask = 0xFFULL << (8 * (i / 8));
        for (size_t up = 0; up < 8; up++) {
            uint64_t n_position = position << (8 * (up + 1));
            ROOK_RAY_MAP[i] |= n_position;
            ROOK_MOVE_MAP[i][0][up] = n_position;
        }
        for (size_t down = 0; down < 8; down++) {
            uint64_t n_position = position >> (8 * (down + 1));
            ROOK_RAY_MAP[i] |= n_position;
            ROOK_MOVE_MAP[i][1][down] = n_position;
        }
        for (size_t left = 0; left < 8; left++) {
            uint64_t n_position = (position << (1 * (left + 1))) & row_mask;
            ROOK_RAY_MAP[i] |= n_position;
            ROOK_MOVE_MAP[i][2][left] = n_position;
        }
        for (size_t right = 0; right < 8; right++) {
            uint64_t n_position = (position >> (1 * (right + 1))) & row_mask;
            ROOK_RAY_MAP[i] |= n_position;
            ROOK_MOVE_MAP[i][3][right] = n_position;
        }
    }

    // init bishop map and ray map
    for (int i = 0; i < 64; i++) {
        BISHOP_RAY_MAP[i] = 0;
        int row = i / 8;
        int col = i % 8;

        // up-left (northwest)
        int step = 0;
        for (int r = row - 1, c = col - 1; r >= 0 && c >= 0; r--, c--) {
            uint64_t square = 1ULL << (r * 8 + c);
            BISHOP_MOVE_MAP[i][0][step++] = square;
            BISHOP_RAY_MAP[i] |= square;
        }
        while (step < 8) BISHOP_MOVE_MAP[i][0][step++] = 0;

        // up-right (northeast)
        step = 0;
        for (int r = row - 1, c = col + 1; r >= 0 && c < 8; r--, c++) {
            uint64_t square = 1ULL << (r * 8 + c);
            BISHOP_MOVE_MAP[i][1][step++] = square;
            BISHOP_RAY_MAP[i] |= square;
        }
        while (step < 8) BISHOP_MOVE_MAP[i][1][step++] = 0;

        // down-left (southwest)
        step = 0;
        for (int r = row + 1, c = col - 1; r < 8 && c >= 0; r++, c--) {
            uint64_t square = 1ULL << (r * 8 + c);
            BISHOP_MOVE_MAP[i][2][step++] = square;
            BISHOP_RAY_MAP[i] |= square;
        }
        while (step < 8) BISHOP_MOVE_MAP[i][2][step++] = 0;

        // down-right (southeast)
        step = 0;
        for (int r = row + 1, c = col + 1; r < 8 && c < 8; r++, c++) {
            uint64_t square = 1ULL << (r * 8 + c);
            BISHOP_MOVE_MAP[i][3][step++] = square;
            BISHOP_RAY_MAP[i] |= square;
        }
        while (step < 8) BISHOP_MOVE_MAP[i][3][step++] = 0;
    }

    //init knight map
    uint64_t position_map = 1;
    for (size_t i = 0; i < 64; i++) {

        uint64_t position = 1ULL << i;
        char file = bitmapToPosition(position).file;
        for (size_t move = 0; move < 8; move++) KNIGHT_MOVE_MAP[i][move] = 0;
        
        if (file > 'a') {
            KNIGHT_MOVE_MAP[i][0] = position >> 15;
            KNIGHT_MOVE_MAP[i][1] = position << 17;
        }

        if (file > 'b') {
            KNIGHT_MOVE_MAP[i][2] = position << 10;
            KNIGHT_MOVE_MAP[i][3] = position >> 6;
        }

        if (file < 'g') {
            KNIGHT_MOVE_MAP[i][4] = position << 6;
            KNIGHT_MOVE_MAP[i][5] = position >> 10;

        }

        if (file < 'h') {
            KNIGHT_MOVE_MAP[i][6] = position << 15;
            KNIGHT_MOVE_MAP[i][7] = position >> 17;
        }
    }


    initRays();
}
