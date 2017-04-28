#ifndef FUNCS_H_
#define FUNCS_H_

#include "types.h"
#include "../src/seq.h"

/* TODO: Figure out whether or not vectorization here is the
 * corect API choice. */
Decompressor *LoadDecompressors(CSeg cs);
void FreeDecompressors(CSeg cs, Decompressor *decomps);

Compressor *LoadCompressors(Seg qs);
void FreeCompressors(Seg s, Compressor *comps);

QSeg Quantize(Seg s);
Seg UndoQuantize(QSeg qs);

QSeg Decompress(CSeg cs, Decompressor *decomps);
CSeg Compress(QSeg qs, Compressor *comps);

U8BigSeq ToBytes(CSeg cs);
CSeg FromBytes(U8BigSeq bytes);

/* Note that Seg_Free will not free your data arrays. */
void Seg_Free(Seg s);
void QSeg_Free(QSeg qs);
void CSeg_Free(CSeg cs);


#endif
