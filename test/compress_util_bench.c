#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "bench.h"
#include "util.h"
#include "rand.h"
#include "seq.h"

void FShuffle(FSeq x);
void U32Shuffle(U32Seq x, uint32_t lim);

uint64_t MinMaxTrial_100MB(Benchmark *b) {
    FSeq x = FSeq_New((int32_t) 25e6);
    FShuffle(x);

    Benchmark_Start(b);

    for (uint64_t i = 0; i < b->N; i++) {
        float min, max;
        util_MinMax(x, &min, &max);
    }

    Benchmark_End(b);

    FSeq_Free(x);

    return 0;
}

uint64_t PeriodicTrial_InBounds_100MB (Benchmark *b) {
    FSeq x = FSeq_New((int32_t) 25e6);

    Benchmark_Start(b);

    for (uint64_t i = 0; i < b->N; i++) {
        Benchmark_Pause(b);
        FShuffle(x);
        Benchmark_Resume(b);

        util_Periodic(x, 2);
    }

    Benchmark_End(b);

    return 0;
}

uint64_t PeriodicTrial_OffCenter_100MB (Benchmark *b) {
    FSeq x = FSeq_New((int32_t) 25e6);

    Benchmark_Start(b);

    for (uint64_t i = 0; i < b->N; i++) {
        Benchmark_Pause(b);
        FShuffle(x);
        for (int32_t j = 0; j < x.Len; j++) { x.Data[j] += 1.5; }
        Benchmark_Resume(b);

        util_Periodic(x, 2);
    }

    Benchmark_End(b);

    return 0;
}

uint64_t UndoPeriodicTrial_InBounds_100MB (Benchmark *b) {
    FSeq x = FSeq_New((int32_t) 25e6);

    Benchmark_Start(b);

    for (uint64_t i = 0; i < b->N; i++) {
        Benchmark_Pause(b);
        FShuffle(x);
        util_Periodic(x, 2);
        Benchmark_Resume(b);

        util_UndoPeriodic(x, 2);
    }

    Benchmark_End(b);

    return 0;
}

uint64_t UndoPeriodicTrial_OffCenter_100MB (Benchmark *b) {
    FSeq x = FSeq_New((int32_t) 25e6);

    Benchmark_Start(b);

    for (uint64_t i = 0; i < b->N; i++) {
        Benchmark_Pause(b);
        FShuffle(x);
        util_Periodic(x, 2);
        for (int32_t j = 0; j < x.Len; j++) { x.Data[j] += 1.5; }
        Benchmark_Resume(b);        

        util_UndoPeriodic(x, 2);
    }

    Benchmark_End(b);

    return 0;
}

uint64_t UniformBinIndexTrial_100MB(Benchmark *b) {
    FSeq x = FSeq_New((int32_t) 25e6);
    U32Seq buf = U32Seq_New((int32_t) 25e6);
    float x0 = 0;
    float dx = 1;
    uint8_t level = 14;

    FShuffle(x);

    Benchmark_Start(b);

    for (uint64_t i = 0; i < b->N; i++) {
        buf = util_UniformBinIndex(x, level, x0, dx, buf);
    }

    Benchmark_End(b);

    return 0;
}

uint64_t UndoUniformBinIndexTrial_100MB(Benchmark *b) {
    U32Seq x = U32Seq_New((int32_t) 25e6);
    FSeq buf = FSeq_New((int32_t) 25e6);
    float x0 = 0;
    float dx = 1;
    uint8_t level = 14;
    uint32_t lim = 1<<14;

    U32Shuffle(x, lim);

    rand_State *state = rand_Seed(0, 1);
    Benchmark_Start(b);

    for (uint64_t i = 0; i < b->N; i++) {
        buf = util_UndoUniformBinIndex(x, level, x0, dx, state, buf);
    }

    Benchmark_End(b);
    free(state);

    return 0;
}

uint64_t UniformPackTrial_Aligned_100MB(Benchmark *b) {
    U32Seq x = U32Seq_New((int32_t) 25e6);
    U32Seq buf = U32Seq_New((int32_t) 25e6);

    uint8_t width = 8;
    uint32_t lim = 1<<width;
    U32Shuffle(x, lim);

    Benchmark_Start(b);

    for (uint64_t i = 0; i < b->N; i++) {
        buf = util_U32UniformPack(x, width, buf);
    }

    Benchmark_End(b);

    return 0;
}

uint64_t UniformPackTrial_Unaligned_100MB(Benchmark *b) {
    U32Seq x = U32Seq_New((int32_t) 25e6);
    U32Seq buf = U32Seq_New((int32_t) 25e6);

    uint8_t width = 9;
    uint32_t lim = 1<<width;
    U32Shuffle(x, lim);

    Benchmark_Start(b);

    for (uint64_t i = 0; i < b->N; i++) {
        buf = util_U32UniformPack(x, width, buf);
    }

    Benchmark_End(b);

    return 0;
}

uint64_t UndoUniformPackTrial_Aligned_100MB(Benchmark *b) {
    U32Seq x = U32Seq_New((int32_t) 25e6);
    U32Seq buf = U32Seq_New((int32_t) 25e6);

    uint8_t width = 8;
    uint32_t lim = 1<<width;
    U32Shuffle(x, lim);

    Benchmark_Start(b);

    for (uint64_t i = 0; i < b->N; i++) {
        Benchmark_Pause(b);
        buf = util_U32UniformPack(x, width, buf);
        Benchmark_Resume(b);

        x = util_U32UndoUniformPack(buf, width, (int32_t) 25e6, x);
    }

    Benchmark_End(b);

    return 0;
}

uint64_t UndoUniformPackTrial_Unaligned_100MB(Benchmark *b) {
    U32Seq x = U32Seq_New((int32_t) 25e6);
    U32Seq buf = U32Seq_New((int32_t) 25e6);

    uint8_t width = 9;
    uint32_t lim = 1<<width;
    U32Shuffle(x, lim);

    Benchmark_Start(b);

    for (uint64_t i = 0; i < b->N; i++) {
        Benchmark_Pause(b);
        buf = util_U32UniformPack(x, width, buf);
        Benchmark_Resume(b);

        x = util_U32UndoUniformPack(buf, width, (int32_t) 25e6, x);
    }

    Benchmark_End(b);

    return 0;
}

uint64_t FastCompressTrial_100MB(Benchmark *b) {
    FSeq x = FSeq_New((int32_t) 25e6);
    U32Seq buf = U32Seq_New(x.Len);
    U32Seq out = U32Seq_New(x.Len);
    
    uint8_t level = 11;

    Benchmark_Start(b);

    for (uint32_t i = 0; i < b->N; i++){
        Benchmark_Pause(b);

        FShuffle(x);
        for (int32_t j = 0; j < x.Len; j++)
            x.Data[j] += 1.5;

        Benchmark_Resume(b);

        util_UndoPeriodic(x, 2);
        float min, max;
        util_MinMax(x, &min, &max);
        buf = util_UniformBinIndex(x, level, min, max - min, buf);
        out = util_U32UniformPack(buf, level, out);
    }

    Benchmark_End(b);

    return 0;
}

uint64_t UndoFastCompressTrial_100MB(Benchmark *b) {
    FSeq x = FSeq_New((int32_t) 25e6);
    U32Seq buf = U32Seq_New(x.Len);
    U32Seq out = U32Seq_New(x.Len);
    
    uint8_t level = 11;
    rand_State *state = rand_Seed(0, 1);

    Benchmark_Start(b);

    for (uint32_t i = 0; i < b->N; i++){
        Benchmark_Pause(b);

        FShuffle(x);
        for (int32_t j = 0; j < x.Len; j++)
            x.Data[j] += 1.5;

        util_UndoPeriodic(x, 2);
        float min, max;
        util_MinMax(x, &min, &max);
        buf = util_UniformBinIndex(x, level, min, max - min, buf);
        out = util_U32UniformPack(buf, level, out);

        Benchmark_Resume(b);

        buf = util_U32UndoUniformPack(out, level, x.Len, buf);
        x = util_UndoUniformBinIndex(buf, level, min, max - min, state, x);
        util_Periodic(x, 2);
    }

    Benchmark_End(b);
    free(state);

    return 0;
}

void FShuffle(FSeq x) {
    rand_State *s =rand_Seed(0, 1);
    for (int32_t i = 0; i < x.Len; i++) {
        x.Data[i] = rand_Float(s);
    }
    free(s);
}

void U32Shuffle(U32Seq x, uint32_t lim) {
    rand_State *s =rand_Seed(0, 1);
    for (int32_t i = 0; i < x.Len; i++) {
        x.Data[i] = (uint32_t) rand_Uint63Lim(s, (uint64_t) lim);
    }
    free(s);
}

int main() {
    
    Benchmark_Run("util_MinMax, 100 MB", &MinMaxTrial_100MB, (uint64_t) 100e6);
    /*
    Benchmark_Run("util_Periodic (in bounds), 100 MB",
                  &PeriodicTrial_InBounds_100MB, (uint64_t) 1e8);
    Benchmark_Run("util_Periodic (off center), 100 MB",
                  &PeriodicTrial_OffCenter_100MB, (uint64_t) 1e8);
    Benchmark_Run("util_UndoPeriodic (in bounds), 100 MB",
                  &UndoPeriodicTrial_InBounds_100MB, (uint64_t) 1e8);
    Benchmark_Run("util_UndoPeriodic (off center), 100 MB",
                  &UndoPeriodicTrial_OffCenter_100MB, (uint64_t) 1e8);
    Benchmark_Run("util_UniformBinIndex, 100 MB",
                  &UniformBinIndexTrial_100MB, (uint64_t) 1e8);
    Benchmark_Run("util_UndoUniformBinIndex, 100 MB",
                  &UndoUniformBinIndexTrial_100MB, (uint64_t) 1e8);
    Benchmark_Run("util_UniformPack (aligned), 100 MB",
                  &UniformPackTrial_Aligned_100MB, (uint64_t) 1e8);
    Benchmark_Run("util_UniformPack (unaligned), 100 MB",
                  &UniformPackTrial_Unaligned_100MB, (uint64_t) 1e8);
    Benchmark_Run("util_UndoUniformPack (aligned), 100 MB",
                  &UndoUniformPackTrial_Aligned_100MB, (uint64_t) 1e8);
    Benchmark_Run("util_UndoUniformPack (unaligned), 100 MB",
                  &UndoUniformPackTrial_Unaligned_100MB, (uint64_t) 1e8);
    */

    Benchmark_Run("(mock) fast compress, 100 MB",
                  &FastCompressTrial_100MB, (uint64_t) 1e8);
    Benchmark_Run("(mock) undo fast compress, 100 MB",
                  &FastCompressTrial_100MB, (uint64_t) 1e8);
}
