#include "compress_util.h"
#include "debug.h"
#include "lz4.h"
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

void util_MinMax(FSeq x, float *minPtr, float *maxPtr) {
    int32_t n = x.Len;
    float *xs = x.Data;

    DebugAssert(n > 0) {
        Panic("Empty sequence given to util_MinMax.%s", "");
    }

    /* This is a hot inner loop for some algorithms. */

    float min, max;
    int32_t i;
    if ((n & 1) == 1) {
        min = xs[0];
        max = xs[0];
        i = 1;
    } else {
        if (xs[0] > xs[1]) {
            min = xs[1];
            max = xs[0];
        } else {
            min = xs[0];
            max = xs[1];
        }
        i = 2;
    }

    for (; i < n; i++) {
        if (xs[i] > max) {
            max = xs[i];
        } else if (xs[i] < min) {
            min = xs[i];
        }
    }
    
    *maxPtr = max;
    *minPtr = min;
}

void util_U32MinMax(U32Seq x, uint32_t *minPtr, uint32_t *maxPtr) {
    int32_t n = x.Len;
    uint32_t *xs = x.Data;

    DebugAssert(n > 0) {
        Panic("Empty sequence given to util_MinMax.%s", "");
    }

    /* This is a hot inner loop for some algorithms. */

    uint32_t min, max;
    int32_t i;
    if ((n & 1) == 1) {
        min = xs[0];
        max = xs[0];
        i = 1;
    } else {
        if (xs[0] > xs[1]) {
            min = xs[1];
            max = xs[0];
        } else {
            min = xs[0];
            max = xs[1];
        }
        i = 2;
    }

    for (; i < n; i++) {
        if (xs[i] > max) {
            max = xs[i];
        } else if (xs[i] < min) {
            min = xs[i];
        }
    }
    
    *maxPtr = max;
    *minPtr = min;
}

void util_Periodic(FSeq x, float L) {
    /* This is a hot inner loop for some algorithms. */

    int32_t n = x.Len;
    float *xs = x.Data;

    for (int32_t i = 0; i < n; i++) {
        float val = xs[i];
        if (val >= L) {
            xs[i] -= L;
        } else if (val < 0) {
            xs[i] += L;
        }
    }
}

void util_U32Periodic(U32Seq x, uint32_t L) {
    /* This is a hot inner loop for some algorithms. */

    int32_t n = x.Len;
    uint32_t *xs = x.Data;

    for (int32_t i = 0; i < n; i++) {
        uint32_t val = xs[i];
        if (val >= L) { xs[i] -= L; }
    }
}

void util_UndoPeriodic(FSeq x, float L) {
    /* This is a hot inner loop for some algorithms. */

    if (x.Len == 0) { return; }

    int32_t n = x.Len;
    float *xs = x.Data;
    float x0 = xs[0];

    for (int32_t i = 0; i < n; i++) {
        if (xs[i] - x0 >= L/2) {
            xs[i] -= L;
        } else if (xs[i] - x0  < -L/2) {
            xs[i] += L;
        }
    }
}

void util_U32UndoPeriodic(U32Seq x, uint32_t L) {
    DebugAssert(UINT32_MAX/2 > L) {
        Panic("L range of %"PRIu32" not supported by util_U32UndoPeriodic.", L);
    }

    if (x.Len == 0) { return; }

    int32_t n = x.Len;
    uint32_t *xs = x.Data;

    for (int32_t i = 0; i < n; i++) { xs[i] += L; }

    uint32_t x0 = xs[0];
    uint32_t min = x0;
    for (int32_t i = 0; i < n; i++) {
        if (xs[i] >= L/2 + x0) {
            xs[i] -= L;
            if (xs[i] < min) { min = xs[i]; }
        } else if (xs[i] < x0 - L + L/2) {
            xs[i] += L;
        }
    }

    if (min >= L) {
        for (int32_t i = 0; i < n; i++) { xs[i] -= L; }
    }
}

U32Seq util_BinIndex(FSeq x, U8Seq level, float x0, float dx, U32Seq buf) {
    DebugAssert(x.Len == level.Len) {
        Panic("BinIndex given x with length %"PRId32", but level with length %"
              PRId32".", x.Len, level.Len);
    }

    buf = U32SeqSetLen(buf, x.Len);
    
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
    DebugAssert(level <= 32) {
        Panic("level set to %"PRIu8", which is above the "
              "limit of 32.", level);
    }

    buf = U32SeqSetLen(buf, x.Len);
    
    float numBins = (float) (1 << level);
    for (int32_t i = 0; i < x.Len; i++) {
        float delta = (x.Data[i] - x0) / dx;
        if (delta < 0) { // must be floating point error
            buf.Data[i] = 0;
        } else if (delta >= 1) { // must be floating point error
            buf.Data[i] = (1<<(uint32_t)level) - 1;
        } else {
            buf.Data[i] = (uint32_t) (delta * numBins);
        }
    }

    return buf;
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
    }

    return buf;
}

FSeq util_UndoUniformBinIndex(
    U32Seq idx, uint8_t level, float x0, float dx, rand_State *state, FSeq buf
) {

    buf = FSeqSetLen(buf, idx.Len);
    uint32_t bins = 1 << (uint32_t) level;
    float binWidth = dx / ((float) bins);

    for (int32_t i = 0; i < idx.Len; i++) {
        DebugAssert(idx.Data[i] < bins) {
            Panic("At index %d, idx = %"PRIu32", which is >= to "
                  "the level, 2^%"PRIu8".", i, idx.Data[i], level);
        }

        float offset = x0 + binWidth*((float)idx.Data[i]);
        buf.Data[i] = offset + rand_Float(state)*binWidth;
    }

    return buf;
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

U32Seq util_U32UniformPack(U32Seq x, uint8_t width, U32Seq buf) {
    DebugAssert(width <= 32) {
        Panic("width = %"PRIu8" specified in UniformPack.", width);
    }

    uint64_t packedBits = width * (uint64_t)x.Len;
    int32_t len = (int32_t)(packedBits/32) + (1-(int32_t)(packedBits%32 == 0));
    buf = U32SeqSetLen(buf, len);
    /* Special cases which break the general version without special casing. */
    if (buf.Len == 0) {
        return buf;
    } else if (width == 32) {
        for (int32_t i = 0; i < buf.Len; i++) {
            buf.Data[i] = x.Data[i];
        }
        return buf;
    }

    for (int32_t i = 0; i < buf.Len; i++) {
        buf.Data[i] = 0;
    }

    uint32_t widthFlag = ~(0xffffffff << width);

    for (int32_t i = 0; i < x.Len; i++) {
        uint32_t val = x.Data[i];

        uint64_t startBit = width * (uint64_t)i;
        int32_t startInt = (int32_t) (startBit / 32);
        uint64_t endBit = width * (1 + (uint64_t)i) - 1;
        int32_t endInt = (int32_t) (endBit / 32);

        if (startInt == endInt) {
            buf.Data[startInt] |= (val & widthFlag) << (startBit % 32);
        } else {
            uint32_t fval = val & widthFlag;
            uint8_t startOffset = (uint8_t) (startBit % 32);
            uint8_t endOffset = (uint8_t) (endBit % 32);
            buf.Data[startInt] |= fval << (startOffset);
            buf.Data[endInt] |= fval >> (width - 1 - endOffset);
        }
    }

    return buf;
}

U32Seq util_U32UndoUniformPack(
    U32Seq x, uint8_t width, int32_t len, U32Seq buf
) {
    DebugAssert(width <= 32) {
        Panic("width = %"PRIu8" specified in UndoUniformPack.", width);
    }

    buf = U32SeqSetLen(buf, len);
    /* Special cases which break the general version without special casing. */
    if (len == 0) {
        return U32Seq_Empty();
    } else if (width == 32) {
        for (int32_t i = 0; i < buf.Len; i++) {
            buf.Data[i] = x.Data[i];
        }
        return buf;
    }

    for (int32_t i = 0; i < buf.Len; i++) {
        buf.Data[i] = 0;
    }

    if (width == 0) {
        return buf;
    }

    uint32_t widthFlag = ~(0xffffffff << width);

    for (int32_t i = 0; i < buf.Len; i++) {
        uint64_t startBit = width * (uint64_t)i;
        int32_t startInt = (int32_t) (startBit / 32);
        uint64_t endBit = width * (1 + (uint64_t)i) - 1;
        int32_t endInt = (int32_t) (endBit / 32);

        if (startInt == endInt) {
            uint8_t startOffset = startBit % 32;
            buf.Data[i] = ((x.Data[startInt] & (widthFlag << startOffset)) >> 
                           startOffset);
        } else {
            uint8_t startOffset = (uint8_t) (startBit % 32);
            uint8_t endOffset = (uint8_t) (endBit % 32);
            uint32_t startFlag = 0xffffffff << startOffset;
            uint32_t endFlag = 0xffffffff >> (32 - endOffset - 1);
            buf.Data[i] = (startFlag & x.Data[startInt]) >> startOffset;
            buf.Data[i] |= (endFlag&x.Data[endInt]) << (width - endOffset - 1);
        }
    }

    return buf;
}

U8Seq util_EntropyEncode(U8Seq data, U8Seq buf) {
    int boundSize = LZ4_compressBound((int) data.Len);
    buf = U8SeqSetLen(buf, (int32_t) boundSize);
    int compressedSize = LZ4_compress_fast(
        (char *)data.Data, (char *)buf.Data, data.Len, buf.Len, 1
    );
    DebugAssert(compressedSize) {
        Panic("%s has failed to compress an input byte sequence.", "LZ4");
    }

    buf = U8Seq_Sub(buf, 0, (int32_t)compressedSize);

    return buf;
}

U8Seq util_UndoEntropyEncode(
    U8Seq compressedData, int32_t uncompressedSize, U8Seq buf
) {
    buf = U8SeqSetLen(buf, uncompressedSize);
    int read = LZ4_decompress_fast(
        (char*)compressedData.Data, (char*)buf.Data, (int) buf.Len
    );

    DebugAssert(read >= 0) {
        Panic("%s has failed to decompress an input byte sequence.", "LZ4");
    }

    return buf;
}

/********************/
/* Helper Functions */
/********************/

U8Seq U8SeqSetLen(U8Seq buf, int32_t len) {
    buf = U8Seq_Extend(buf, len);
    buf = U8Seq_Sub(buf, 0, len);
    return buf;
}

U32Seq U32SeqSetLen(U32Seq buf, int32_t len) {
    buf = U32Seq_Extend(buf, len);
    buf = U32Seq_Sub(buf, 0, len);
    return buf;
}

FSeq FSeqSetLen(FSeq buf, int32_t len) {
    buf = FSeq_Extend(buf, len);
    buf = FSeq_Sub(buf, 0, len);
    return buf;
}
