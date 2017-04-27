#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <float.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "algo.h"
#include "debug.h"
#include "util.h"
    
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
U8SeqSeq U8SeqSeqSetLen(U8SeqSeq s, int32_t len);
U64SeqSeq U64SeqSeqSetLen(U64SeqSeq s, int32_t len);
FSeqSeq FSeqSeqSetLen(FSeqSeq s, int32_t len);
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

/* algo_Quantize subprocedures */
void UndoPeriodic(algo_Particles p);
void Periodic(algo_Particles p);
void Logify(algo_Particles p);
void UndoLogify(algo_Particles p);
algo_QuantizedParticles SetQuantizedRanges(
    algo_Particles p, algo_QuantizedParticles buf
);
algo_QuantizedParticles Quantize(
    algo_Particles p, algo_QuantizedParticles buf
);
algo_QuantizedParticles VectorizeIDs(
    algo_Particles p, algo_QuantizedParticles buf
);
algo_QuantizedParticles CopyU64s(
    algo_Particles p, algo_QuantizedParticles buf
);

/* algo_UndoQuantize subprocedures */
algo_Particles UndoVectorizeIDs(
    algo_QuantizedParticles p, algo_Particles buf
);
algo_Particles UndoCopyU64s(
    algo_QuantizedParticles p, algo_Particles buf
);
algo_Particles SetAccuracies(
    algo_QuantizedParticles p, algo_Particles buf
);
algo_Particles UndoQuantize(
    algo_QuantizedParticles p, algo_Particles buf
);

/**********************/
/* Exported Functions */
/**********************/

algo_QuantizedParticles algo_Quantize(
    algo_Particles p, algo_QuantizedParticles buf
) {
    UndoPeriodic(p);

    Logify(p);

    buf = VectorizeIDs(p, buf);

    buf = SetQuantizedRanges(p, buf);

    buf = Quantize(p, buf);

    buf = CopyU64s(p, buf);

    UndoLogify(p);

    Periodic(p);
	
    return buf;
}

algo_Particles algo_UndoQuantize(
    algo_QuantizedParticles p, algo_Particles buf
) {
    buf = UndoVectorizeIDs(p, buf);

    buf = UndoCopyU64s(p, buf);

    buf = SetAccuracies(p, buf);

    buf = UndoQuantize(p, buf);

    UndoLogify(buf);

    Periodic(buf);

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

/* sizeof(SegmentHeader) MUST be divisible by 4. Otherwise  */
typedef struct {
    int32_t BlockNum, ParticleNum;
    int32_t HasV, HasID, FVarsLen, U64VarsLen;
} SegmentHeader;

U8BigSeq CompressedParticles_ToBytes(
    algo_CompressedParticles p, U8BigSeq buf
) {
    int64_t headerSize = sizeof(SegmentHeader);
    int64_t lenSize = (int64_t) (4 * p.Blocks.Len);
    int64_t checksumSize = (int64_t) (4 * p.Blocks.Len);
    int64_t dataSize = 0;
    for (int32_t i = 0; i < p.Blocks.Len; i++) {
        dataSize += (int64_t) p.Blocks.Data[i].Len;
    }

    buf = U8BigSeq_Extend(buf, headerSize + lenSize + dataSize);
    buf = U8BigSeq_Sub(buf, 0, headerSize + lenSize + dataSize);

    uint8_t *headerPtr = buf.Data;
    int32_t *lenPtr = (int32_t*)(void*)(buf.Data + headerSize);
    uint32_t *checksumPtr = (uint32_t*)(void*)(buf.Data + headerSize + lenSize);
    uint8_t *dataPtr = buf.Data + headerSize + lenSize + checksumSize;

    SegmentHeader hd = {
        .BlockNum = p.Blocks.Len, .ParticleNum = p.ParticleNum,
        .HasV = (int32_t) p.HasV, .HasID = (int32_t) p.HasID,
        .FVarsLen = p.FVarsLen, .U64VarsLen = p.FVarsLen
    };

    hd.BlockNum = util_I32LittleEndian(hd.BlockNum);
    hd.ParticleNum = util_I32LittleEndian(hd.ParticleNum);
    hd.HasV = util_I32LittleEndian(hd.HasV);

    memcpy(headerPtr, &hd, headerSize);

    for (int32_t i = 0; i < p.Blocks.Len; i++) {
        U8Seq block = p.Blocks.Data[i];
        lenPtr[i] = block.Len;
        checksumPtr[i] = util_Checksum(block);
        memcpy(dataPtr, block.Data, block.Len);
        dataPtr += block.Len;
    }

    return buf;
}

algo_CompressedParticles CompressedParticles_FromBytes(
    U8BigSeq bytes, algo_CompressedParticles buf
) {
    int64_t headerSize = sizeof(SegmentHeader);
    SegmentHeader hd;
    memcpy(&hd, bytes.Data, headerSize);
    int64_t lenSize = (int64_t) sizeof(buf.Blocks.Data[0].Len) * hd.BlockNum;

    uint8_t *lenPtr = bytes.Data + headerSize;
    uint8_t *dataPtr = bytes.Data + headerSize + lenSize;

    buf.Blocks = U8SeqSeqSetLen(buf.Blocks, hd.BlockNum);
    buf.Blocks = U8SeqSeq_Sub(buf.Blocks, 0, hd.BlockNum);
    int32_t blockLenSize = sizeof(buf.Blocks.Data[0].Len);
    for (int32_t i = 0; i < buf.Blocks.Len; i++) {
        int32_t len;
        memcpy(&len, lenPtr, blockLenSize);

        buf.Blocks.Data[i] = U8SetLen(buf.Blocks.Data[i], len);
        buf.Blocks.Data[i] = U8Seq_Sub(buf.Blocks.Data[i], 0, len);
        memcpy(buf.Blocks.Data[i].Data, dataPtr, len);

        lenPtr += blockLenSize;
        dataPtr += len;
    }

    return buf;
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
            if (x.Data[i] >= bound || x.Data[i] < 0) {
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

/* The *SetLen functions are the correct way to set the length of a buffer to
 * some value which is potentially larger than its cap size. The scalar
 * sequences clear the buffer, and the nested sequences do something slightly
 * more complicated to make sure memory isn't leaked or unneccessarily
 * reallocated. The idea is that you would SetLen the nested sequence and then
 * SetLen all the elements. As long as you do that, nothing bad happens. */

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

U32SeqSeq U32SeqSeqSetLen(U32SeqSeq s, int32_t len) {
    int32_t initialSize = s.Len;
    s = U32SeqSeq_Extend(s, len);
    s = U32SeqSeq_Sub(s, 0, len);
    if (initialSize < s.Len) {
        memset(s.Data + initialSize, 0,
               (size_t)(s.Len - initialSize) * sizeof(*s.Data));
    }
    return s;
}

U8SeqSeq U8SeqSeqSetLen(U8SeqSeq s, int32_t len) {
    int32_t initialSize = s.Len;
    s = U8SeqSeq_Extend(s, len);
    s = U8SeqSeq_Sub(s, 0, len);
    if (initialSize < s.Len) {
        memset(s.Data + initialSize, 0,
               (size_t)(s.Len - initialSize) * sizeof(*s.Data));
    }
    return s;
}

U64SeqSeq U64SeqSeqSetLen(U64SeqSeq s, int32_t len) {
    int32_t initialSize = s.Len;
    s = U64SeqSeq_Extend(s, len);
    s = U64SeqSeq_Sub(s, 0, len);
    if (initialSize < s.Len) {
        memset(s.Data + initialSize, 0,
               (size_t)(s.Len - initialSize) * sizeof(*s.Data));
    }
    return s;
}

FSeqSeq FSeqSeqSetLen(FSeqSeq s, int32_t len) {
    int32_t initialSize = s.Len;
    s = FSeqSeq_Extend(s, len);
    s = FSeqSeq_Sub(s, 0, len);
    if (initialSize < s.Len) {
        memset(s.Data + initialSize, 0,
               (size_t)(s.Len - initialSize) * sizeof(*s.Data));
    }
    return s;
}

FSeq FSetLen(FSeq s, int32_t len) {
    s = FSeq_Extend(s, len);
    s = FSeq_Sub(s, 0, len);
    memset(s.Data, 0, (size_t)s.Len * sizeof(*s.Data));
    return s;
}

void UndoPeriodic(algo_Particles p) {
    for (int i = 0; i < 3; i++) {
        util_UndoPeriodic(p.X[i], p.XWidth);
    }
}

void Periodic(algo_Particles p) {
    for (int i = 0; i < 3; i++) {
        util_Periodic(p.X[i], p.XWidth);
    }
}

void Logify(algo_Particles p) {
    for (int32_t i = 0; i < p.FVars.Len; i++) {
        if (!p.FVarsAcc[i].Log) { continue; }
        FSeq var = p.FVars.Data[i];
        for (int32_t j = 0; j < var.Len; j++) {
            var.Data[j] = log10f(var.Data[j]);
        }
    }
}

void UndoLogify(algo_Particles p) {
    for (int32_t i = 0; i < p.FVars.Len; i++) {
        if (!p.FVarsAcc[i].Log) { continue; }
        FSeq var = p.FVars.Data[i];
        for (int32_t j = 0; j < var.Len; j++) {
            var.Data[j] = powf(10, var.Data[j]);
        }
    }
}

/* SetQuantizedRanges sets all the QuantizedRange fields to the correct values.
 * 
 * Note that the IDRange structs aren't included in this. Those are modified
 * in*/
algo_QuantizedParticles SetQuantizedRanges(
    algo_Particles p, algo_QuantizedParticles buf
) {
    float xMin[3], xMax[3];
    for (int i = 0; i < 3; i++) {
        util_MinMax(p.X[i], &xMin[i], &xMax[i]);
    }
    buf.XRange = AccuracyToXVectorRange(
        p.XAcc, xMin, xMax, p.XWidth, buf.XRange
    );

    if (p.V[0].Len > 0) {
        float vMin[3], vMax[3];
        for (int i = 0; i < 3; i++) {
            util_MinMax(p.V[i], &vMin[i], &vMax[i]);
        }
        buf.VRange = AccuracyToVectorRange(p.VAcc, vMin, vMax, buf.VRange);
    }

    buf.FVarsRange = realloc(
        buf.FVarsRange, sizeof(*buf.FVarsRange) * (size_t)p.FVars.Len
    );

	memset(buf.FVarsRange + buf.FVars.Len, 0,
		   sizeof(*buf.FVarsRange) * (size_t)(p.FVars.Len - buf.FVars.Len));

    for (int32_t i = 0; i < p.FVars.Len; i++) {
        if (p.FVars.Data[i].Len == 0) { continue; }
        float min, max;
        util_MinMax(p.FVars.Data[i], &min, &max);
        buf.FVarsRange[i] = AccuracyToRange(
            p.FVarsAcc[i], min, max, buf.FVarsRange[i]
        );
    }

    return buf;
}

/* VectorizedIDs converts the particle IDs into vectors. */
algo_QuantizedParticles VectorizeIDs(
    algo_Particles p, algo_QuantizedParticles buf
) {
    if (p.ID.Len == 0) {
        for (int i = 0; i < 3; i++) {
            buf.ID[i] = U32Seq_Sub(buf.ID[i], 0, 0);
        }
        return buf;
    }

    buf.IDWidth = p.IDWidth;
    for (int i = 0; i < 3; i++) {
        buf.ID[i] = U32SetLen(buf.ID[i], p.ID.Len);
    }

    uint64_t w = (uint64_t) buf.IDWidth;
    for (int32_t i = 0; i < p.ID.Len; i++)  {
        uint64_t id = p.ID.Data[i];
        buf.ID[0].Data[i] = (uint32_t) (id % w);
        buf.ID[1].Data[i] = (uint32_t) ((id / w) % w);
        buf.ID[2].Data[i] = (uint32_t) (id / (w*w));
    }

    for (int i = 0; i < 3; i++) {
        util_U32UndoPeriodic(buf.ID[i], buf.IDWidth);
        /* Note that the range setting is done here and not in
         * SetQuantizedRanges. */
        util_U32MinMax(buf.ID[i], &buf.IDRange.X0[i], &buf.IDRange.X1[i]);
        for (int32_t j = 0; j < buf.ID[i].Len; j++) {
            buf.ID[i].Data[j] -= buf.IDRange.X0[i];
        }
    }

    return buf;
}

/* Quantize floats quantizes the floating point fields of p into buf. */
algo_QuantizedParticles Quantize(
    algo_Particles p, algo_QuantizedParticles buf
) {
    buf.XWidth = p.XWidth;

    for (int i = 0; i < 3; i++) {
        float x0 = buf.XRange.X0[i];
        float dx = buf.XRange.X1[i] - buf.XRange.X0[i];

        if (buf.XRange.Depths.Len == 0) {
            buf.X[i] = util_UniformBinIndex(
                p.X[i], buf.XRange.Depth, x0, dx, buf.X[i]
            );
        } else {
            buf.X[i] = util_BinIndex(
                p.X[i], buf.XRange.Depths, x0, dx, buf.X[i]
            );
        }
    }

    if (p.V[0].Len != 0) {
        for (int i = 0; i < 3; i++) {
            float v0 = buf.VRange.X0[i];
            float dv = buf.VRange.X1[i] - buf.VRange.X0[i];

            if (buf.XRange.Depths.Len == 0) {
                buf.V[i] = util_UniformBinIndex(
                    p.V[i], buf.VRange.Depth, v0, dv, buf.V[i]
                );
            } else {
                buf.V[i] = util_BinIndex(
                    p.V[i], buf.VRange.Depths, v0, dv, buf.V[i]
                );
            }
        }
    }

    buf.FVars = U32SeqSeqSetLen(buf.FVars, p.FVars.Len);

    for (int32_t i = 0; i < p.FVars.Len; i++) {
        FSeq var = p.FVars.Data[i];
        if (var.Len == 0) { continue; }
		
        float f0 = buf.FVarsRange[i].X0;
        float df = buf.FVarsRange[i].X1 - buf.FVarsRange[i].X0;
        if (buf.FVarsRange[i].Depths.Len == 0) {
            buf.FVars.Data[i] = util_UniformBinIndex(
                var, buf.FVarsRange[i].Depth, f0, df, buf.FVars.Data[i]
            );
        } else {
            buf.FVars.Data[i] = util_BinIndex(
                var, buf.FVarsRange[i].Depths, f0, df, buf.FVars.Data[i]
            );
        }
    }

    return buf;
}

/* CopyU64s copies the U64 variables from p to buf. */
algo_QuantizedParticles CopyU64s(
    algo_Particles p, algo_QuantizedParticles buf
) {
    buf.U64Vars = U64SeqSeqSetLen(buf.U64Vars, p.U64Vars.Len);
    for (int32_t i = 0; i < p.U64Vars.Len; i++) {
        U64Seq src = p.U64Vars.Data[i];
        U64Seq dst = buf.U64Vars.Data[i];

        dst = U64SetLen(dst, src.Len);
        buf.U64Vars.Data[i] = dst;

        memcpy(dst.Data, src.Data, sizeof(dst.Data[0]) * (size_t)dst.Len);
    }

    return buf;
}

algo_Particles UndoVectorizeIDs(
    algo_QuantizedParticles p, algo_Particles buf
) {
    if (p.ID[0].Len == 0) {
        buf.ID = U64Seq_Sub(buf.ID, 0, 0);
        return buf;
    }

    for (int i = 0; i < 3; i++) {
        for (int32_t j = 0; j < p.ID[i].Len; j++) {
            p.ID[i].Data[j] += p.IDRange.X0[i];
        }
        util_U32Periodic(p.ID[i], p.IDWidth);
    }

    buf.ID = U64SetLen(buf.ID, p.ID[0].Len);
    buf.IDWidth = p.IDWidth;

    uint64_t w = (uint64_t)p.IDWidth;
    for (int32_t i = 0; i < buf.ID.Len; i++) {
        uint64_t x = p.ID[0].Data[i];
        uint64_t y = p.ID[1].Data[i];
        uint64_t z = p.ID[2].Data[i];
        buf.ID.Data[i] = x + y*w + z*w*w;
    }

    return buf;
}

void RangeToAccuracy(
    uint8_t depth, U8Seq depths, float x0, float x1,
    float *deltaPtr, FSeq *deltasPtr
) {
    if (depths.Len == 0) {
        *deltaPtr = (x1 - x0) / (float)(1<<depth);
    } else {
        FSeq deltas = FSetLen(*deltasPtr, depths.Len);
        *deltasPtr = deltas;
        for (int32_t i = 0; i < deltas.Len; i++) {
            deltas.Data[i] = (x1 - x0) / (float)(1<<depth);
        }
    }
}

algo_Particles SetAccuracies(algo_QuantizedParticles p, algo_Particles buf) {
    RangeToAccuracy(
        p.XRange.Depth, p.XRange.Depths,
        p.XRange.X0[0], p.XRange.X1[0],
        &buf.XAcc.Delta, &buf.XAcc.Deltas
    );

    RangeToAccuracy(
        p.VRange.Depth, p.VRange.Depths,
        p.VRange.X0[0], p.VRange.X1[0],
        &buf.VAcc.Delta, &buf.VAcc.Deltas
    );

    buf.FVarsAcc = realloc(
        buf.FVarsAcc, sizeof(*buf.FVarsAcc) * (size_t)p.FVars.Len
    );

	memset(buf.FVarsAcc + buf.FVars.Len, 0,
		   sizeof(*buf.FVarsAcc) * (size_t)(p.FVars.Len - buf.FVars.Len));

    for (int32_t i = 0; i < p.FVars.Len; i++) {
        buf.FVarsAcc[i].Log = p.FVarsRange[i].Log;
        RangeToAccuracy(
            p.FVarsRange[i].Depth, p.FVarsRange[i].Depths,
            p.FVarsRange[i].X0, p.FVarsRange[i].X1, 
            &buf.FVarsAcc[i].Delta, &buf.FVarsAcc[i].Deltas
        );
    }

    return buf;
}

algo_Particles UndoCopyU64s(
    algo_QuantizedParticles p, algo_Particles buf
) {
    /* hmmmmm */

    buf.U64Vars = U64SeqSeqSetLen(buf.U64Vars, p.U64Vars.Len);
    for (int32_t i = 0; i < p.U64Vars.Len; i++) {
        U64Seq src = p.U64Vars.Data[i];
        U64Seq dst = buf.U64Vars.Data[i];

        dst = U64SetLen(dst, src.Len);
        buf.U64Vars.Data[i] = dst;

        memcpy(dst.Data, src.Data, sizeof(dst.Data[0]) * (size_t)dst.Len);
    }

    return buf;
}

algo_Particles UndoQuantize(
    algo_QuantizedParticles p, algo_Particles buf
) {
    rand_State *s = rand_Seed((uint64_t)clock(), 1);

    buf.XWidth = p.XWidth;

    for (int i = 0; i < 3; i++) {
        float x0 = p.XRange.X0[i];
        float dx = p.XRange.X1[i] - p.XRange.X0[i];

        if (p.XRange.Depths.Len == 0) {
            buf.X[i] = util_UndoUniformBinIndex(
                p.X[i], p.XRange.Depth, x0, dx, s, buf.X[i]
            );
        } else {
            buf.X[i] = util_UndoBinIndex(
                p.X[i], p.XRange.Depths, x0, dx, s, buf.X[i]
            );
        }
    }

    if (p.V[0].Len != 0) {
        for (int i = 0; i < 3; i++) {
            float v0 = p.VRange.X0[i];
            float dv = p.VRange.X1[i] - p.VRange.X0[i];

            if (p.XRange.Depths.Len == 0) {
                buf.V[i] = util_UndoUniformBinIndex(
                    p.V[i], p.VRange.Depth, v0, dv, s, buf.V[i]
                );
            } else {
                buf.V[i] = util_UndoBinIndex(
                    p.V[i], p.VRange.Depths, v0, dv, s, buf.V[i]
                );
            }
        }
    }

    buf.FVars = FSeqSeqSetLen(buf.FVars, p.FVars.Len);

    for (int32_t i = 0; i < p.FVars.Len; i++) {
        U32Seq var = p.FVars.Data[i];
        if (var.Len == 0) { continue; }
		
        float f0 = p.FVarsRange[i].X0;
        float df = p.FVarsRange[i].X1 - p.FVarsRange[i].X0;
        if (p.FVarsRange[i].Depths.Len == 0) {
            buf.FVars.Data[i] = util_UndoUniformBinIndex(
                var, p.FVarsRange[i].Depth, f0, df, s, buf.FVars.Data[i]
            );
        } else {
            buf.FVars.Data[i] = util_UndoBinIndex(
                var, p.FVarsRange[i].Depths, f0, df, s, buf.FVars.Data[i]
            );
        }
    }

    free(s);

    return buf;
}

/* AccuracyToRange returns the algo_QuantizedRange which specifies
 * particles to the lowest amount of precision that is still consistent with an
 * input algo_Accuracy. */
algo_QuantizedRange AccuracyToRange(
    algo_Accuracy acc, float x0, float x1, algo_QuantizedRange buf
) {
    buf.Log = acc.Log;

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

        buf.Depths = U8SetLen(buf.Depths, acc.Deltas.Len);

        /* These are tracked for the common case where subseqeunt values have
         * the same depth, as an optimization. */
        float prevDelta = -1;
        uint8_t prevDepth = (uint8_t)256;

        float minWidth = 2*(x1 - x0);
        if (minWidth == 0) { minWidth = FLT_MAX; }

        for (int32_t i = 0; i < acc.Deltas.Len; i++) {
            float delta = acc.Deltas.Data[i];

            if (prevDelta == delta) {
                buf.Depths.Data[i] = prevDepth;

            } else {
                uint8_t depth;
                for (depth = 0; depth <= 24; depth++) {
                    /* A faster approach would be to use binary search. */
                    float width = acc.Deltas.Data[i] * (float) (1 << depth);
                    if (width > x1 - x0) {
                        if (width < minWidth) { minWidth = width; }
                        buf.Depths.Data[i] = depth;
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
        buf.X1 = x0 + minWidth;
    }

    return buf;
}

/* AccuracyToVectorRange returns the algo_QuantizedVectorRange which specifies
 * particles to the lowest amount of precision that is still consistent with an
 * input algo_Accuracy. */
algo_QuantizedVectorRange AccuracyToVectorRange(
    algo_Accuracy acc, float x0[3], float x1[3], algo_QuantizedVectorRange buf
) {
    if (acc.Deltas.Len == 0) {

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
                  "24 bits of mantissa precision.", acc.Delta,
                  x0[0], x0[1], x0[2], x1[0], x1[1], x1[2]);
        }
        
        for (int i = 0; i < 3; i++) {
            buf.X0[i] = x0[i];
            buf.X1[i] = x0[i] + acc.Delta * (float) (1 << depth);
        }
        buf.Depth = depth;
        buf.Depths = U8Seq_Sub(buf.Depths, 0, 0);
        
    } else {
        
        buf.Depths = U8SetLen(buf.Depths, acc.Deltas.Len);
        
        /* These are tracked for the common case where subseqeunt values have
         * the same depth, as an optimization. */
        float prevDelta = -1;
        uint8_t prevDepth = (uint8_t)256;

        float minWidth = 0;
        for (int i = 0; i < 3; i++) {
            if (minWidth < 2*(x1[i] - x0[i])) {
                minWidth = 2*(x1[i] - x0[i]);
            }
        }
        if (minWidth == 0) { minWidth = FLT_MAX; }

        for (int32_t i = 0; i < acc.Deltas.Len; i++) {
            float delta = acc.Deltas.Data[i];

            if (prevDelta == delta) {
                buf.Depths.Data[i] = prevDepth;

            } else {
                uint8_t depth;
                for (depth = 0; depth <= 24; depth++) {
                    /* A faster approach would be to use binary search. */
                    float width = acc.Deltas.Data[i] * (float) (1 << depth);
                    if (width > x1[0] - x0[0] &&
                        width > x1[1] - x0[1] &&
                        width > x1[2] - x0[2]) {
                        if (width < minWidth) { minWidth = width; }
                        buf.Depths.Data[i] = depth;
                        break;
                    }
                }

                DebugAssert(depth <= 24) {
                    Panic("An accuracy of %g was requested for variables with "
                          "a range of [(%g, %g, %g), (%g, %g, %g)], but this "
                          "exceeds the granularity of single precision "
                          "floats, which only support 24 bits of mantissa "
                          "precision.", acc.Delta,
                          x0[0], x0[1], x0[2], x1[0], x1[1], x1[2]);
                }       
            }
        }

        for (int i = 0; i < 3; i++) {
            buf.X0[i] = x0[i];
            buf.X1[i] = x0[i] + minWidth;
        }
    }

    return buf;
}

/* AccuracyToXVectorRange returns the algo_QuantizedVectorRange which specifies
 * particles to the lowest amount of precision that is still consistent with an
 * input algo_Accuracy. It is identical to AccuracyToVectorRange, except it
 * forces grid cells to be aligned to some global grid with a physical width of
 * xWidth.
 *
 * This requirement adds significant complexity. */
algo_QuantizedVectorRange AccuracyToXVectorRange(
    algo_Accuracy acc, float x0[3], float x1[3],
    float xWidth, algo_QuantizedVectorRange buf
) {
    DebugAssert(acc.Deltas.Len == 0) {
        Panic("Variable accuracies not supported yet.%s", "");
    }

    uint8_t fullDepth;
    for (fullDepth = 0; fullDepth <= 24; fullDepth++) {
        if (acc.Delta * (float) (1 << fullDepth) >= xWidth) { break; }
    }

    if (fullDepth > 24) {
        Panic("An accuracy of %g was requested for variables with a range "
              "of [(%g, %g, %g), (%g, %g, %g)], but this exceeds the "
              "granularity of single precision floats, which only support "
              "24 bits of mantissa precision.", acc.Delta, x0[0], x0[1], x0[2],
              x1[0], x1[1], x1[2]);
    }

    /* Align range edges with global grid. */
    float trueDelta = xWidth / (float) (1 << fullDepth);

    float maxDiff = 0;
    for (int i = 0; i < 3; i++) {
        buf.X0[i] = trueDelta * floorf(x0[i] / trueDelta);
        float xx1 = trueDelta * ceilf(x1[i] / trueDelta);
        if (maxDiff < xx1 - buf.X0[i]) { maxDiff = xx1 - buf.X0[i]; }
    }

    uint8_t segmentDelta;
    for (segmentDelta = 0; segmentDelta <= 24; segmentDelta++) {
        if (trueDelta * (float) (1 << segmentDelta) >= maxDiff) { break; }
    }

    buf.Depth = segmentDelta;
    for (int i = 0; i < 3; i++) {
        buf.X1[i] = buf.X0[i] + trueDelta  * (float) (1 << buf.Depth);
    }

    return buf;
}
