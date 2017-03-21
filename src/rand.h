#ifndef MNW_RAND_H_
#define MNW_RAND_H_

/* Author: Phil Mansfield (mansfield@uchicago.edu) (kind of: see below) 
 *
 * rand.h contains the random number generator for Minnow. It is based around
 * splitmix64 -- written by Sebastiano Vigna (vigna@acm.org) -- and xorshift128+
 * -- written by David Blackman and Sebastiano Vigna. The only parts which I
 * wrote were wrappers which make these things sane to use for multi-threaded
 * environments. */

#include <stdint.h>
#include <stdbool.h>

typedef uint64_t rand_State[2];

/* rand_Seed returns an array of n RNG states corresponding to the given
 * seed. */
rand_State *rand_Seed(uint64_t seed, int32_t n);

/* rand_Uint64 returns a random integer and updates the given RNG state. */
uint64_t rand_Uint64(rand_State *state);
float rand_Float(rand_State *state);
bool rand_Bool(rand_State *state);

/* rand_Uint64Range return a random (63 bit!) integer in the range [0, lim), and
 * updates the given RNG state. This function Panics if low >= high. */
uint64_t rand_Uint63Lim(rand_State *state, uint64_t lim);

#endif /* MNW_RAND_H_ */
