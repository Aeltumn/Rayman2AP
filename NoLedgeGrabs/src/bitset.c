#include "bitset.h"

/** Clears a bitset. */
void clearBitSet(BitSet* set) {
	memset(set->contents, 0, sizeof(set->contents));
}

/** Returns the value of the bitset at the given index. */
unsigned char getBitSet(BitSet* set, int index) {
    if (index <= 0 || index > BOOL_ARRAY_SIZE) return false;
    int word = (index - 1) / BITS_PER_WORD;
    int bit = (index - 1) % BITS_PER_WORD;
    return (set->contents[word] >> bit) & 1U;
}

/** Writes the given value to the bitset at the given index. */
void setBitSet(BitSet* set, int index, unsigned char value) {
    if (index <= 0 || index > BOOL_ARRAY_SIZE) return;
    int word = (index - 1) / BITS_PER_WORD;
    int bit = (index - 1) % BITS_PER_WORD;
    if (value) {
        set->contents[word] |= (1U << bit);
    } else {
        set->contents[word] &= ~(1U << bit);
    }
}