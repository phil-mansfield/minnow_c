#ifndef ALGO_H_
#define ALGO_H_

#include <stdbool.h>
#include <stdint.h>
#include "seq.h"

/* algo_Particles contains information on */
typedef struct algo_Particles {
    FSeq Position[3];
    FSeq Velocity[3];
    U64Seq ID64;
    U32Seq ID32;
    FSeqSeq FVars;
    U64SeqSeq U64Vars;
} algo_Particles;

algo_Particles algo_EmptyParticles;

typedef struct algo_CompressedParticles {
    U8SeqSeq Blocks;
    int32_t ParticleNum, FVarsNum, U64VarsNum;
    bool HasPosition, HasVeclocity, HasID64, HasID32;
} algo_CompressedParticles;

algo_CompressedParticles EmptyCompressedParticles;

typedef struct algo_Accuracy {
    float Accuracy;
    FSeq Accuracies;
    float Bound;
    bool LogScale;
} algo_Accuracy;

typedef struct algo_ParticleAccuracy {
    algo_Accuracy Position;
    algo_Accuracy Velocity;
    algo_Accuracy *FVars;
    
} algo_ParticleAccuracy;

/*********************/
/* Utility Functions */
/*********************/

/* algo_CheckSnapshot checks that every field in an algo_Snapshot is either
 * empty or the same length. Otherwise, it Panics. */
void algo_CheckParticles(algo_Particles p);

/* algo_ExtendBufferSnapshot expands the seqeunces in buf to be consistent with
 * the values contained in ref. The extended buffer is returned. None of the
 * sequences in the initial buffer can be assumed to be valid. */
algo_Particles algo_ExtendBufferParticles(
    algo_CompressedParticles ref, algo_Particles buf
);

#endif /* ALGO_H_ */
