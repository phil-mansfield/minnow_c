#ifndef MNW_COMPRESS_UTIL_H_
#define MNW_COMPRESS_UTIL_H_

/* Author: Phil Mansfield (mansfield@uchicago.edu)
 *
 * util_compress.h contains many small, simple, and useful functions which are
 * used by multiple minnow compresison algorithms. */

#include <stdint.h>
#include "seq.h"
#include "rand.h"

/* util_MinMax computes the minimum and maximum of a sequence. */
void util_MinMax(FSeq x, float *minPtr, float *maxPtr);
void util_U32MinMax(U32Seq x, uint32_t *minPtr, uint32_t *maxPtr);

/* util_Periodic applies periodic boundary conditions of length L to a
 * sequence. It assumes that all points are no more than a distance L outside
 * of the range. */
void util_Periodic(FSeq x, float L);
void util_U32Periodic(U32Seq x, uint32_t L);

/* util_UndoPeriodic reverses a call to Periodic so that all all values are 
 * within a contiguous range. */
void util_UndoPeriodic(FSeq x, float L);
void util_U32UndoPeriodic(U32Seq x, uint32_t L);

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
    U32Seq idx, U8Seq level, float x0, float dx, rand_State *state, FSeq buf
);
FSeq util_UndoUniformBinIndex(
    U32Seq idx, uint8_t level, float x0, float dx, rand_State *state, FSeq buf
);

/* utilU32TransposeBytes transforms an integer seqeunce into a byte sequence
 * where the 0th byte is the 0th byte of the 0th integer, the 1st byte is the
 * 0th byte of the 1st int, and so on. A buffer sequence may be passed to this
 * function to prevent unneeded heap allocations. You may not assume that a
 * reference to this buffer continues to exist after the end of this function
 * call. */
U8Seq util_U32TransposeBytes(U32Seq x, U8Seq buf);

/* util_U32UndoTransposeBytes reverses the results of a call to
 * util_U8TransposeBytes. A buffer sequence may be passed to this function to
 * prevent unneeded heap allocations. You may not assume that a reference to
 * this buffer continues to exist after the end of this function call.*/
U32Seq util_U32UndoTransposeBytes(U8Seq x, U32Seq buf);

/* util_U8DeltaEncode delta encodes a sequence of eight byte integers. A buffer
 * may be supplied to this function to prevent unneccessary heap allocations.
 * You may not assume that a reference to this buffer continues to exist after
 * the end of this function call. Passing the same sequence to both arguments
 * will result in the calculation being done in place. */
U8Seq util_U8DeltaEncode(U8Seq x, U8Seq buf);

/* util_U8UndoDeltaEncode reverses the results of a call to
 * util_U8DeltaEncode. A buffer may be supplied to this function to prevent
 * unneccessary heap allocations. You may not assume that a reference to this
 * buffer continues to exist after the end of this function call. Passing the
 * same sequence to both arguments will result in the calculation being done in
 * place. */
U8Seq util_U8UndoDeltaEncode(U8Seq x, U8Seq buf);

/* util_u32UniformPack stores the least significant bits of a seqeunce of
 * integers in contiguous order. The number of bits stored per integer is
 * given by width. Any extra bits in the output byte sequence will be set to
 * zero. A buffer may be supplied to this function to prevent
 * unneccessary heap allocations. You may not assume that a reference to this
 * buffer continues to exist after the end of this function call.*/
U32Seq util_U32UniformPack(U32Seq x, uint8_t width, U32Seq buf);

/* util_U32UndoUniformPack reverses the results of a call to
 * util_U32UniformPack. The number of elements in the original seqeunce is
 * given by len. A buffer may be supplied to this function to prevent
 * unneccessary heap allocations. You may not assume that a reference to this
 * buffer continues to exist after the end of this function call. */
U32Seq util_U32UndoUniformPack(
    U32Seq x, uint8_t width, int32_t len, U32Seq buf
);

/* util_EntropyEncode will apply an (unspecified) entropy encoding scheme to
 * stream of data.  */
U8Seq util_EntropyEncode(U8Seq data, U8Seq buf);

/* util_UndoEntropyEncode reverses a call to util_EntropyEncode. It must be
 * passed the original size of the uncompressed data sequence. */
U8Seq util_UndoEntropyEncode(
    U8Seq compressedData, int32_t uncompressedSize, U8Seq buf
);

#endif /* MNW_COMPRESS_UTIL_H_ */
