#import "utils.h"
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>


void printBinary(uint64_t num) {
    for (int i = 63; i >= 0; i--) {
        if ((num >> i) & 1ULL) {
            printf("1");
        } else {
            printf("0");
        }
        if (i % 8 == 0 && i != 0) {
            printf(" ");
        }
    }
    printf("\n");
}

uint64_t extract_nth_set_bit(uint64_t bitboard, int n) {
    for (int i = 0; i < 64; i++) {
        if ((bitboard >> i) & 1) {
            if (n == 0) return 1ULL << i;
            n--;
        }
    }
    return 0;
}
