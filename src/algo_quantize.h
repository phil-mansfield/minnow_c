#ifndef ALGO_QUANTIZE_H_
#define ALGO_QUANTIZE_H_

#include "seq.h"
#include "algo.h"

algo_CompressedParticles algoQ_Compress(
    algo_Particles p, algo_CompressedParticles buf
);

algo_Snapshot algoQ_Decompress(
    algo_CompressedParticles p, algo_Particles buf
);

#endif /* ALGO_A0_H_ */
