#ifndef FUNCS_H_
#define FUNCS_H_

#include "types.h"
#include "../src/seq.h"

/* TODO: Figure out whether or not vectorization here is corect. */
Decompressor *LoadDecompressors(CSeg cs, Register reg);
void FreeDecompressors(CSeg cs, Register reg, Decompressor *decomps);

Compressor *LoadCompressors(Seg qs, Register reg);
void FreeCompressors(Seg s, Register reg, Compressor *comps);

QSeg Quantize(Seg s);
Seg UndoQuantize(QSeg qs);

QSeg Decompress(CSeg cs, Decompressor *decomps);
CSeg Compress(QSeg qs, Compressor *comps);

U8BigSeq ToBytes(CSeg cs);
CSeg FromBytes(U8BigSeq bytes);

#endif
