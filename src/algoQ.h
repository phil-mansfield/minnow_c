#ifndef ALGO_QUANTIZE_H_
#define ALGO_QUANTIZE_H_

#include "seq.h"
#include "algo.h"
#include <stdint.h>

typedef struct {
    algo_ParticleAccuracy Accuracy;
    uint64_t ParticleCbrt;
    bool UniformGridIDs;
} algoQ_Info;

typedef struct {
    U32Seq Quantized;
    FSeq Log;
} algoQ_CompressState;

typedef struct {
    U32Seq Quantized;
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
