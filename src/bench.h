#ifndef MNW_BENCHMARK_H_
#define MNW_BENCHMARK_H_

#include <time.h>

#define TARGET_TRIAL_LENGTH 0.5
#define WALL_CLOCK_LIMIT 30.0

typedef struct Benchmark {
    uint64_t N;
    clock_t tSum;
    clock_t t0, t1;
} Benchmark;

void Benchmark_Start(Benchmark *b);
void Benchmark_End(Benchmark *b);
void Benchmark_Pause(Benchmark *b);
void Benchmark_Resume(Benchmark *b);
double Benchmark_Duration(Benchmark *b);
void Benchmark_Run(char *name, uint64_t (*trial) (Benchmark*), uint64_t bytes);


#endif /* MNW_BENCHMARK_H_ */
