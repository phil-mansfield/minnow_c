#include <stdlib.h>

#include "types.h"
#include "../src/seq.h"
#include "../src/debug.h"
#include "../src/compress_util.h"

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
    QSeg qs;
    qs.FieldLen = cs.FieldLen;
    qs.Fields = calloc((size_t)qs.FieldLen, sizeof(*qs.Fields));

    for (int32_t i = 0; i < qs.FieldLen; i++) {
        uint32_t checksum = util_Checksum(
            U8Seq_WrapArray(cs.Fields[i].Data, cs.Fields[i].DataLen)
        );
        if (checksum == cs.Fields[i].Checksum || decomps[i].NaNFlag) {
            qs.Fields[i] = decomps[i].DFunc(cs.Fields[i], decomps[i].Buffer);
        } else {
            /* QField.Data will be left NULL, so all values will be
             * dequantized to NaN.*/
            memcpy(&qs.Fields[i].Hd, &qs.Fields[i].Hd, sizeof(qs.Fields[i].Hd));
        }
    }

    return qs;
}

CSeg Compress(QSeg qs, Compressor *comps) {
    CSeg cs;
    cs.FieldLen = qs.FieldLen;
    cs.Fields = calloc((size_t)cs.FieldLen, sizeof(*cs.Fields));

    for (int32_t i = 0; i < cs.FieldLen; i++) {
        cs.Fields[i] = comps[i].CFunc(qs.Fields[i], comps[i].Buffer);
        cs.Fields[i].Checksum = util_Checksum(
            U8Seq_WrapArray(cs.Fields[i].Data, cs.Fields[i].DataLen)
        );
    }

    return cs;
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
