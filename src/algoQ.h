#ifndef ALGO_QUANTIZE_H_
#define ALGO_QUANTIZE_H_

#include "seq.h"
#include "algo.h"
#include <stdint.h>

typedef struct {
    algo_Accuracy Acc;
    uint64_t ParticleCbrt;
} algoQ_Info;

typedef struct {
    U32Seq QuantizedVals;
} algoQ_CompressState;

typedef struct {
    U32Seq QuantizedVals;
} algoQ_DecompressState;

algo_CompressedParticles algoQ_Compress(
    algo_Particles p, algoQ_Info info,
    algoQ_CompressState state, algo_CompressedParticles buf
);

algo_Particles algoQ_Decompress(
    algo_CompressedParticles p, algoQ_DecompressState state,
    algo_Particles buf
);

#endif /* ALGO_A0_H_ */
