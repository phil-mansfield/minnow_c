#include <stdlib.h>
#include <inttypes.h>
#include "rand.h"
#include "debug.h"

/* Warning to maintainers: This is the only file Minnow which is not
 * auto-tested. Tests are done by inspecting output. If you break something
 * here, you won't find out just by running "make test". */

/***********************/
/* Forward definitions */
/***********************/

uint64_t xorshiftNext(rand_State *state);
void xorshiftJump(rand_State *state);
uint64_t splitmixNext(uint64_t *state);

/**********************/
/* Exported Functions */
/**********************/

rand_State *rand_Seed(uint64_t seed, int32_t n) {
    DebugAssert(n > 0) {
        Panic("rand_Seed given non-positive length, %"PRId32".", n);
    }

    rand_State *state = calloc((size_t)n, sizeof(*state));
    AssertAlloc(state);
    state[0][0]= splitmixNext(&seed);
    state[0][1]= splitmixNext(&seed);

    for (int32_t i = 1; i < n; i++) {
        state[i][0] = state[i-1][0];
        state[i][1] = state[i-1][1];
        xorshiftJump(state + i);
    }

    return state;
}

uint64_t rand_Uint64(rand_State *state) {
    return xorshiftNext(state);
}

uint64_t rand_Uint63Lim(rand_State *state, uint64_t lim) {
    /* This is based off of Go's Int63n function. I have no idea what I'm
     * doing. */

    uint64_t high = (uint64_t) 1 << 63;
    uint64_t mask = ~high;
    uint64_t max = high - 1 - high%lim;
    uint64_t v = rand_Uint64(state) & mask;
    while ((v & mask) > max) {
        v = rand_Uint64(state) & mask;
    }

    return v % lim;
}

float rand_Float(rand_State *state) {
    uint64_t x = rand_Uint64(state);
    const uint64_t mask = ~((~0) << 24);
    return (float) (x & mask) / (float) (mask + 1) ;
}

bool rand_Bool(rand_State *state) {
    return (rand_Uint64(state) & 2) != 0;
}

/********************/
/* Helper Functions */
/********************/

/* I did not write these. See rand.h for attriutions. */

static inline uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

uint64_t xorshiftNext(rand_State *state) {    
    const uint64_t s0 = (*state)[0];
    uint64_t s1 = (*state)[1];
    const uint64_t result = s0 + s1;

    s1 ^= s0;
    (*state)[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
    (*state)[1] = rotl(s1, 36); // c

    return result;
}


/* This is the jump function for the generator. It is equivalent
   to 2^64 calls to next(); it can be used to generate 2^64
   non-overlapping subsequences for parallel computations. */
void xorshiftJump(rand_State *state) {
    static const uint64_t JUMP[] = { 0xbeac0467eba5facb, 0xd86b048b86aa9922 };

    uint64_t s0 = 0;
    uint64_t s1 = 0;
    for(unsigned int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
        for(unsigned int b = 0; b < 64; b++) {
            if (JUMP[i] & 1ULL << b) {
                s0 ^= (*state)[0];
                s1 ^= (*state)[1];
            }
            xorshiftNext(state);
        }

    (*state)[0] = s0;
    (*state)[1] = s1;
}

uint64_t splitmixNext(uint64_t *state) {
    uint64_t x = *state;
    uint64_t z = (x += UINT64_C(0x9E3779B97F4A7C15));
    *state = x;
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
}
