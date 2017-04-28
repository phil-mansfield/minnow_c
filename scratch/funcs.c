#include <stdlib.h>
#include <inttypes.h>

#include "register.h"
#include "funcs.h"
#include "quant.h"
#include "../src/debug.h"
#include "../src/util.h"
#include "../src/stream.h"
#include "../src/semver.h"

/* TODO: functions for freeing data. */

/* For now we're going to take the perspective that buffers are for nerds. */

Decompressor *LoadDecompressors(CSeg cs, Register reg) {
    Decompressor *decomps = calloc((size_t) cs.FieldLen, sizeof(decomps[0]));

    for (int32_t i = 0; i < cs.FieldLen; i++) {
        uint32_t algo = cs.Fields[i].Hd.AlgoCode;
        uint32_t version = cs.Fields[i].Hd.AlgoVersion;

        if (!Register_Supports(reg, algo, version)) {
            /* TODO: Improve this error message. */
            char versionStr[semver_BUF_SIZE];
            semver_ToString(version, versionStr);
            Panic("Version %s of algorithm %x is not supported.",
                  versionStr, algo);
        }

        decomps[i] = Register_GetDecompressor(reg, algo, version);
    }

    return decomps;
}

Compressor *LoadCompressors(Seg s, Register reg) {
    Compressor *comps = calloc((size_t) s.FieldLen, sizeof(comps[0]));

    for (int32_t i = 0; i < s.FieldLen; i++) {
        uint32_t algo = s.Fields[i].Hd.AlgoCode;
        uint32_t version = s.Fields[i].Hd.AlgoVersion;

        if (!Register_Supports(reg, algo, version)) {
            /* TODO: Improve this error message. */
            char versionStr[semver_BUF_SIZE];
            semver_ToString(version, versionStr);
            Panic("Version %s of algorithm %x is not supported.",
                  versionStr, algo);
        }

        comps[i] = Register_GetCompressor(reg, algo, version);
    }

    return comps;
}

void FreeDecompressors(CSeg cs, Register reg, Decompressor *decomps) {
    for (int32_t i = 0; i < cs.FieldLen; i++) {
        uint32_t algo = cs.Fields[i].Hd.AlgoCode;
        uint32_t version = cs.Fields[i].Hd.AlgoVersion;
        Register_FreeDecompressor(reg, algo, version, decomps[i]);
    }

    free(decomps);
}

void FreeCompressors(Seg s, Register reg, Compressor *comps) {
    for (int32_t i = 0; i < s.FieldLen; i++) {
        uint32_t algo = s.Fields[i].Hd.AlgoCode;
        uint32_t version = s.Fields[i].Hd.AlgoVersion;
        Register_FreeCompressor(reg, algo, version, comps[i]);
    }

    free(comps);
}

QSeg Quantize(Seg s) {
    QSeg qs;
    qs.FieldLen = s.FieldLen;
    qs.Fields = calloc((size_t)qs.FieldLen, sizeof(qs.Fields[0]));

    for (int32_t i = 0; i < qs.FieldLen; i++) {
        Field *f = &s.Fields[i];
        QField *qf = &qs.Fields[i];

        switch(f->Hd.FieldCode) {
        case field_Posn:
            *qf = quant_Position(f->Data, f->Acc, f->Hd.ParticleLen);
            break;
        case field_Velc:
            *qf = quant_Velocity(f->Data, f->Acc, f->Hd.ParticleLen);
            break;
        case field_Ptid:
            *qf = quant_ID(f->Data, f->Acc, f->Hd.ParticleLen);
            break;
        case field_Unsf:
            *qf = quant_Float(f->Data, f->Acc, f->Hd.ParticleLen);
            break;
        case field_Unsi:
            *qf = quant_Int(f->Data, f->Acc, f->Hd.ParticleLen);
            break;
        default:
            Panic("Field %"PRId32" has unrecognized field code %"PRIx32".",
                  i, f->Hd.FieldCode);
        }
    }

    return qs;
}

Seg UndoQuantize(QSeg qs) {
    Seg s;
    s.FieldLen = qs.FieldLen;
    s.Fields = calloc((size_t)s.FieldLen, sizeof(s.Fields[0]));

    for (int32_t i = 0; i < s.FieldLen; i++) {
        Field *f = &s.Fields[i];
        QField *qf = &qs.Fields[i];

        switch(qf->Hd.FieldCode) {
        case field_Posn:
            *f = quant_UndoPosition(qf->Data, qf->Quant, qf->Hd.ParticleLen);
            break;
        case field_Velc:
            *f = quant_UndoVelocity(qf->Data, qf->Quant, qf->Hd.ParticleLen);
            break;
        case field_Ptid:
            *f = quant_UndoID(qf->Data, qf->Quant, qf->Hd.ParticleLen);
            break;
        case field_Unsf:
            *f = quant_UndoFloat(qf->Data, qf->Quant, qf->Hd.ParticleLen);
            break;
        case field_Unsi:
            *f = quant_UndoInt(qf->Data, qf->Quant, qf->Hd.ParticleLen);
            break;
        default:
            Panic("Field %"PRId32" has unrecognized field code %"PRIx32".",
                  i, f->Hd.FieldCode);
        }
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
            U8Seq_WrapArray(cf->Data, cf->DataLen)
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
        cf->Checksum = util_Checksum(U8Seq_WrapArray(cf->Data, cf->DataLen));
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

Seg getSegment(Register reg);
Seg getSegment(Register reg) {
   float *x = NULL;
    float *v = NULL;
    uint64_t *id = NULL;
    int32_t len = 0;

    Seg s;
    s.FieldLen = 3;
    s.Fields = calloc(3, sizeof(s.Fields[0]));

    /* Position intialization. */

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
                              .Len = 0, .SymLogThreshold = false };
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

    return s;
}

int main() {
    /* Specify data. */

    Register reg = Register_New();
    Seg s = getSegment(reg);

    /* Compress */

    QSeg qs = Quantize(s);
    Compressor *comps = LoadCompressors(s, reg);
    CSeg cs = Compress(qs, comps);
    U8BigSeq bytes = ToBytes(cs);

    /* Do whatever you want with bytes. */
    
    cs = FromBytes(bytes);
    Decompressor *decomps = LoadDecompressors(cs, reg);
    qs = Decompress(cs, decomps);
    s = UndoQuantize(qs);

    /* Clean up */

    FreeCompressors(s, reg, comps);
    FreeDecompressors(cs, reg, decomps);
    Register_Free(reg);
}
