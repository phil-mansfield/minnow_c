#ifndef FUNCS_H_
#define FUNCS_H_

#include "types.h"
#include "../src/seq.h"

QSeg Quantize(Seg s);
Seg UndoQuantize(QSeg qs);

QSeg Decompress(CSeg cs, Decompressor *decomps);
CSeg Compress(QSeg qs, Compressor *comps);

U8BigSeq ToBytes(CSeg cs);
CSeg FromBytes(U8BigSeq bytes);

#endif
