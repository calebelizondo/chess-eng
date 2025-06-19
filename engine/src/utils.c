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