#include "algo.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include "debug.h"
#include <float.h>
#include <string.h>
#include "math.h"

    
/************************/
/* Forward Declarations */
/************************/

/* Functions that check for invalid values. */
void FCheckValues(FSeq x, algo_Accuracy acc, float bound, char *seqName);
void U64CheckValues(U64Seq x, uint64_t bound, char *seqName);
void U32CheckValues(
    U32Seq x, uint32_t bound, algo_QuantizedRange range, char *seqName
);
void U32CheckVectorValues(
    U32Seq x, uint32_t bound, algo_QuantizedVectorRange range, char *seqName
);

/* Functions that wrap around low level sequence functions to (prevent bugs). */
U8Seq U8SetLen(U8Seq s, int32_t len);
U32Seq U32SetLen(U32Seq s, int32_t len);
U64Seq U64SetLen(U64Seq s, int32_t len);
FSeq FSetLen(FSeq s, int32_t len);

/* Functions that convert algo_Accuracy structs to algo_*Range structs. */
algo_QuantizedRange AccuracyToRange(
    algo_Accuracy acc, float x0, float x1, algo_QuantizedRange buf
);

algo_QuantizedVectorRange AccuracyToVectorRange(
    algo_Accuracy acc, float x0[3], float x1[3], algo_QuantizedVectorRange buf
);

algo_QuantizedVectorRange AccuracyToXVectorRange(
    algo_Accuracy acc, float x0[3], float x1[3],
    float xWdith, algo_QuantizedVectorRange buf
);

/* Range calculation */
void FMinMax(FSeq x, float *minOut, float *maxOut);
void FBoundedMinMax(FSeq x, float dx, float *minOut, float *maxOut);

/* algo_Quantize subprocedures */
algo_QuantizedParticles SetQuantizedRanges(
    algo_Particles p, algo_QuantizedParticles buf
);

/**********************/
/* Exported Functions */
/**********************/

algo_QuantizedParticles algo_Quantize(
    algo_Particles p, algo_QuantizedParticles buf
) {
    buf = SetQuantizedRanges(p, buf);

    /* Quantize floating point variables. */

    /* Convert IDs into ID vectors. */

    /* Copy (not reference) uint64 variables into QuantizedParticles. */

    return buf;
}

algo_Particles algo_UndoQuantize(
    algo_QuantizedParticles p, algo_Particles buf
) {
    (void) p;
    return buf;
}

void Particles_Free(algo_Particles p) {
    for (int i = 0; i < 3; i++) {
	    FSeq_Free(p.X[i]);
        FSeq_Free(p.V[i]);
    }
    U64Seq_Free(p.ID);
	FSeq_Free(p.XAcc.Deltas);
	FSeq_Free(p.VAcc.Deltas);
	
    for (int32_t i = 0; i < p.FVars.Len; i++) {
        FSeq_Free(p.FVars.Data[i]);
    }
	for (int32_t i = 0; i < p.FVars.Len; i++) {
	    FSeq_Free(p.FVarsAcc[i].Deltas);
	}
    FSeqSeq_Free(p.FVars);
    free(p.FVarsAcc);

    for (int32_t i = 0; i < p.U64Vars.Len; i++) {
        U64Seq_Free(p.U64Vars.Data[i]);
    }
    U64SeqSeq_Free(p.U64Vars);
}

void QuantizedParticles_Free(algo_QuantizedParticles p) {
    for (int i = 0; i < 3; i++) {
        U32Seq_Free(p.X[i]);
        U32Seq_Free(p.V[i]);
        U32Seq_Free(p.ID[i]);
    }
    U8Seq_Free(p.XRange.Depths);
    U8Seq_Free(p.VRange.Depths);

    for (int32_t i = 0; i < p.FVars.Len; i++) {
        U32Seq_Free(p.FVars.Data[i]);
        U8Seq_Free(p.FVarsRange[i].Depths);
    }
    U32SeqSeq_Free(p.FVars);
    free(p.FVarsRange);

    for (int32_t i = 0; i < p.U64Vars.Len; i++) {
        U64Seq_Free(p.U64Vars.Data[i]);
    }
    U64SeqSeq_Free(p.U64Vars);
}

void CompressedParticles_Free(algo_CompressedParticles p) {
    for (int32_t i = 0; i < p.Blocks.Len; i++) {
        U8Seq_Free(p.Blocks.Data[i]);
    }
    U8SeqSeq_Free(p.Blocks);
}

void QuantizedParticles_Check(algo_QuantizedParticles p) {
    int32_t len = p.X[0].Len;
    if (len <= 0) {
        Panic("X sequences have been left empty.%s", "");
    }

    if (p.X[1].Len != len || p.X[2].Len != len) {
        Panic("The lengths of the three position sequences are (%"PRId32
              ", %"PRId32", %"PRId32"), but they all need to be the "
              "same length.", p.X[0].Len, p.X[1].Len,
              p.X[2].Len);
    }
    U32CheckVectorValues(p.X[0], 0, p.XRange, "p.X[0]");
    U32CheckVectorValues(p.X[1], 0, p.XRange, "p.X[1]");
    U32CheckVectorValues(p.X[2], 0, p.XRange, "p.X[2]");

    if (p.V[0].Len != p.V[1].Len ||  p.V[0].Len != p.V[2].Len) {
        Panic("The lengths of the three velocity sequences are (%"PRId32
              ", %"PRId32", %"PRId32"), but they all need to be the "
              "same length.", p.V[0].Len, p.V[1].Len,
              p.V[2].Len);
    }
    U32CheckVectorValues(p.V[0], 0, p.VRange, "p.V[0]");
    U32CheckVectorValues(p.V[1], 0, p.VRange, "p.V[1]");
    U32CheckVectorValues(p.V[2], 0, p.VRange, "p.V[2]");

    if (p.ID[0].Len != p.ID[1].Len ||  p.ID[0].Len != p.ID[2].Len) {
        Panic("The lengths of the three ID sequences are (%"PRId32
              ", %"PRId32", %"PRId32"), but they all need to be the "
              "same length.", p.ID[0].Len, p.ID[1].Len,
              p.ID[2].Len);
    }
    algo_QuantizedRange empty = {0, U8Seq_Empty(), 0, 0, false};
    U32CheckValues(p.ID[0], p.IDWidth, empty, "p.ID[0]");
    U32CheckValues(p.ID[1], p.IDWidth, empty, "p.ID[1]");
    U32CheckValues(p.ID[2], p.IDWidth, empty, "p.ID[2]");

    if (p.V[0].Len != len && p.V[0].Len != 0) {
        Panic("The length of the velocity sequence is %"PRId32", but the "
              "length of the position sequence is %"PRId32".", p.V[0].Len, len);
    }

    if (p.ID[0].Len != len && p.ID[0].Len != 0) {
        Panic("The length of the ID sequence is %"PRId32", but the length of "
              "the position sequence is %"PRId32".", p.ID[0].Len, len);
    }

    for (int32_t i = 0; i < p.FVars.Len; i++) {
        if (p.FVars.Data[i].Len != len &&
            p.FVars.Data[i].Len != 0) {
            Panic("The length of the FVars[%"PRId32"] sequence is %"PRId32
                  ", but the length of the position sequence is %"PRId32".",
                  i, p.FVars.Data[i].Len, len);
        }

        char name[20];
        sprintf(name, "p.FVars[%"PRId32"]", i);
        U32CheckValues(p.FVars.Data[i], 0, p.FVarsRange[i], name);
    }

    for (int32_t i = 0; i < p.U64Vars.Len; i++) {
        if (p.U64Vars.Data[i].Len != len &&
            p.U64Vars.Data[i].Len != 0) {
            Panic("The length of the U64Vars[%"PRId32"] sequence is %"PRId32
                  ", but the length of the position sequence is %"PRId32".",
                  i, p.U64Vars.Data[i].Len, len);
        }
    }
}

void Particles_Check(algo_Particles p) {
    int32_t len = p.X[0].Len;
    if (len <= 0) {
        Panic("X sequences have been left empty.%s", "");
    }

    if (p.X[1].Len != len || p.X[2].Len != len) {
        Panic("The lengths of the three position sequences are (%"PRId32
              ", %"PRId32", %"PRId32"), but they all need to be the "
              "same length.", p.X[0].Len, p.X[1].Len,
              p.X[2].Len);
    }
    FCheckValues(p.X[0], p.XAcc, p.XWidth, "p.X[0]");
    FCheckValues(p.X[1], p.XAcc, p.XWidth, "p.X[1]");
    FCheckValues(p.X[2], p.XAcc, p.XWidth, "p.X[2]");

    if (p.V[0].Len != p.V[1].Len ||  p.V[0].Len != p.V[2].Len) {
        Panic("The lengths of the three velocity sequences are (%"PRId32
              ", %"PRId32", %"PRId32"), but they all need to be the "
              "same length.", p.V[0].Len, p.V[1].Len,
              p.V[2].Len);
    }
    FCheckValues(p.V[0], p.VAcc, 0, "p.V[0]");
    FCheckValues(p.V[1], p.VAcc, 0, "p.V[1]");
    FCheckValues(p.V[2], p.VAcc, 0, "p.V[2]");

    if (p.V[0].Len != len && p.V[0].Len != 0) {
        Panic("The length of the velocity sequence is %"PRId32", but the "
              "length of the position sequence is %"PRId32".", p.V[0].Len, len);
    }

    if (p.ID.Len != len && p.ID.Len != 0) {
        Panic("The length of the ID sequence is %"PRId32", but the length of "
              "the position sequence is %"PRId32".", p.ID.Len, len);
    }
    U64CheckValues(p.ID, p.IDWidth*p.IDWidth*p.IDWidth, "p.ID");

    for (int32_t i = 0; i < p.FVars.Len; i++) {
        if (p.FVars.Data[i].Len != len &&
            p.FVars.Data[i].Len != 0) {
            Panic("The length of the FVars[%"PRId32"] sequence is %"PRId32
                  ", but the length of the position sequence is %"PRId32".",
                  i, p.FVars.Data[i].Len, len);
        }
        char name[20];
        sprintf(name, "p.FVars[%"PRId32"]", i);
        FCheckValues(p.FVars.Data[i], p.FVarsAcc[i], 0, name);
    }

    for (int32_t i = 0; i < p.U64Vars.Len; i++) {
        if (p.U64Vars.Data[i].Len != len &&
            p.U64Vars.Data[i].Len != 0) {
            Panic("The length of the U64Vars[%"PRId32"] sequence is %"PRId32
                  ", but the length of the position sequence is %"PRId32".",
                  i, p.U64Vars.Data[i].Len, len);
        }
    }
}

/********************/
/* Helper Functions */
/********************/

/* FCheckValues ensures that all values are bounded and non-NaN. */
void FCheckValues(FSeq x, algo_Accuracy acc, float bound, char *seqName) {
    if (x.Len == 0) { return; }

    if (acc.Delta <= 0 && x.Len != acc.Deltas.Len) {
        Panic("Length of %s is %"PRId32", but length of the corresponding "
              "Deltas sequences is %"PRId32".", seqName, x.Len, acc.Deltas.Len);
    }

    if (bound > 0) {
        for (int32_t i = 0; i < x.Len; i++) {
            if (x.Data[i] > bound || x.Data[i] < 0) {
                Panic("Value %"PRId32" in %s is %g, which is outside "
                      "its range of [0, %g).", i, seqName, x.Data[i], bound);
            } else if (x.Data[i] != x.Data[i]) {
                Panic("Value %"PRId32" in %s is NaN.", i, seqName);
            }
        }
    } else {
        for (int32_t i = 0; i < x.Len; i++) {
            if (x.Data[i] > FLT_MAX || x.Data[i] < -FLT_MAX) {
                Panic("Value %"PRId32" in %s is %g.", i, seqName, x.Data[i]);
            } else if (x.Data[i] != x.Data[i]) {
                Panic("Value %"PRId32" in %s is NaN.", i, seqName);
            }
        }
    }
}

/* U64CheckValues ensures that all values are bounded. */
void U64CheckValues(U64Seq x, uint64_t bound, char *seqName) {
    if (x.Len == 0) { return; }

    for (int32_t i = 0; i < x.Len; i++) {
        if (x.Data[i] >= bound) {
            Panic("Value %"PRId32" in %s is %"PRIu64", which is outside"
                  " its range [0, %"PRIu64").", i, seqName, x.Data[i], bound);
        }
    }
}

void U32CheckValues(
    U32Seq x, uint32_t bound, algo_QuantizedRange range, char *seqName
) {
    if (x.Len == 0) { return; }

    if (range.Depths.Len != 0) {
        if (range.Depths.Len != x.Len) {
            Panic("Length of %s is %"PRId32", but length of corresponding "
                  "Depths sequence is %"PRId32".", seqName, x.Len,
                  range.Depths.Len);
        }

        for (int32_t i = 0; i < x.Len; i++) {
            bound = 1 << range.Depths.Data[i];
            if (x.Data[i] >= bound) {
                Panic("Value %"PRId32" in %s is %"PRIu32", which is outside"
                      " its range [0, %"PRIu32").", i, seqName, x.Data[i],
                      bound);
            }
        }
    } else {
        if (bound == 0) { bound = 1<<range.Depth; }
        for (int32_t i = 0; i < x.Len; i++) {
            if (x.Data[i] >= bound) {
                Panic("Value %"PRId32" in %s is %"PRIu32", which is outside"
                      " its range [0, %"PRIu32").", i, seqName, x.Data[i],
                      bound);
            }   
        }
    }
}

void U32CheckVectorValues(
    U32Seq x, uint32_t bound, algo_QuantizedVectorRange range, char *seqName
) {
    if (x.Len == 0) { return; }

    if (range.Depths.Len != 0) {
        if (range.Depths.Len != x.Len) {
            Panic("Length of %s is %"PRId32", but length of corresponding "
                  "Depths sequence is %"PRId32".", seqName, x.Len,
                  range.Depths.Len);
        }

        for (int32_t i = 0; i < x.Len; i++) {
            bound = 1 << range.Depths.Data[i];
            if (x.Data[i] >= bound) {
                Panic("Value %"PRId32" in %s is %"PRIu32", which is outside"
                      " its range [0, %"PRIu32").", i, seqName, x.Data[i],
                      bound);
            }
        }
    } else {
        if (bound == 0) { bound = 1<<range.Depth; }
        for (int32_t i = 0; i < x.Len; i++) {
            if (x.Data[i] >= bound) {
                Panic("Value %"PRId32" in %s is %"PRIu32", which is outside"
                      " its range [0, %"PRIu32").", i, seqName, x.Data[i],
                      bound);
            }   
        }
    }
}

U8Seq U8SetLen(U8Seq s, int32_t len) {
    s = U8Seq_Extend(s, len);
    s = U8Seq_Sub(s, 0, len);
    memset(s.Data, 0, (size_t)s.Len * sizeof(*s.Data));
    return s;
}

U32Seq U32SetLen(U32Seq s, int32_t len) {
    s = U32Seq_Extend(s, len);
    s = U32Seq_Sub(s, 0, len);
    memset(s.Data, 0, (size_t)s.Len * sizeof(*s.Data));
    return s;
}

U64Seq U64SetLen(U64Seq s, int32_t len) {
    s = U64Seq_Extend(s, len);
    s = U64Seq_Sub(s, 0, len);
    memset(s.Data, 0, (size_t)s.Len * sizeof(*s.Data));
    return s;
}

FSeq FSetLen(FSeq s, int32_t len) {
    s = FSeq_Extend(s, len);
    s = FSeq_Sub(s, 0, len);
    memset(s.Data, 0, (size_t)s.Len * sizeof(*s.Data));
    return s;
}

/* MinMax calculates the minimum and maximum values of a sequence  */
void FMinMax(FSeq x, float *minOut, float *maxOut) {
    DebugAssert(x.Len > 0) {
        Panic("Cannot find maximum of an empty sequence.%s", "");
    }

    float min = x.Data[0];
    float max = x.Data[0];
    for (int32_t i = 0; i < x.Len; i++) {
        if (x.Data[i] > max) {
            max = x.Data[i];
        } else if (x.Data[i] < min) {
            min = x.Data[i];
        }
    }

    *minOut = min;
    *maxOut = max;
}

void FBoundedMinMax(FSeq x, float dx, float *minOut, float *maxOut) {
    (void) dx;
    FMinMax(x, minOut, maxOut);
}

/* SetQuantizedRanges sets all the *Range fields to the correct values. */
algo_QuantizedParticles SetQuantizedRanges(
    algo_Particles p, algo_QuantizedParticles buf
) {
    float xMin[3], xMax[3];
    for (int i = 0; i < 3; i++) {
        FBoundedMinMax(p.X[i], p.XWidth, &xMin[i], &xMax[i]);
    }
    buf.XRange = AccuracyToXVectorRange(
        p.XAcc, xMin, xMax, p.XWidth, buf.XRange
    );

    if (p.V[0].Len > 0) {
        float vMin[3], vMax[3];
        for (int i = 0; i < 3; i++) {
            FMinMax(p.V[i], &vMin[i], &vMax[i]);
        }
        buf.VRange = AccuracyToVectorRange(
            p.XAcc, vMin, vMax, buf.VRange
        );
    }

    buf.FVarsRange = realloc(
        buf.FVarsRange, sizeof(*buf.FVarsRange) * (size_t)p.FVars.Len
    );

    for (int32_t i = 0; i < p.FVars.Len; i++) {
        if (p.FVars.Data[i].Len == 0) { continue; }
        float min, max;
        FMinMax(p.FVars.Data[i], &min, &max);
        buf.FVarsRange[i] = AccuracyToRange(
            p.FVarsAcc[i], min, max, buf.FVarsRange[i]
        );
    }

    return buf;
}

/* AccuracyToRange returns the algo_QuantizedRange which specifies
 * particles to the lowest amount of precision that is still consistent with an
 * input algo_Accuracy. */
algo_QuantizedRange AccuracyToRange(
    algo_Accuracy acc, float x0, float x1, algo_QuantizedRange buf
) {
    if (acc.Deltas.Len == 0) {
        uint8_t depth;
        for (depth = 0; depth <= 24; depth++) {
            if (acc.Delta * (float) (1 << depth) > x1 - x0) { break; }
        }

        DebugAssert(depth <= 24) {
            Panic("An accuracy of %g was requested for variables with a range "
                  "of [%g, %g], but this exceeds the granularity of single "
                  "precision floats, which only support 24 bits of mantissa "
                  "precision.", acc.Delta, x0, x1);
        }
        
        buf.X0 = x0;
        buf.X1 = x0 + acc.Delta * (float) (1 << depth);
        buf.Depth = depth;
        buf.Depths = U8Seq_Sub(buf.Depths, 0, 0);
    } else {
        /* This can be optimized with a binary search if need be. */

        buf.Depths = U8SetLen(buf.Depths, acc.Deltas.Len);

        float prevDelta = -1;
        uint8_t prevDepth = (uint8_t)256;
        float maxWidth = x1 - x0;

        for (int32_t i = 0; i < acc.Deltas.Len; i++) {
            float delta = acc.Deltas.Data[i];
            if (prevDelta == delta) {
                buf.Depths.Data[i] = prevDepth;
            } else {

                uint8_t depth;
                for (depth = 0; depth <= 24; depth++) {
                    float width = acc.Delta * (float) (1 << depth);
                    if (width > x1 - x0) {
                        if (width > maxWidth) { maxWidth = width; }
                        break;
                    }
                }

                DebugAssert(depth <= 24) {
                    Panic("An accuracy of %g was requested for variables with "
                          "a range of [%g, %g], but this exceeds the "
                          "granularity of single precision floats, which only "
                          "support 24 bits of mantissa precision.",
                          acc.Delta, x0, x1);
                }       
            }
        }

        buf.X0 = x0;
        buf.X1 = x0 + maxWidth;
    }

    return buf;
}

/* AccuracyToVectorRange returns the algo_QuantizedVectorRange which specifies
 * particles to the lowest amount of precision that is still consistent with an
 * input algo_Accuracy. */
algo_QuantizedVectorRange AccuracyToVectorRange(
    algo_Accuracy acc, float x0[3], float x1[3], algo_QuantizedVectorRange buf
) {
    DebugAssert(acc.Deltas.Len == 0) {
        Panic("Variable accuracies not supported yet.%s", "");
    }

    uint8_t depth;
    for (depth = 0; depth <= 24; depth++) {
        if ((acc.Delta * (float) (1 << depth) > x1[0] - x0[0]) &&
            (acc.Delta * (float) (1 << depth) > x1[1] - x0[1]) &&
            (acc.Delta * (float) (1 << depth) > x1[2] - x0[2])) { break; }
    }
    if (depth > 24) {
        Panic("An accuracy of %g was requested for variables with a range "
              "of [(%g, %g, %g), (%g, %g, %g)], but this exceeds the "
              "granularity of single precision floats, which only support "
              "24 bits of mantissa precision.", acc.Delta, x0[0], x0[1], x0[2],
              x1[0], x1[1], x1[2]);
    }

    for (int i = 0; i < 3; i++) {
        buf.X0[i] = x0[i];
        buf.X1[i] = x0[i] + acc.Delta * (float) (1 << depth);
    }
    buf.Depth = depth;
    buf.Depths = U8Seq_Sub(buf.Depths, 0, 0);

    return buf;
}

/* AccuracyToXVectorRange returns the algo_QuantizedVectorRange which specifies
 * particles to the lowest amount of precision that is still consistent with an
 * input algo_Accuracy. It is identical to AccuracyToVectorRange, except it
 * forces grid cells to be aligned to some global grid with a physical width of
 * xWidth. */
algo_QuantizedVectorRange AccuracyToXVectorRange(
    algo_Accuracy acc, float x0[3], float x1[3],
    float xWidth, algo_QuantizedVectorRange buf
) {
    DebugAssert(acc.Deltas.Len == 0) {
        Panic("Variable accuracies not supported yet.%s", "");
    }

    uint8_t fullDepth;
    for (fullDepth = 0; fullDepth <= 24; fullDepth++) {
        if (acc.Delta * (float) (1 << fullDepth) > xWidth) { break; }
    }
    if (fullDepth > 24) {
        Panic("An accuracy of %g was requested for variables with a range "
              "of [(%g, %g, %g), (%g, %g, %g)], but this exceeds the "
              "granularity of single precision floats, which only support "
              "24 bits of mantissa precision.", acc.Delta, x0[0], x0[1], x0[2],
              x1[0], x1[1], x1[2]);
    }

    for (int i = 0; i < 3; i++) {
        x0[i] = acc.Delta * floorf(x0[i] / acc.Delta);
        x1[i] = acc.Delta * ceilf(x1[i] / acc.Delta);
    }

    return AccuracyToVectorRange(acc, x0, x1, buf);
}
