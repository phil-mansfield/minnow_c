#include "compress_util.h"
#include "debug.h"
#include <inttypes.h>
#include <math.h>

/************************/
/* Forward Declarations */
/************************/

void checkBinIndexRange(FSeq x, float x0, float dx);
U8Seq U8SeqSetLen(U8Seq buf, int32_t len);
U32Seq U32SeqSetLen(U32Seq buf, int32_t len);
FSeq FSeqSetLen(FSeq buf, int32_t len);

/**********************/
/* Exported Functions */
/**********************/

U32Seq util_BinIndex(FSeq x, U8Seq level, float x0, float dx, U32Seq buf) {
    DebugAssert(x.Len == level.Len) {
        Panic("BinIndex given x with length %"PRId32", but level with length %"
              PRId32".", x.Len, level.Len);
    }

    buf = U32SeqSetLen(buf, x.Len);
    checkBinIndexRange(x, x0, dx);
    
    for (int32_t i = 0; i < x.Len; i++) {
        DebugAssert(level.Data[i] <= 32) {
            Panic("level[%"PRId32"] set to %"PRIu8", which is above the "
                  "limit of 32.", i, level.Data[i]);
        }


        float delta = (x.Data[i] - x0) / dx;
        if (delta < 0) { // must be floating point error
            buf.Data[i] = 0;
        } else if (delta >= 1) { // must be floating point error
            buf.Data[i] = (1<<(uint32_t)level.Data[i]) - 1;
        } else {
            buf.Data[i] = (uint32_t) (delta * (float) (1 << level.Data[i]));
        }
    }

    return buf;
}


U32Seq util_UniformBinIndex(
    FSeq x, uint8_t level, float x0, float dx, U32Seq buf
) {
    (void) x;
    (void) x0;
    (void) dx;
    (void) level;
    (void) buf;
    Panic("%s Not Yet Implemented.", __FUNCTION__);
    return U32Seq_Empty();
}

FSeq util_UndoBinIndex(
    U32Seq idx, U8Seq level, float x0, float dx, rand_State *state, FSeq buf
) {
    DebugAssert(idx.Len == level.Len) {
        Panic("UndoBinIndex given idx with length %"PRId32", but level with "
              "length %"PRId32".", idx.Len, level.Len);
    }

    buf = FSeqSetLen(buf, idx.Len);

    for (int32_t i = 0; i < idx.Len; i++) {
        uint32_t bins = (1 << (uint32_t) level.Data[i]);
        DebugAssert(idx.Data[i] < bins) {
            Panic("At index %d, idx = %"PRIu32", which is >= to "
                  "the level, 2^%"PRIu8".", i, idx.Data[i], level.Data[i]);
        }

        float binWidth = dx / ((float) bins);
        float offset = x0 + binWidth*((float)idx.Data[i]);
        buf.Data[i] = offset + rand_Float(state)*binWidth;

        // Dealing with floating point errors:
        float x1 = x0 + dx;
        if (buf.Data[i] < x0) {
            buf.Data[i] = x0;
        } else if (buf.Data[i] >= x1) {
            buf.Data[i] = nextafterf(x1, -INFINITY);
        }
    }

    return buf;
}

FSeq util_UndoUniformBinIndex(
    U32Seq idx, uint8_t level, float x0, float dx, rand_State *state, FSeq buf
) {
    (void) idx;
    (void) x0;
    (void) dx;
    (void) level;
    (void) state;
    (void) buf;
    Panic("%s Not Yet Implemented.", __FUNCTION__);
    return FSeq_Empty();
}

U8Seq util_U32TransposeBytes(U32Seq x, U8Seq buf) {
    DebugAssert(INT32_MAX / 4 > x.Len) {
        Panic("Input sequence to util_U32TransposeBytes has length %"PRId32
              ", which means Len*4 would overflow.", x.Len);
    }

    buf = U8SeqSetLen(buf, x.Len*4);

    for (int32_t j = 0; j < 4; j++) {
        for (int32_t i = 0; i < x.Len; i++) {
            buf.Data[i + x.Len*j] = (uint8_t) ((x.Data[i] >> 8*j) & 0xff);
        }
    }

    return buf;
}

U32Seq util_U32UndoTransposeBytes(U8Seq x, U32Seq buf) {
    DebugAssert(x.Len % 4 == 0) {
        Panic("util_U32UndoTranposeBytes given a byte sequence of length %"
              PRId32". This cannot be correct because it is not divisible "
              "by four and thus cannot be an encoded uint32 sequence.", x.Len);
    }

    buf = U32SeqSetLen(buf, x.Len / 4);

    for (int32_t i = 0; i < buf.Len; i++) {
        buf.Data[i] = 0;
    }
	
    for (int32_t j = 0; j < 4; j++) {
        for (int32_t i = 0; i < buf.Len; i++) {
            buf.Data[i] |= ((uint32_t) x.Data[i + j*buf.Len]) << 8 * j;
        }
    }

    return buf;
}

U8Seq util_U8DeltaEncode(U8Seq x, U8Seq buf) {
    buf = U8SeqSetLen(buf, x.Len);      
  
    if (buf.Len == 0) {
        return buf; 
    }

    buf.Data[0] = x.Data[0];
    for (int32_t i = x.Len - 1; i > 0; i--) {
        buf.Data[i] = x.Data[i] - x.Data[i - 1];
    }
    return buf;
}

U8Seq util_U8UndoDeltaEncode(U8Seq x, U8Seq buf) {
    buf = U8SeqSetLen(buf, x.Len);

    if (buf.Len == 0) {
        return buf;
    }

    buf.Data[0] = x.Data[0];
    for (int32_t i = 1; i < buf.Len; i++) {
        buf.Data[i] = buf.Data[i-1] + x.Data[i];
    }
    return buf;
}

/********************/
/* Helper Functions */
/********************/

U8Seq U8SeqSetLen(U8Seq buf, int32_t len) {
    buf = U8Seq_Extend(buf, len);
    buf = U8Seq_Sub(buf, 0, len);
    U8Seq_Deref(buf);
    return buf;
}

U32Seq U32SeqSetLen(U32Seq buf, int32_t len) {
    buf = U32Seq_Extend(buf, len);
    buf = U32Seq_Sub(buf, 0, len);
    U32Seq_Deref(buf);
    return buf;
}

FSeq FSeqSetLen(FSeq buf, int32_t len) {
    buf = FSeq_Extend(buf, len);
    buf = FSeq_Sub(buf, 0, len);
    FSeq_Deref(buf);
    return buf;
}

void checkBinIndexRange(FSeq x, float x0, float dx) {
    if (dx > 0)  {
        for (int32_t i = 0; i < x.Len; i++) {
            if (x.Data[i] < x0 || x.Data[i] >= x0 + dx) {
                Panic("Element %"PRId32" of x = %g, but range is [%g, %g).",
                      i, x.Data[i], x0, x0+dx);
            }
        }
    } else {
        for (int32_t i = 0; i < x.Len; i++) {
            if (x.Data[i] > x0 || x.Data[i] <= x0 + dx) {
                Panic("Element %"PRId32" of x = %g, but range is (%g, %g].",
                      i, x.Data[i], x0+dx, x0);
            }
        }
    }
}
