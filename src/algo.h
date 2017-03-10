#ifndef ALGO_H_
#define ALGO_H_

#include <stdbool.h>
#include <stdint.h>
#include "seq.h"

/* algo_Snapshot contains information on */
typedef struct algo_Particles {
    FSeq Position[3];
    FSeq Velocity[3];
    U64Seq ID64;
    U32Seq ID32;
    FSeqSeq Variables;
} algo_Particles;

algo_Particles algo_EmptyParticles;

typedef struct algo_CompressedParticles {
    U8SeqSeq Blocks;
    int32_t Num, VariableNum;
    bool HasPosition, HasVeclocity, HasID64, HasID32;
} algo_CompressedParticles;

algo_CompressedParticles EmptyCompressedParticles;

typedef struct algo_Accuracy {
    float Accuracy;
    FSeq Accuracies;
} algo_Info;

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
