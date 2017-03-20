#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "bench.h"
#include "compress_util.h"
#include "rand.h"
#include "seq.h"

void Shuffle(FSeq x);

uint64_t MinMaxTrial_100MB(Benchmark *b) {
    FSeq x = FSeq_New((int32_t) 25e6);
	Shuffle(x);
    Benchmark_Start(b);

	float min, max;
	
    for (uint64_t i = 0; i < b->N; i++) {
        util_MinMax(x, &min, &max);
    }

    Benchmark_End(b);

    FSeq_Free(x);

    return 0;
}

void Shuffle(FSeq x) {
    rand_State *s =rand_Seed(0, 1);
    for (int32_t i = 0; i < x.Len; i++) {
        x.Data[i] = rand_Float(s);
    }
    free(s);
}

int main() {
    Benchmark_Run("util_MinMax, 100 MB", &MinMaxTrial_100MB, (uint64_t) 1e8);
}
