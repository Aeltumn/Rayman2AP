#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define BOOL_ARRAY_SIZE 1400
#define BITS_PER_WORD 32
#define WORD_COUNT ((BOOL_ARRAY_SIZE + BITS_PER_WORD - 1) / BITS_PER_WORD)

typedef struct {
    uint32_t contents[WORD_COUNT];
} BitSet;

void clearBitSet(BitSet* set);
unsigned char getBitSet(BitSet* set, int index);
void setBitSet(BitSet* set, int index, unsigned char value);