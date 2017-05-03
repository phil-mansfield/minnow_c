#include <stdlib.h>
#include <inttypes.h>

#include "register.h"
#include "funcs.h"
#include "quant.h"
#include "../src/debug.h"
#include "../src/util.h"
#include "../src/stream.h"
#include "../src/semver.h"

QSeg Quantize(Seg s) {
    QSeg qs;
    qs.FieldLen = s.FieldLen;
    qs.Fields = calloc((size_t)qs.FieldLen, sizeof(qs.Fields[0]));

    for (int32_t i = 0; i < qs.FieldLen; i++) {
        qs.Fields[i] = quant_QField(s.Fields[i]);
    }

    return qs;
}

Seg UndoQuantize(QSeg qs) {
    Seg s;
    s.FieldLen = qs.FieldLen;
    s.Fields = calloc((size_t)s.FieldLen, sizeof(s.Fields[0]));

    for (int32_t i = 0; i < s.FieldLen; i++) {
        s.Fields[i] = quant_Field(qs.Fields[i]);
    }

    return s;
}

QSeg Decompress(CSeg cs, Decompressor *decomps) {
    QSeg qs;
    qs.FieldLen = cs.FieldLen;
    qs.Fields = calloc((size_t)qs.FieldLen, sizeof(*qs.Fields));

    for (int32_t i = 0; i < qs.FieldLen; i++) {
        QField *qf = &qs.Fields[i];
        CField *cf = &cs.Fields[i];

        uint32_t checksum = util_Checksum(
            U8BigSeq_WrapArray(cf->Data, cf->DataLen)
        );
        if (checksum == cf->Checksum || decomps[i].NaNFlag) {
            *qf = decomps[i].DFunc(*cf, decomps[i].Buffer);
        } else {
            memcpy(&qf->Hd, &qf->Hd, sizeof(qf->Hd));
        }
    }

    return qs;
}

CSeg Compress(QSeg qs, Compressor *comps) {
    CSeg cs;
    cs.FieldLen = qs.FieldLen;
    cs.Fields = calloc((size_t)cs.FieldLen, sizeof(*cs.Fields));

    for (int32_t i = 0; i < cs.FieldLen; i++) {
        QField *qf = &qs.Fields[i];
        CField *cf = &cs.Fields[i];
        
        *cf = comps[i].CFunc(*qf, comps[i].Buffer);
        cf->Checksum = util_Checksum(U8BigSeq_WrapArray(cf->Data, cf->DataLen));
    }

    return cs;
}

U8BigSeq ToBytes(CSeg cs) {
    stream_Writer writer = stream_NewWriter();

    stream_Write(writer, &cs.FieldLen, 4, 4);

    for (int32_t i = 0; i < cs.FieldLen; i++) {
        CField f = cs.Fields[i];
        stream_Write(writer, &f.Hd, sizeof(f.Hd), 4);
        stream_Write(writer, &f.Checksum, 4, 4);
        stream_Write(writer, &f.DataLen, 4, 4);
    }

    for (int32_t i = 0; i < cs.FieldLen; i++) {
        stream_Write(
            writer, cs.Fields[i].Data, (size_t)cs.Fields[i].DataLen, 1
        );
    }

    return writer;
}

CSeg FromBytes(U8BigSeq bytes) {
    stream_Reader reader = stream_NewReader(bytes);
    CSeg cs;

    stream_Read(reader, &cs.FieldLen, 4, 4);
    cs.Fields = calloc((size_t)cs.FieldLen, sizeof(*cs.Fields));

    for (int32_t i = 0; i < cs.FieldLen; i++) {
        CField *f = &cs.Fields[i];
        stream_Read(reader, &f->Hd, sizeof(f->Hd), 4);
        stream_Read(reader, &f->Checksum, 4, 4);
        stream_Read(reader, &f->DataLen, 4, 4);
    }

    for (int32_t i = 0; i < cs.FieldLen; i++) {
        stream_Read(
            reader, cs.Fields[i].Data, (size_t)cs.Fields[i].DataLen, 1
        );
    }

    return cs;
}

Decompressor *LoadDecompressors(CSeg cs) {
    Register reg = Register_New();
    Decompressor *decomps = calloc((size_t) cs.FieldLen, sizeof(decomps[0]));

    for (int32_t i = 0; i < cs.FieldLen; i++) {
        uint32_t algo = cs.Fields[i].Hd.AlgoCode;
        uint32_t version = cs.Fields[i].Hd.AlgoVersion;

        if (!Register_Supports(reg, algo, version)) {
            Panic("v%d.%d of algorithm %x is not supported.",
                  semver_Major(version), semver_Minor(version), algo);
        }

        decomps[i] = Register_GetDecompressor(reg, algo, version);
    }

    Register_Free(reg);

    return decomps;
}

Compressor *LoadCompressors(Seg s) {
    Register reg = Register_New();
    Compressor *comps = calloc((size_t) s.FieldLen, sizeof(comps[0]));

    for (int32_t i = 0; i < s.FieldLen; i++) {
        uint32_t algo = s.Fields[i].Hd.AlgoCode;
        uint32_t version = s.Fields[i].Hd.AlgoVersion;

        if (!Register_Supports(reg, algo, version)) {
            Panic("v%d.%d of algorithm %x is not supported.",
                  semver_Major(version), semver_Minor(version), algo);
        }

        comps[i] = Register_GetCompressor(reg, algo, version);
    }

    Register_Free(reg);

    return comps;
}

void FreeDecompressors(CSeg cs, Decompressor *decomps) {
    Register reg = Register_New();

    for (int32_t i = 0; i < cs.FieldLen; i++) {
        uint32_t algo = cs.Fields[i].Hd.AlgoCode;
        uint32_t version = cs.Fields[i].Hd.AlgoVersion;
        Register_FreeDecompressor(reg, algo, version, decomps[i]);
    }

    free(decomps);
    Register_Free(reg);
}

void FreeCompressors(Seg s, Compressor *comps) {
    Register reg = Register_New();

    for (int32_t i = 0; i < s.FieldLen; i++) {
        uint32_t algo = s.Fields[i].Hd.AlgoCode;
        uint32_t version = s.Fields[i].Hd.AlgoVersion;
        Register_FreeCompressor(reg, algo, version, comps[i]);
    }

    free(comps);
    Register_Free(reg);
}

/* Note: this function will not free your data arrays. */
void Seg_Free(Seg s) {
    for (int32_t i = 0; i < s.FieldLen; i++) {
        quant_FreeField(s.Fields[i]);
    }
    free(s.Fields);
}

void QSeg_Free(QSeg qs) {
    for (int32_t i = 0; i < qs.FieldLen; i++) {
        quant_FreeQField(qs.Fields[i]);
    }
    free(qs.Fields);
}

void CSeg_Free(CSeg cs) {
    for (int32_t i = 0; i < cs.FieldLen; i++) {
        free(cs.Fields[i].Data);
    }
    free(cs.Fields);
}

Seg getSegment(void);
Seg getSegment(void) {
    Register reg = Register_New();

    float *x = NULL;
    float *v = NULL;
    uint64_t *id = NULL;
    int32_t len = 0;

    Seg s;
    s.FieldLen = 3;
    s.Fields = calloc(3, sizeof(s.Fields[0]));

    /* Position intialization. */

    /* Instead of using Register_Newest(), we could also use
     * semver_FromString(0.9.0-dev) or something similar if we want a specific
     * version. */
    FieldHeader xHd = { .FieldCode = field_Posn, .AlgoCode = algo_Test,
                        .AlgoVersion = Register_Newest(reg, algo_Test),
                        .ParticleLen = len };
    s.Fields[0].Hd = xHd;
    s.Fields[0].Data = x;
    PositionAccuracy xAcc = { .Deltas = NULL, .Delta = (float) 1e-3,
                              .BoxWidth = 64, .Len = 0 };
    s.Fields[0].Acc = calloc(1, sizeof(xAcc));
    *(PositionAccuracy*)s.Fields[0].Acc = xAcc;

    /* Velocity */

    FieldHeader vHd = { .FieldCode = field_Velc, .AlgoCode = algo_Test,
                        .AlgoVersion = Register_Newest(reg, algo_Test),
                        .ParticleLen = len };
    s.Fields[0].Hd = vHd;
    s.Fields[0].Data = v;
    VelocityAccuracy vAcc = { .Deltas = NULL, .Delta = 1,
                              .Len = 0, .SymLog10Threshold = false };
    s.Fields[1].Acc = calloc(1, sizeof(vAcc));
    *(VelocityAccuracy*)s.Fields[1].Acc = vAcc;

    /* ID */

    FieldHeader idHd = { .FieldCode = field_Ptid, .AlgoCode = algo_Test,
                         .AlgoVersion = Register_Newest(reg, algo_Test),
                         .ParticleLen = len };
    s.Fields[2].Hd = idHd;
    s.Fields[2].Data = id;
    IDAccuracy idAcc = { .Width = 1024 };
    s.Fields[2].Acc = calloc(1, sizeof(idAcc));
    *(IDAccuracy*)s.Fields[2].Acc = idAcc;

    Register_Free(reg);

    return s;
}

int main() {
    /* Load data and specify segments. */

    Seg s = getSegment();

    /* Compress */

    Register reg = Register_New();

    QSeg qs = Quantize(s);
    Compressor *comps = LoadCompressors(s);
    CSeg cs = Compress(qs, comps);
    U8BigSeq bytes = ToBytes(cs);

    Seg_Free(s);
    QSeg_Free(qs);

    /* Decompress */
    
    cs = FromBytes(bytes);
    Decompressor *decomps = LoadDecompressors(cs);
    qs = Decompress(cs, decomps);
    s = UndoQuantize(qs);

    CSeg_Free(cs);
    QSeg_Free(qs);
    Seg_Free(s);

    /* Shared cleanup */

    FreeCompressors(s, comps);
    FreeDecompressors(cs, decomps);
    Register_Free(reg);
}
