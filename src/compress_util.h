#ifndef MNW_COMPRESS_UTIL_H_
#define MNW_COMPRESS_UTIL_H_

/* Author: Phil Mansfield (mansfield@uchicago.edu)
 *
 * util_compress.h contains many small, simple, and useful functions which are
 * used by multiple minnow compresison algorithms. */

#include <stdint.h>
#include "seq.h"

/* util_BinIndex returns the bin indices of a sequence of floats, x, 
 * within the range [x0, x0 + dx) with bin width dx/(2^level[i]). A buffer
 * sequence may be passed to this function to prevent unneeded heap
 * allocations. You may not assume that a reference to this buffer continues
 * to exist after the end of this function call.
 * 
 * util_BinIndex will Panic if any element of x is outside the specified range
 * or if dx is negative.
 *
 * The BinIndex functions are the only steps in any Minnow encoding algorithm
 * which lose information. */
U32Seq util_BinIndex(FSeq x, U8Seq level, float x0, float dx, U32Seq buf);
/* util_UniformBinIndex is identical to util_BinIndex, but uses the same value
 * of level for every element of x. */
U32Seq util_UniformBinIndex(
    FSeq x, uint8_t level, float x0, float dx, U32Seq buf
);

/* util_UndoBinIndex reverses the results of a call to util_BinIndex. The
 * resultant floats will be uniformly distributed within their respective bins.
 * A buffer sequence may be passed to this function to prevent unneeded heap
 * allocations. You may not assume that a reference to this buffer continues
 * to exist after the end of this function call.
 *
 * The UndoBinIndex functions are the only steps in any Minnow decoding
 * algorithm which lose information. */
FSeq util_UndoBinIndex(
    U32Seq idx, U8Seq level, float x0, float dx, FSeq buf
);
FSeq util_UndoUniformBinIndex(
    U32Seq idx, uint8_t level, float x0, float dx, FSeq buf
);

/* utilU32TransposeBytes transforms an integer seqeunce into a byte sequence
 * where the 0th byte is the 0th byte of the 0th integer, the 1st byte is the
 * 0th byte of the 1st int, and so on. A buffer sequence may be passed to this
 * function to prevent unneeded heap allocations. You may not assume that a
 * reference to this buffer continues to exist after the end of this function
 * call. */
U8Seq util_U32TransposeBytes(U32Seq x, U8Seq buf);
/* util_U32TransposeBits is identical to util_U32TransposeBytes, except the
 * transposition occurs at the bit level, not the byte level. */
U8Seq util_U32TransposeBits(U32Seq x, U8Seq buf);

/* util_U32UndoTransposeBytes reverses the results of a call to
 * util_U8TransposeBytes. A buffer sequence may be passed to this function to
 * prevent unneeded heap allocations. You may not assume that a reference to
 * this buffer continues to exist after the end of this function call.*/
U32Seq util_U32UndoTransposeBytes(U8Seq x, U32Seq buf);
U32Seq util_U32UndoTransposeBits(U32Seq x, U8Seq buf);

/* util_U8DeltaEncode delta encodes a sequence of eight byte integers. A buffer
 * may be supplied to this function to prevent unneccessary heap allocations.
 * You may not assume that a refeence to this buffer continues to exist after
 * the end of this function call. Passing the same sequence to both arguments
 * will result in the calculation being done in place. */
U8Seq util_U8DeltaEncode(U8Seq x, U8Seq buf);
U16Seq util_U16DeltaEncode(U8Seq x, U8Seq buf);
U32Seq util_U32DeltaEncode(U8Seq x, U8Seq buf);
U64Seq util_U64DeltaEncode(U8Seq x, U8Seq buf);

/* util_U8UndoDeltaEncode reverses the results of a call to
 * util_U8DeltaEncode.  A buffer  may be supplied to this function to prevent
 * unneccessary heap allocations. You may not assume that a refeence to this
 * buffer continues to exist after the end of this function call. Passing the
 * same sequence to both arguments will result in the calculation being done in
 * place. */
U8Seq util_U8UndoDeltaEncode(U8Seq x, U8Seq buf);
U16Seq util_U16UndoDeltaEncode(U16Seq x, U8Seq buf);
U32Seq util_U32UndoDeltaEncode(U32Seq x, U8Seq buf);
U64Seq util_U64UndoDeltaEncode(U64Seq x, U8Seq buf);

#endif /* MNW_COMPRESS_UTIL_H_ */
