#ifndef MAPS_H
#define MAPS_H
#include <stdio.h>

extern uint64_t ROOK_MOVE_MAP[64][4][8];
extern uint64_t BISHOP_MOVE_MAP[64][4][8];
extern uint64_t KNIGHT_MOVE_MAP[64][8];
extern uint64_t ROOK_RAY_MAP[64];
extern uint64_t BISHOP_RAY_MAP[64];
extern uint64_t RAYS[64][64];

extern void initMoveMaps();


#endif