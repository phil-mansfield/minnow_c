#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include "bench.h"

const double TARGET_TRIAL_LENGTH = 0.5;
const double WALL_CLOCK_LIMIT = 30.0;

void Benchmark_Start(Benchmark *b) {
    b->t0 = clock();
}

void Benchmark_End(Benchmark *b) {
    b->t1 = clock();
}

void Benchmark_Pause(Benchmark *b) {
    Benchmark_End(b);
    b->tSum += b->t1 - b->t0;
}

void Benchmark_Resume(Benchmark *b) {
    Benchmark_Start(b);
}

double Benchmark_Duration(Benchmark *b) {
    return ((double) (b->t1 - b->t0 + b->tSum) ) / CLOCKS_PER_SEC;
}

void Benchmark_Run(char *name, uint64_t (*trial) (Benchmark*), uint64_t bytes) {
    double trialLength = 0.0;
    uint64_t totalN = 0;
    Benchmark b = {0, 0, 0, 0};

    printf("Running %s: ", name);

    clock_t start = clock();

    for (uint64_t i = 0; i < 15 && trialLength < TARGET_TRIAL_LENGTH; i++) {
        clock_t ti = clock();
        if ((ti - start) / (double) CLOCKS_PER_SEC > WALL_CLOCK_LIMIT) {
            break;
        }

        uint64_t n = 1 << (2*i);
        b.N = n;
        (*trial)(&b);

        totalN += n;
        trialLength += Benchmark_Duration(&b);
    }

    double bytesPerSec = (bytes*totalN) / trialLength;

    if (bytesPerSec < 1e3) {
        printf("    %.4g B/s\n", bytesPerSec);
    } else if (bytesPerSec < 1e6) {
        printf("    %.4g kB/s\n", bytesPerSec / 1e3);
    } else if (bytesPerSec < 1e9) {
        printf("    %.4g MB/s\n", bytesPerSec / (1e6));
    } else {
        printf("    %.4g GB/s\n", bytesPerSec / (1e9));
    }
}
