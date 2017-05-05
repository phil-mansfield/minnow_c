#include <inttypes.h>
#include <math.h>
#include <time.h>

#include "quant.h"
#include "debug.h"
#include "rand.h"
#include "seq.h"
#include "util.h"

/************************/
/* forward declarations */
/************************/

QField position(Field f);
Field undoPosition(QField qf);
QField velocity(Field f);
Field undoVelocity(QField qf);
QField id(Field f);
Field undoID(QField qf);
QField ufloat(Field f);
Field undoUfloat(QField qf);
QField uint(Field f);
Field undoUint(QField qf);

void undoLog10Float(
    float x0, float x1,
    uint8_t depth, uint8_t *depths,
    uint64_t *qdata, float *buf, int32_t len
);

void undoSymLog10Float(
    float x0, float x1, float symLogThreshold,
    uint8_t depth, uint8_t *depths,
    uint64_t *qdata, float *buf, int32_t len
);

void undoFloat(
    float x0, float x1,
    uint8_t depth, uint8_t *depths,
    uint64_t *qdata, float *buf, int32_t len
);

void depthToDelta(
    uint8_t depth, uint8_t *depths,
    float x0, float x1,
    float *deltaPtr, float**deltasPtr,
    int32_t len
);

void deltaToDepth(
    float delta, float *deltas,
    float x0, float x1,
    uint8_t *depthPtr, uint8_t **depthsPtr,
    int32_t len
);

FSeq mapFloat(FSeq data, int32_t log10Scaled, float symLog10Threshold);
void unmapFloat(FSeq map, int32_t log10Scaled);

/******************************/
/* dynamic dispatch functions */
/******************************/

void quant_FreeQField(QField qf) {
    switch(qf.Hd.FieldCode) {

    case field_Posn:
        free(((PositionQuantization*)qf.Quant)->Depths);
        free(qf.Quant);
        free(qf.Data);
        return;

    case field_Velc:
        free(((VelocityQuantization*)qf.Quant)->Depths);
        free(qf.Quant);
        free(qf.Data);
        return;

    case field_Ptid:
        free(qf.Quant);
        free(qf.Data);
        return;

    case field_Unsf:
        free(((FloatQuantization*)qf.Quant)->Depths);
        free(qf.Quant);
        free(qf.Data);
        return;

    case field_Unsi:
        free(qf.Quant);
        free(qf.Data);
        return;

    default: Panic("Unrecognized field code %"PRIx32".", qf.Hd.FieldCode);
    }
}

void quant_FreeField(Field f) {
    switch(f.Hd.FieldCode) {

    case field_Posn:
        free(((PositionAccuracy*)f.Acc)->Deltas);
        free(f.Acc);
        free(f.Data);
        return;

    case field_Velc:
        free(((VelocityAccuracy*)f.Acc)->Deltas);
        free(f.Acc);
        free(f.Data);
        return;

    case field_Ptid:
        free(f.Acc);
        free(f.Data);
        return;

    case field_Unsf:
        free(((FloatAccuracy*)f.Acc)->Deltas);
        free(f.Acc);
        free(f.Data);
        return;

    case field_Unsi:
        free(f.Acc);
        free(f.Data);
        return;

    default: Panic("Unrecognized field code %"PRIx32".", f.Hd.FieldCode);
    }
}

QField quant_QField(Field f) {
    switch(f.Hd.FieldCode) {
    case field_Posn: return position(f);
    case field_Velc: return velocity(f);
    case field_Ptid: return id(f);
    case field_Unsf: return ufloat(f);
    case field_Unsi: return uint(f);
    default: Panic("Unrecognized field code %"PRIx32".", f.Hd.FieldCode);
    }
}

Field quant_Field(QField qf) {
    switch(qf.Hd.FieldCode) {
    case field_Posn: return undoPosition(qf);
    case field_Velc: return undoVelocity(qf);
    case field_Ptid: return undoID(qf);
    case field_Unsf: return undoUfloat(qf);
    case field_Unsi: return undoUint(qf);
    default: Panic("Unrecognized field code %"PRIx32".", qf.Hd.FieldCode);
    }
}

/**************************/
/* quantization funcitons */
/**************************/

QField position(Field f) {
    /* Set things up. */
    QField qf;
    memset(&qf, 0, sizeof(f));
    memcpy(&qf.Hd, &f.Hd, sizeof(f.Hd));

    int32_t len = f.Hd.ParticleLen;
    PositionAccuracy *acc = f.Acc;
    PositionQuantization *quant = calloc(1, sizeof(*quant));
    FSeq x = FSeq_WrapArray((float*)f.Data, len);
    FSeq y = FSeq_WrapArray((float*)f.Data + len, len);
    FSeq z = FSeq_WrapArray((float*)f.Data + 2*(size_t)len, len);
    uint64_t *qdata = calloc((size_t)len, sizeof(*qdata));
    U64Seq qx = U64Seq_WrapArray(qdata, len);
    U64Seq qy = U64Seq_WrapArray(qdata + len, len);
    U64Seq qz = U64Seq_WrapArray(qdata + 2*(size_t)len, len);
    FSeq xDim[3] = { x, y, z };
    U64Seq qDim[3] = { qx, qy, qz };

    /* Quantize */
    float maxDiff = 0;
    for (int i = 0; i < 3; i++) {
        FSeq buf = FSeq_New(len);
        memcpy(buf.Data, xDim[i].Data, sizeof(buf.Data)*(size_t)len);
        util_UndoPeriodic(buf, acc->Width);
        xDim[i] = buf;

        util_MinMax(xDim[i], &quant->X0[i], &quant->X1[i]);
        if (maxDiff < quant->X1[i] - quant->X0[i]) {
            maxDiff = quant->X1[i] - quant->X0[i];
        }
    }

    uint8_t depth, *depths;
    deltaToDepth(acc->Delta, acc->Deltas, quant->X0[0],
                 quant->X0[0] + maxDiff, &depth, &depths, len);

    for (int i = 0; i < 3; i++) {
        if (depths == NULL) {
            qDim[i] = util_UniformBinIndex(
                xDim[i], depth, quant->X0[i], maxDiff, qDim[i]
            );
        } else {
            U8Seq depthsSeq = U8Seq_WrapArray(depths, len);
            qDim[i] = util_BinIndex(
                xDim[i], depthsSeq, quant->X0[i], maxDiff, qDim[i]
            );
        }
    }

    /* Initialize  */
    quant->Depths = depths;
    quant->Depth = depth;
    quant->Len = acc->Len;
    quant->Width = acc->Width;

    /* Clean up */
    for (int i = 0; i < 3; i++) {
        FSeq_Free(xDim[i]);
    }

    qf.Data = qdata;
    return qf;
}

QField velocity(Field f) {
    /* Set things up. */
    QField qf;
    memset(&qf, 0, sizeof(f));
    memcpy(&qf.Hd, &f.Hd, sizeof(f.Hd));

    int32_t len = f.Hd.ParticleLen;
    VelocityAccuracy *acc = f.Acc;
    VelocityQuantization *quant = calloc(1, sizeof(*quant));
    FSeq vx = FSeq_WrapArray((float*)f.Data, len);
    FSeq vy = FSeq_WrapArray((float*)f.Data + len, len);
    FSeq vz = FSeq_WrapArray((float*)f.Data + 2*(size_t)len, len);
    uint64_t *qdata = calloc((size_t)len, sizeof(*qdata));
    U64Seq qx = U64Seq_WrapArray(qdata, len);
    U64Seq qy = U64Seq_WrapArray(qdata + len, len);
    U64Seq qz = U64Seq_WrapArray(qdata + 2*(size_t)len, len);
    FSeq vDim[3] = { vx, vy, vz };
    U64Seq qDim[3] = { qx, qy, qz };

    /* Quantize */
    float maxDiff = 0;
    int32_t flag = 0;
    if (acc->SymLog10Scaled) { flag = 2; }
    for (int i = 0; i < 3; i++) {
        vDim[i] = mapFloat(vDim[i], flag, acc->SymLog10Threshold);

        util_MinMax(vDim[i], &quant->X0[i], &quant->X1[i]);
        if (maxDiff < quant->X1[i] - quant->X0[i]) {
            maxDiff = quant->X1[i] - quant->X0[i];
        }
    }

    uint8_t depth, *depths;
    deltaToDepth(acc->Delta, acc->Deltas, quant->X0[0],
                 quant->X0[0] + maxDiff, &depth, &depths, len);

    for (int i = 0; i < 3; i++) {
        if (depths == NULL) {
            qDim[i] = util_UniformBinIndex(
                vDim[i], depth, quant->X0[i], maxDiff, qDim[i]
            );
        } else {
            U8Seq depthsSeq = U8Seq_WrapArray(depths, len);
            qDim[i] = util_BinIndex(
                vDim[i], depthsSeq, quant->X0[i], maxDiff, qDim[i]
            );
        }
    }

    /* Initialize  */
    quant->Depths = depths;
    quant->Depth = depth;
    quant->Len = acc->Len;
    quant->SymLog10Threshold = acc->SymLog10Threshold;
    quant->SymLog10Scaled = acc->SymLog10Scaled;

    /* Clean up */
    for (int i = 0; i < 3; i++) {
        unmapFloat(vDim[i], flag);
    }

    qf.Data = qdata;
    return qf;
}

QField id(Field f) {
    /* Set things up. */
    QField qf;
    memset(&qf, 0, sizeof(f));
    memcpy(&qf.Hd, &f.Hd, sizeof(f.Hd));

    int32_t len = f.Hd.ParticleLen;
    IDAccuracy *acc = f.Acc;
    IDQuantization *quant = calloc(1, sizeof(*quant));
    uint64_t *data = f.Data;
    uint64_t *qdata = calloc(3 * (size_t) len, sizeof(*qdata));
    U64Seq qx = U64Seq_WrapArray(qdata, len);
    U64Seq qy = U64Seq_WrapArray(qdata + len, len);
    U64Seq qz = U64Seq_WrapArray(qdata + 2*(size_t)len, len);
    U64Seq qDim[3] = { qx, qy, qz };

    /* Quantize */
    for (int32_t i = 0; i < len; i++) {
        qx.Data[i] = data[i] % acc->Width;
        qy.Data[i] = (data[i] / acc->Width) % acc->Width;
        qz.Data[i] = data[i] / (acc->Width * acc->Width);
    }

    for (int j = 0; j < 3; j++) {
        util_U64UndoPeriodic(qDim[j], acc->Width);
        util_U64MinMax(qDim[j], &quant->X0[j], &quant->X1[j]);
        for (int32_t i = 0; i < len; i++) {
            qDim[j].Data[i] -= quant->X0[j];
        }
    }

    /* Initialize  */
    quant->Width = acc->Width;

    qf.Data = qdata;
    return qf;
}

QField ufloat(Field f) {
    /* Set things up. */
    QField qf;
    memset(&qf, 0, sizeof(f));
    memcpy(&qf.Hd, &f.Hd, sizeof(f.Hd));

    int32_t len = f.Hd.ParticleLen;
    FloatAccuracy *acc = f.Acc;
    FloatQuantization *quant = calloc(1, sizeof(*quant));
    FSeq data = FSeq_WrapArray(f.Data, len);
    U64Seq qdata = U64Seq_New(len);

    data = mapFloat(data, acc->Log10Scaled, acc->SymLog10Threshold);

    /* Quantize */
    float x0, x1;
    util_MinMax(data, &x0, &x1);

    uint8_t depth, *depths;
    deltaToDepth(acc->Delta, acc->Deltas, x0, x1, &depth, &depths, len);

    if (depths == NULL) {
        qdata = util_UniformBinIndex(data, depth, x0, x1 - x0, qdata);
    } else {
        U8Seq depthsSeq = U8Seq_WrapArray(depths, len);
        qdata = util_BinIndex(data, depthsSeq, x0, x1 - x0, qdata);
    }

    /* Initialize  */
    quant->X0 = x0;
    quant->X1 = x1;
    quant->Depths = depths;
    quant->Depth = depth;
    quant->Len = acc->Len;
    quant->SymLog10Threshold = acc->SymLog10Threshold;
    quant->Log10Scaled = acc->Log10Scaled;

    /* Clean up */
    unmapFloat(data, acc->Log10Scaled);

    qf.Data = qdata.Data;
    return qf;
}

QField uint(Field f) {    
    /* Set things up. */
    QField qf;
    memset(&qf, 0, sizeof(f));
    memcpy(&qf.Hd, &f.Hd, sizeof(f.Hd));

    int32_t len = f.Hd.ParticleLen;
    IntAccuracy *acc = f.Acc;
    IntQuantization *quant = calloc(1, sizeof(*quant));
    uint64_t *data = f.Data;
    uint64_t *qdata = calloc((size_t) len, sizeof(*qdata));

    /* Quantize */
    uint64_t x0, x1;
    util_U64MinMax(U64Seq_WrapArray(data, len), &x0, &x1);
    memcpy(qdata, data, sizeof(*data) * (size_t)len);
    for (int32_t i = 0; i < len; i++) { qdata[i] -= x0; }

    /* Initialize  */
    (void) acc;
    quant->X0 = x0;
    quant->X1 = x1;

    qf.Data = qdata;
    return qf;
}


/****************************/
/* dequantization functions */
/****************************/

Field undoUfloat(QField qf) {
    /* Set things up. */
    Field f;
    memset(&f, 0, sizeof(f));
    memcpy(&f.Hd, &qf.Hd, sizeof(f.Hd));

    int32_t len = f.Hd.ParticleLen;
    FloatAccuracy *acc = calloc(1, sizeof(*acc));
    FloatQuantization quant = *(FloatQuantization*)qf.Quant;
    uint64_t *qdata = (uint64_t*)qf.Data;
    float *data = calloc((size_t) len, sizeof(*data));
    
    /* Dequantize data. */
    if (quant.Log10Scaled == 1) {
        undoLog10Float(
            quant.X0, quant.X1, quant.Depth,
            quant.Depths, qdata, data, len
        );
    } else if (quant.Log10Scaled == 2) {
        undoSymLog10Float(
            quant.X0, quant.X1, quant.SymLog10Threshold,
            quant.Depth, quant.Depths, qdata, data, len
        );
    } else {
        undoFloat(
            quant.X0, quant.X1, quant.Depth, quant.Depths,
            qdata, data, f.Hd.ParticleLen
        );
    }

    f.Data = data;

    /* Set Acc. */
    acc->SymLog10Threshold = quant.SymLog10Threshold;
    acc->Len = quant.Len;
    acc->Log10Scaled = quant.Log10Scaled;
    depthToDelta(
        quant.Depth, quant.Depths, quant.X0,
        quant.X1, &acc->Delta, &acc->Deltas, len
    );

    f.Acc = acc;
    
    return f;
}

Field undoPosition(QField qf) {
    /* Set things up. */
    Field f;
    memset(&f, 0, sizeof(f));
    memcpy(&f.Hd, &qf.Hd, sizeof(f.Hd));
    
    int32_t len = f.Hd.ParticleLen;
    PositionAccuracy *acc = calloc(1, sizeof(*acc));
    PositionQuantization quant = *(PositionQuantization*)qf.Quant;
    uint64_t *qdata = (uint64_t*)qf.Data;
    float *data = calloc(3*(size_t)len, sizeof(*data));
    
    /* Dequantize data. */
    float *xData = data;
    float *yData = data + len;
    float *zData = data + 2 * (size_t) len;
    float *dimData[3] = {xData, yData, zData}; 

    float maxDiff = 0;
    for (int i = 0; i < 3; i++) {
        if (quant.X1[i] - quant.X0[i] > maxDiff) {
            maxDiff = quant.X1[i] - quant.X0[i];
        }
    }

    for (int i = 0; i < 3; i++) {
        undoFloat(
            quant.X0[i], quant.X1[i], quant.Depth,
            quant.Depths, qdata, dimData[i], len
        );
        FSeq dataSeq = FSeq_WrapArray(dimData[i], len);
        util_Periodic(dataSeq, quant.Width);
    }

    f.Data = data;

    /* Set Acc. */
    acc->Len = quant.Len;
    acc->Width = quant.Width;
    depthToDelta(
        quant.Depth, quant.Depths, quant.X0[0],
        quant.X1[0], &acc->Delta, &acc->Deltas, len
    );
    f.Acc = acc;

    return f;
}

Field undoVelocity(QField qf) {
    /* Set things up. */
    Field f;
    memset(&f, 0, sizeof(f));
    memcpy(&f.Hd, &qf.Hd, sizeof(f.Hd));

    int32_t len = f.Hd.ParticleLen;
    VelocityAccuracy *acc = calloc(1, sizeof(*acc));
    VelocityQuantization quant = *(VelocityQuantization*)qf.Quant;
    uint64_t *qdata = (uint64_t*)qf.Data;
    float *data = calloc(3 * (size_t)len, sizeof(*data));
    
    /* Dequantize data. */
    float *xData = data;
    float *yData = data + len;
    float *zData = data + 2*(size_t)len;
    float *dimData[3] = {xData, yData, zData}; 

    float maxDiff = 0;
    for (int i = 0; i < 3; i++) {
        if (quant.X1[i] - quant.X0[i] > maxDiff) {
            maxDiff = quant.X1[i] - quant.X0[i];
        }
    }

    for (int i = 0; i < 3; i++) {
        if (quant.SymLog10Scaled) {
            undoSymLog10Float(
                quant.X0[i], quant.X0[i] + maxDiff, quant.SymLog10Threshold,
                quant.Depth, quant.Depths, qdata, dimData[i], len
            );
        } else {
            undoFloat(
                quant.X0[i], quant.X0[i] + maxDiff, quant.Depth,
                quant.Depths, qdata, dimData[i], len
            );
        }
    }

    f.Data = data;

    /* Set Acc. */
    acc->SymLog10Threshold = quant.SymLog10Threshold;
    acc->Len = quant.Len;
    acc->SymLog10Scaled = quant.SymLog10Scaled;
    depthToDelta(
        quant.Depth, quant.Depths, quant.X0[0],
        quant.X1[0], &acc->Delta, &acc->Deltas, len
    );
    f.Acc = acc;

    return f;
}

Field undoID(QField qf) {
    /* Set things up. */
    Field f;
    memset(&f, 0, sizeof(f));
    memcpy(&f.Hd, &qf.Hd, sizeof(f.Hd));

    int32_t len = f.Hd.ParticleLen;
    IDQuantization quant = *(IDQuantization*)qf.Quant;
    IDAccuracy *acc = calloc(1, sizeof(IDAccuracy));
    uint64_t *data = malloc(sizeof(*data)*(size_t)len);
    uint64_t *qdata = (uint64_t*)qf.Data;
    
    /* Dequantize data. */
    uint64_t *qdata0 = qdata;
    uint64_t *qdata1 = qdata0 + f.Hd.ParticleLen;
    uint64_t *qdata2 = qdata1 + f.Hd.ParticleLen;

    uint64_t w = quant.Width;
    for (int32_t i = 0; i < len; i++) {
        uint64_t x = qdata0[i] + quant.X0[0];
        if (x >= quant.Width) x -= quant.Width;
        uint64_t y = qdata1[i] + quant.X0[1];
        if (y >= quant.Width) y -= quant.Width;
        uint64_t z = qdata2[i] + quant.X0[2];
        if (z >= quant.Width) z -= quant.Width;
        data[i] = x + w*y + w*w*z;
    }

    /* Set Acc. */
    acc->Width = quant.Width;
    f.Acc = acc;

    f.Data = data;
    return f;
}

Field undoUint(QField qf) {
    /* Set things up. */
    Field f;
    memset(&f, 0, sizeof(f));
    memcpy(&f.Hd, &qf.Hd, sizeof(f.Hd));

    int32_t len = f.Hd.ParticleLen;
    IntQuantization quant = *(IntQuantization*)qf.Quant;
    uint64_t *qdata = (uint64_t*)qf.Data;
    uint64_t *data = malloc(sizeof(*data)*(size_t)len);
    
    /* Dequantize data. */
    memcpy(data, qdata, sizeof(*data)*(size_t)len);
    for (int32_t i = 0; i < len; i++) { data[i] += quant.X0; }
    f.Data = data;

    /* No need to set Acc. */

    return f;
}

/********************/
/* Helper Functions */
/********************/

void undoLog10Float(
    float x0, float x1,
    uint8_t depth, uint8_t *depths,
    uint64_t *qdata, float *buf, int32_t len
) {
    undoFloat(x0, x1, depth, depths, qdata, buf, len);
    for (int32_t i = 0; i < len; i++) { buf[i] = powf(10, buf[i]); }
}

void undoSymLog10Float(
    float x0, float x1, float symLog10Threshold,
    uint8_t depth, uint8_t *depths,
    uint64_t *qdata, float *buf, int32_t len
) {
    (void) symLog10Threshold;
    undoFloat(x0, x1, depth, depths, qdata, buf, len);
     
    Panic("SymLog10 not yet implemented.%s", "");
}

void undoFloat(
    float x0, float x1,
    uint8_t depth, uint8_t *depths,
    uint64_t *qdata, float *buf, int32_t len
) {
    rand_State *state = rand_Seed(clock(), 1);

    if (!depths) {
        float dx = (x1 - x0) / (float) (1<<depth);
        for (int32_t i = 0; i < len; i++) {
            buf[i] = x0 + dx*((float)qdata[i] + rand_Float(state));
        }
    } else {
        for (int32_t i = 0; i < len; i++) {
            float dx = (x1 - x0) / (float) (1 << depths[i]);
            buf[i] = x0 + dx*((float)qdata[i] + rand_Float(state));
        }
    }
}

void depthToDelta(
    uint8_t depth, uint8_t *depths,
    float x0, float x1,
    float *deltaPtr, float**deltasPtr,
    int32_t len
) {
    if (!depths) {
        *deltaPtr = (x1 - x0) / (float) (1 << depth);
        *deltasPtr = NULL;
        return;
    }

    float *deltas = calloc((size_t) len, sizeof(*deltas));
    for (int32_t i = 0; i < len; i++) {
        deltas[i] = (x1 - x0) / (float) (1 << depths[i]);
    }

    *deltasPtr = deltas;
    *deltaPtr = 0;
}

void deltaToDepth(
    float delta, float *deltas,
    float x0, float x1,
    uint8_t *depthPtr, uint8_t **depthsPtr,
    int32_t len
) {
    if (deltas == NULL) {

        uint8_t depth;
        for (depth = 0; depth <= 24; depth++) {
            if ((delta*(float)(1 << depth) > x1 - x0)) { break; }
        }

        if (depth > 24) {
            Panic("An accuracy of %g was requested for a variable with a range "
                  "of [%g, %g], but this exceeds the granularity of single "
                  "precision floats, which only support "
                  "24 bits of mantissa precision.", delta, x0, x1);
        }

        *depthPtr = depth;
        *depthsPtr = NULL;

    } else {
        /* These are tracked for the common case where subseqeunt values have
         * the same depth, as an optimization. */
        float prevDelta = -1;
        uint8_t prevDepth = (uint8_t)256;
        uint8_t *depths = calloc((size_t)len, sizeof(*depths));

        for (int32_t i = 0; i < len; i++) {
            if (prevDelta == deltas[i]) {
                depths[i] = prevDepth;
                continue;
            }

            uint8_t depth;
            for (depth = 0; depth <= 24; depth++) {
                float width = deltas[i] * (float) (1 << depth);
                if (width > x1 - x0) {
                    depths[i] = depth;
                    prevDepth = depth;
                    break;
                }
            }

            DebugAssert(depth <= 24) {
                Panic("An accuracy of %g was requested for variables with "
                      "a range of [%g, %g], but this "
                      "exceeds the granularity of single precision "
                      "floats, which only support 24 bits of mantissa "
                      "precision.", delta, x0, x1);
            }
        }

        *depthPtr = 0;
        *depthsPtr = depths;
    }
}

FSeq mapFloat(FSeq data, int32_t log10Scaled, float symLog10Threshold) {
    switch (log10Scaled) {
    case 0:
        return data;
    case 1:
        ;
        FSeq map = FSeq_New(data.Len);
        for (int32_t i = 0; i < map.Len; i++) {
            map.Data[i] = log10f(data.Data[i]);
        }

        return map;
    case 2:
        (void) symLog10Threshold;
        Panic("symLog10 not supported.%s", "");
    default:
        Panic("log10Scaled not set to 0, 1, or 2.%s", "");
    }
}

void unmapFloat(FSeq map, int32_t log10Scaled) {
    if (log10Scaled) { FSeq_Free(map); }
}
