#include "compress_util.h"
#include "debug.h"
#include <inttypes.h>

U32Seq util_BinIndex(FSeq x, U8Seq level, float x0, float dx, U32Seq buf) {
    (void) x;
    (void) x0;
    (void) dx;
    (void) level;
    (void) buf;
    Panic("%s Not Yet Implemented.", __FUNCTION__);
    return U32Seq_Empty();
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
    (void) idx;
    (void) x0;
    (void) dx;
    (void) level;
    (void) state;
    (void) buf;
    Panic("%s Not Yet Implemented.", __FUNCTION__);
    return FSeq_Empty();
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

    buf = U8Seq_Extend(buf, x.Len*4);
    buf = U8Seq_Sub(buf, 0, x.Len*4);
    U8Seq_Deref(buf);

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

    buf = U32Seq_Extend(buf, x.Len / 4);
    buf = U32Seq_Sub(buf, 0, x.Len / 4);
    U32Seq_Deref(buf);

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
    buf = U8Seq_Extend(buf, x.Len);
    buf = U8Seq_Sub(buf, 0, x.Len);
    U8Seq_Deref(buf);
        
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
    buf = U8Seq_Extend(buf, x.Len);
    buf = U8Seq_Sub(buf, 0, x.Len);
    U8Seq_Deref(buf);

    if (buf.Len == 0) {
        return buf;
    }

    buf.Data[0] = x.Data[0];
    for (int32_t i = 1; i < buf.Len; i++) {
        buf.Data[i] = buf.Data[i-1] + x.Data[i];
    }
    return buf;
}
