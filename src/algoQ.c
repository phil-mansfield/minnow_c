#include "algoQ.h"
#include "algo.h"

/************************/
/* Forward Declarations */
/************************/

int32_t CountBlocks(algo_Particles p);
U8Seq QuantizeFloat(
    FSeq x, algo_Accuracy acc, algoQ_CompressState state, U8Seq buf
);
U8Seq QuantizeU64(
    FSeq x, algoQ_CompressState state, U8Seq buf
);
U8Seq QuantizeU64ID(
    U64Seq id, int dim, algoQ_Info info, algoQ_CompressState state, U8Seq buf
);
U8Seq QuantizeU32ID(
    U32Seq id, int dim, algoQ_Info info, algoQ_CompressState state, U8Seq buf
);

/**********************/
/* Exported Functions */
/**********************/

algo_CompressedParticles algoQ_Compress(
    algo_Particles p, algoQ_Info info,
    algoQ_CompressState state, algo_CompressedParticles buf
) {
    algo_CheckParticles(p);

    int32_t numBlocks = CountBlocks(p);
    buf.Blocks = U8SeqSeq_Extend(buf.Blocks, numBlocks);
    buf.Blocks = U8SeqSeq_Sub(buf.Blocks, 0, numBlocks);

    /* Loop through all the fields, and compress them if they exist. */
    int32_t blockIdx = 0;

    /* Positions */
    if (p.Position[0].Len > 0) {
        for (int i = 0; i < 3; i++) {
            buf.Blocks.Data[blockIdx] = QuantizeFloat(
                p.Position[i], info.Accuracy.Position,
                state, buf.Blocks.Data[blockIdx]
            );
            blockIdx++;
        }
    }

    /* Velocities */
    if (p.Velocity[0].Len > 0) {
        for (int i = 0; i < 3; i++) {
            buf.Blocks.Data[blockIdx] = QuantizeFloat(
                p.Velocity[i], info.Accuracy.Velocity,
                state, buf.Blocks.Data[blockIdx]
            );
            blockIdx++;
        }
    }

    /* IDs */
    if (p.ID64.Len > 0) {
        for (int dim = 0; dim < 3; dim++) {
            buf.Blocks.Data[blockIdx] = QuantizeU64ID(
                p.ID64, dim, info, state, buf.Blocks.Data[blockIdx]
            );
            blockIdx++;
        }
    }

    if (p.ID32.Len > 0) {
        for (int dim = 0; dim < 3; dim++) {
            buf.Blocks.Data[blockIdx] = QuantizeU32ID(
                p.ID32, dim, info, state, buf.Blocks.Data[blockIdx]
            );
            blockIdx++;
        }
    }

    /* Misc variables */
    for (int32_t i = 0; i < p.FVars.Len; i++) {
        /* No conditioning over non-zero length variables here. */
        buf.Blocks.Data[blockIdx] = QuantizeFloat(
            p.FVars.Data[i], info.Accuracy.FVars[i],
            state, buf.Blocks.Data[blockIdx]
        );
        blockIdx++;
    }

    for (int32_t i = 0; i < p.U64Vars.Len; i++) {
        /* No conditioning over non-zero length variables here. */
        buf.Blocks.Data[blockIdx] = QuantizeU64(
            p.FVars.Data[i], state, buf.Blocks.Data[blockIdx]
        );
        blockIdx++;
    }

    return buf;
}

algo_Particles algoQ_Decompress(
    algo_CompressedParticles p, algoQ_DecompressState state,
    algo_Particles buf
) {
    (void) p;
    (void) state;

    return buf;
}

/********************/
/* Helper Functions */
/********************/

/* CountBlocks counts the number of blocks that will need to be allocated to
 * represent the variable fields in a given algo_Particles. */
int32_t CountBlocks(algo_Particles p) {
    int32_t n = 0;

    if (p.Position[0].Len > 0) { n += 3; }
    if (p.Velocity[0].Len > 0) { n += 3; }
    if (p.ID64.Len > 0) { n++; }
    if (p.ID32.Len > 0) { n++; }
    for (int32_t i = 0; i < p.FVars.Len; i++) {
        if (p.FVars.Data[i].Len > 0) { n++; }
    }
    for (int32_t i = 0; i < p.U64Vars.Len; i++) {
        if (p.U64Vars.Data[i].Len > 0) { n++; }
    }

    return n;
}

/* QuantizeFloat quantizes a float sequence up to the target accuracy. The
 * state and buf parameters are there to prevent unneeded heap allocations. */
U8Seq QuantizeFloat(
    FSeq x, algo_Accuracy info, algoQ_CompressState state, U8Seq buf
) {
    U32Seq u32Buf = U32Seq_Extend(state.Quantized, x.Len);
    u32Buf = U32Seq_Sub(u32Buf, 0, x.Len);

    (void) info;

    /* Garaunteed not to be called on an empty seqeunce. */
    float min = x.Data[0];
    float max = x.Data[0];

    for (int32_t i = 1; i < x.Len; i++) {
        if (x.Data[i] > max) {
            max = x.Data[i];
        } else if (x.Data[i] < min) {
            min = x.Data[i];
        }
    }



    /* TODO: Generalize to non-uniform accuracies. */

    return buf;
}

U8Seq QuantizeU64ID(
    U64Seq id, int dim, algoQ_Info info, algoQ_CompressState state, U8Seq buf
) {
    (void) id;
    (void) dim;
    (void) info;
    (void) state;
    return buf;
}

U8Seq QuantizeU32ID(
    U32Seq id, int dim, algoQ_Info info,  algoQ_CompressState state, U8Seq buf
) {
    (void) id;
    (void) dim;
    (void) info;
    (void) state;
    return buf;
}
