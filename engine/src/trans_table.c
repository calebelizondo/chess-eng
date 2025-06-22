#include "board.h"

//idk some decently large hash???
#define TABLE_SIZE 4073


//in board.h:
// struct PiecePositions {
//     int64_t king;
//     int64_t queens;
//     int64_t rooks;
//     int64_t knights;
//     int64_t bishops;
//     int64_t pawns;
// };
// typedef struct {
//     TURN turn;
//     struct PiecePositions black;
//     struct PiecePositions white;
//     uint64_t black_positions;
//     uint64_t white_positions;
//     bool black_can_castle;
//     bool white_can_castle;
//     uint64_t last_move;
// } BoardState;

// typedef struct {
//     uint64_t key;
//     uint64_t best_rs
// } EngineMove;

// const EngineMove* table = malloc(sizeof(EngineMove) * TABLE_SIZE);

// EngineMove* lookup(BoardState* boardState) {
//     uint64_t hash = hash(boardState);

// }


// uint64_t hash(BoardState* boardState) {

// }