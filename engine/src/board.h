
#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdbool.h>


typedef enum {
    PAWN, KING, QUEEN, BISHOP, KNIGHT, ROOK,
    PIECE_TYPE_COUNT
} PieceType;

typedef enum { WHITE = 0, BLACK = 1, SIDE_COUNT} SIDE;

typedef struct {
    char file;
    char row;
} Position;

typedef enum {
    NONE = 0,
    BLACK_QUEENSIDE = 1 << 0,
    WHITE_QUEENSIDE = 1 << 2,
    BLACK_KINGSIDE = 1 << 3,
    WHITE_KINGSIDE = 1 << 4,
} CanCastle;

typedef struct {
    SIDE turn;
    //index 0: side (BLACK, WHITE), index 1: piece type -> value: bitboard of all places piece type is
    uint64_t pieces[SIDE_COUNT][PIECE_TYPE_COUNT];
    //index 0: side (BLACK, WHITE), index 1: board location -> value: bitboard of all places piece attacks
    uint64_t attacked_by[SIDE_COUNT][64];
    //index 0: side (BLACK, WHITE) -> all occupied positions by side
    //used for quick lookup (is a piece in my way??)
    uint64_t positions[SIDE_COUNT];
    CanCastle can_castle;
    uint64_t pinned[SIDE_COUNT];
    uint64_t check_mask[SIDE_COUNT];
    uint64_t valid_enpassant;
    uint64_t last_move;
    uint64_t hash;
} BoardState;

extern const uint64_t AFILE;
extern const uint64_t BFILE;
extern const uint64_t HFILE;
extern const uint64_t GFILE;

extern const uint64_t QUEENSIDE_BLACK_CASTLE_MASK;
extern const uint64_t QUEENSIDE_WHITE_CASTLE_MASK;
extern const uint64_t KINGSIDE_BLACK_CASTLE_MASK;
extern const uint64_t KINGSIDE_WHITE_CASTLE_MASK;

extern const BoardState STARTING_BOARD_STATE;
extern Position bitmapToPosition(uint64_t position);
extern void printBoard(BoardState* board_state);
extern char* boardStateToArray(BoardState* board_state);
extern char* moveBitmapToString(uint64_t position);
extern uint64_t stringPositionToBitmap(const char* str);

extern void updateAttacks(BoardState* board_state);
extern void updatePositions(BoardState* board_state);

extern PieceType getPieceType(uint64_t piece_mask, const BoardState* const boardState);

#endif