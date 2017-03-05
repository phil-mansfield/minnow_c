#include "compress_util.h"

U32Seq util_BinIndex(FSeq x, U8Seq level, float x0, float dx, U32Seq buf) {
    (void) x;
    (void) x0;
    (void) dx;
    (void) level;
    (void) buf;
    return U32Seq_Empty;
}


U32Seq util_UniformBinIndex(
    FSeq x, uint8_t level, float x0, float dx, U32Seq buf
) {
    (void) x;
    (void) x0;
    (void) dx;
    (void) level;
    (void) buf;
    return U32Seq_Empty;
}

FSeq util_UndoBinIndex(
    U32Seq idx, U8Seq level, float x0, float dx, FSeq buf
) {
    (void) idx;
    (void) x0;
    (void) dx;
    (void) level;
    (void) buf;
    return FSeq_Empty;
}

FSeq util_UndoUniformBinIndex(
    U32Seq idx, uint8_t level, float x0, float dx, FSeq buf
) {
    (void) idx;
    (void) x0;
    (void) dx;
    (void) level;
    (void) buf;
    return FSeq_Empty;
}

U8Seq util_U32TransposeBytes(U32Seq x, U8Seq buf) {
    (void) x;
    (void) buf;
    return U8Seq_Empty;
}

U8Seq util_U32TransposeBits(U32Seq x, U8Seq buf) {
    (void) x;
    (void) buf;
    return U8Seq_Empty;
}

U32Seq util_U32UndoTransposeBytes(U8Seq x, U32Seq buf) {
    (void) x;
    (void) buf;
    return U32Seq_Empty;
}

U32Seq util_U32UndoTransposeBits(U32Seq x, U8Seq buf) {
    (void) x;
    (void) buf;
    return U32Seq_Empty;
}

U8Seq util_U8DeltaEncode(U8Seq x, U8Seq buf) {
    (void) x;
    (void) buf;
    return U8Seq_Empty;
}

U16Seq util_U16DeltaEncode(U8Seq x, U8Seq buf) {
    (void) x;
    (void) buf;
    return U16Seq_Empty;
}

U32Seq util_U32DeltaEncode(U8Seq x, U8Seq buf) {
    (void) x;
    (void) buf;
    return U32Seq_Empty;
}

U64Seq util_U64DeltaEncode(U8Seq x, U8Seq buf) {
    (void) x;
    (void) buf;
    return U64Seq_Empty;
}

U8Seq util_U8UndoDeltaEncode(U8Seq x, U8Seq buf) {
    (void) x;
    (void) buf;
    return U8Seq_Empty;
}

U16Seq util_U16UndoDeltaEncode(U16Seq x, U8Seq buf) {
    (void) x;
    (void) buf;
    return U16Seq_Empty;
}

U32Seq util_U32UndoDeltaEncode(U32Seq x, U8Seq buf) {
    (void) x;
    (void) buf;
    return U32Seq_Empty;
}

U64Seq util_U64UndoDeltaEncode(U64Seq x, U8Seq buf) {
    (void) x;
    (void) buf;
    return U64Seq_Empty;
}
