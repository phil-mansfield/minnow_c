#include "algo.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include "debug.h"
#include <float.h>

/************************/
/* Forward Declarations */
/************************/

void FCheckValues(FSeq x, algo_Accuracy acc, float bound, char *seqName);

void U64CheckValues(U64Seq x, uint64_t bound, char *seqName);

void U32CheckValues(
    U32Seq x, uint32_t bound, algo_QuantizedRange range, char *seqName
);

algo_Particles Particles_Match(
    algo_QuantizedParticles ref, algo_Particles buf
);

algo_QuantizedParticles QuantizedParticles_Match(
    algo_Particles ref, algo_QuantizedParticles buf
);

/**********************/
/* Exported Functions */
/**********************/

algo_QuantizedParticles algo_Quantize(
    algo_Particles p, algo_QuantizedParticles buf
) {
    (void) p;
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

    for (int32_t i = 0; i < p.FVars.Len; i++) {
        U32Seq_Free(p.FVars.Data[i]);
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
    U32CheckValues(p.X[0], 0, p.XRange, "p.X[0]");
    U32CheckValues(p.X[1], 0, p.XRange, "p.X[1]");
    U32CheckValues(p.X[2], 0, p.XRange, "p.X[2]");

    if (p.V[0].Len != p.V[1].Len ||  p.V[0].Len != p.V[2].Len) {
        Panic("The lengths of the three velocity sequences are (%"PRId32
              ", %"PRId32", %"PRId32"), but they all need to be the "
              "same length.", p.V[0].Len, p.V[1].Len,
              p.V[2].Len);
    }
    U32CheckValues(p.V[0], 0, p.VRange, "p.V[0]");
    U32CheckValues(p.V[1], 0, p.VRange, "p.V[1]");
    U32CheckValues(p.V[2], 0, p.VRange, "p.V[2]");

    if (p.ID[0].Len != p.ID[1].Len ||  p.ID[0].Len != p.ID[2].Len) {
        Panic("The lengths of the three ID sequences are (%"PRId32
              ", %"PRId32", %"PRId32"), but they all need to be the "
              "same length.", p.ID[0].Len, p.ID[1].Len,
              p.ID[2].Len);
    }
    algo_QuantizedRange empty = {0, U8Seq_Empty()};
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
            if (x.Data[i] > 2*bound || x.Data[i] < -bound) {
                Panic("Value %"PRId32" in %s is %g, which is very far outside "
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

algo_Particles Particles_Match(
    algo_QuantizedParticles ref, algo_Particles buf
) {
    (void) ref;
    return buf;
}

algo_QuantizedParticles QuantizedParticles_Match(
    algo_Particles ref, algo_QuantizedParticles buf
) {
    (void) ref;
    return buf;
}
