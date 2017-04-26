#include "types.h"
#include "../src/seq.h"

/* For now we're going to take the perspective that buffers are for nerds. */

QSeg Quantize(Seg s) {
    (void) s;
    
    QSeg dummy;
    return dummy;
}

Seg UndoQuantize(QSeg qs) {
    (void) qs;

    Seg s;
    return s;
}

QSeg Decompress(CSeg cs, Decompressor *decomps) {
    (void) cs;
    (void) decomps;
    
    QSeg dummy;
    return dummy;
}

CSeg Compress(QSeg qs, Compressor *comps) {
    
}

U8BigSeq ToBytes(CSeg cs) {
    (void) cs;

    U8BigSeq dummy;
    return dummy;
}

CSeg FromBytes(U8BigSeq bytes) {
    (void) bytes;
    
    CSeg dummy;
    return dummy;
}

int main() {
    return 0;
}
