#include <inttypes.h>
#include <math.h>
#include <time.h>

#include "quant.h"
#include "../src/debug.h"
#include "../src/rand.h"

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

void setNaNs(
    float *data, float *deltas,
    uint64_t *qdata, uint64_t nanFlag,
    int32_t len
);

void depthToDelta(
    uint8_t depth, uint8_t *depths,
    float x0, float x1,
    float *deltaPtr, float**deltasPtr,
    int32_t len
);

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

/* quantization funcitons */

QField position(Field f) {
    (void) f;
    
    QField dummy;
    return dummy;
}

QField velocity(Field f) {
    (void) f;
    
    QField dummy;
    return dummy;
}

QField id(Field f) {
    (void) f;
    
    QField dummy;
    return dummy;
}

QField ufloat(Field f) {
    (void) f;
    
    QField dummy;
    return dummy;
}

QField uint(Field f) {    
    (void) f;
    
    QField dummy;
    return dummy;
}

/* dequantization functions */

Field undoUfloat(QField qf) {
    /* Set things up. */
    Field f;
    memset(&f, 0, sizeof(f));
    memcpy(&f.Hd, &qf.Hd, sizeof(f.Hd));

    if (!qf.Data) {
        /* Deal with completely invalid data. */
        float *x = malloc(sizeof(*x) * (size_t) f.Hd.ParticleLen);
        for (int32_t i = 0; i < f.Hd.ParticleLen; i++) { x[i] = NAN; }
        f.Data = x;
        return f;
    }
    
    /* Dequantize data. */

    FloatQuantization quant = *(FloatQuantization*)qf.Quant;
    uint64_t *qdata = (uint64_t*)qf.Data;
    float *data = calloc((size_t) f.Hd.ParticleLen, sizeof(*data));

    if (quant.Log10Scaled == 1) {
        undoLog10Float(
            quant.X0, quant.X1, quant.Depth, quant.Depths,
            qdata, data, f.Hd.ParticleLen
        );
    } else if (quant.Log10Scaled == 2) {
        undoSymLog10Float(
            quant.X0, quant.X1, quant.SymLog10Threshold,
            quant.Depth, quant.Depths, qdata, data, f.Hd.ParticleLen
        );
    } else {
        undoFloat(
            quant.X0, quant.X1, quant.Depth, quant.Depths,
            qdata, data, f.Hd.ParticleLen
        );
    }

    /* Set Acc. */

    FloatAccuracy *acc = calloc(1, sizeof(*acc));

    acc->SymLog10Threshold = quant.SymLog10Threshold;
    acc->Len = quant.Len;
    acc->Log10Scaled = quant.Log10Scaled;
    depthToDelta(
        quant.Depth, quant.Depths, quant.X0, quant.X1,
        &acc->Delta, &acc->Deltas, f.Hd.ParticleLen
    );

    f.Acc = acc;
    
    /* Deal with partially invalid data. */

    setNaNs(data, acc->Deltas, qdata, quant.NaNFlag, f.Hd.ParticleLen);
    f.Data = data;

    return f;
}

Field undoPosition(QField qf) {
    /* Set things up. */
    Field f;
    memset(&f, 0, sizeof(f));
    memcpy(&f.Hd, &qf.Hd, sizeof(f.Hd));

    if (!qf.Data) {
        /* Deal with completely invalid data. */
        float *x = malloc(sizeof(*x) * (size_t) f.Hd.ParticleLen);
        for (size_t i = 0; i < 3*(size_t) f.Hd.ParticleLen; i++) { x[i] = NAN; }
        f.Data = x;
        return f;
    }
    
    /* Dequantize data. */

    PositionQuantization quant = *(PositionQuantization*)qf.Quant;
    uint64_t *qdata = (uint64_t*)qf.Data;
    float *data = calloc(3 * (size_t)f.Hd.ParticleLen, sizeof(*data));
    float *xData = data;
    float *yData = xData + f.Hd.ParticleLen;
    float *zData = yData + f.Hd.ParticleLen;
    float *dimData[3] = {xData, yData, zData}; 

    for (int i = 0; i < 3; i++) {
        undoFloat(
            quant.X0[i], quant.X1[i], quant.Depth, quant.Depths,
            qdata, dimData[i], f.Hd.ParticleLen
        );
    }

    /* Set Acc. */

    PositionAccuracy *acc = calloc(1, sizeof(*acc));
    acc->Len = quant.Len;
    depthToDelta(
        quant.Depth, quant.Depths, quant.X0[0], quant.X1[0],
        &acc->Delta, &acc->Deltas, f.Hd.ParticleLen
    );
    f.Acc = acc;
    
    /* Deal with partially invalid data. */

    int32_t len = f.Hd.ParticleLen;
    setNaNs(data, acc->Deltas, qdata, quant.NaNFlag, len);
    setNaNs(data + len, acc->Deltas, qdata, quant.NaNFlag, len);
    setNaNs(data + 2*(size_t)len, acc->Deltas, qdata, quant.NaNFlag, len);
    f.Data = data;

    return f;
}

Field undoVelocity(QField qf) {
    /* Set things up. */
    Field f;
    memset(&f, 0, sizeof(f));
    memcpy(&f.Hd, &qf.Hd, sizeof(f.Hd));

    if (!qf.Data) {
        /* Deal with completely invalid data. */
        float *x = malloc(sizeof(*x) * (size_t) f.Hd.ParticleLen);
        for (size_t i = 0; i < 3*(size_t) f.Hd.ParticleLen; i++) { x[i] = NAN; }
        f.Data = x;
        return f;
    }
    
    /* Dequantize data. */

    VelocityQuantization quant = *(VelocityQuantization*)qf.Quant;
    uint64_t *qdata = (uint64_t*)qf.Data;
    float *data = calloc(3 * (size_t)f.Hd.ParticleLen, sizeof(*data));
    float *xData = data;
    float *yData = xData + f.Hd.ParticleLen;
    float *zData = yData + f.Hd.ParticleLen;
    float *dimData[3] = {xData, yData, zData}; 

    for (int i = 0; i < 3; i++) {
        if (quant.SymLog10Scaled) {
            undoSymLog10Float(
                quant.X0[i], quant.X1[i], quant.SymLog10Threshold,
                quant.Depth, quant.Depths,
                qdata, dimData[i], f.Hd.ParticleLen
            );
        } else {
            undoFloat(
                quant.X0[i], quant.X1[i],
                quant.Depth, quant.Depths,
                qdata, dimData[i], f.Hd.ParticleLen
            );
        }
    }

    /* Set Acc. */

    VelocityAccuracy *acc = calloc(1, sizeof(*acc));
    acc->SymLog10Threshold = quant.SymLog10Threshold;
    acc->Len = quant.Len;
    acc->SymLog10Scaled = quant.SymLog10Scaled;
    depthToDelta(
        quant.Depth, quant.Depths, quant.X0[0], quant.X1[0],
        &acc->Delta, &acc->Deltas, f.Hd.ParticleLen
    );
    f.Acc = acc;
    
    /* Deal with partially invalid data. */

    int32_t len = f.Hd.ParticleLen;
    setNaNs(data, acc->Deltas, qdata, quant.NaNFlag, len);
    setNaNs(data + len, acc->Deltas, qdata, quant.NaNFlag, len);
    setNaNs(data + 2*(size_t)len, acc->Deltas, qdata, quant.NaNFlag, len);
    f.Data = data;

    return f;
}

Field undoID(QField qf) {
    /* Set things up. */
    Field f;
    memset(&f, 0, sizeof(f));
    memcpy(&f.Hd, &qf.Hd, sizeof(f.Hd));

    if (!qf.Data) {
        /* Deal with completely invalid data. */
        uint64_t *x = malloc(sizeof(*x) * (size_t) f.Hd.ParticleLen);
        memset(x, 0xff, sizeof(*x) * (size_t) f.Hd.ParticleLen);
        f.Data = x;
        return f;
    }
    
    /* Dequantize data. */

    IDQuantization quant = *(IDQuantization*)qf.Quant;
    uint64_t *qdata0 = (uint64_t*)qf.Data;
    uint64_t *qdata1 = ((uint64_t*)qf.Data) + f.Hd.ParticleLen;
    uint64_t *qdata2 = ((uint64_t*)qf.Data) + 2*f.Hd.ParticleLen;
    uint64_t *data = malloc(sizeof(*data) * (size_t) f.Hd.ParticleLen);

    uint64_t w = quant.Width;
    for (int32_t i = 0; i < f.Hd.ParticleLen; i++) {
        uint64_t x = qdata0[i] + quant.X0[0];
        if (x >= quant.Width) x -= quant.Width;
        uint64_t y = qdata1[i] + quant.X0[1];
        if (y >= quant.Width) y -= quant.Width;
        uint64_t z = qdata2[i] + quant.X0[2];
        if (z >= quant.Width) z -= quant.Width;
        data[i] = x + w*y + w*w*z;
    }

    /* Set Acc. */

    IDAccuracy *acc = calloc(1, sizeof(IDAccuracy));
    acc->Width = quant.Width;
    f.Acc = acc;

    /* Deal with partially invalid data. */

    for (int32_t i = 0; i < f.Hd.ParticleLen; i++) {
        if (qdata0[i] == quant.NaNFlag) { data[i] = 0xffffffffffffffff; }
    }

    f.Data = data;
    return f;
}

Field undoUint(QField qf) {
    /* Set things up. */
    Field f;
    memset(&f, 0, sizeof(f));
    memcpy(&f.Hd, &qf.Hd, sizeof(f.Hd));

    if (!qf.Data) {
        /* Deal with completely invalid data. */
        uint64_t *x = malloc(sizeof(*x) * (size_t) f.Hd.ParticleLen);
        memset(x, 0xff, sizeof(*x) * (size_t) f.Hd.ParticleLen);
        f.Data = x;
        return f;
    }
    
    /* Dequantize data. */

    IntQuantization quant = *(IntQuantization*)qf.Quant;
    uint64_t *qdata = (uint64_t*)qf.Data;
    uint64_t *data = malloc(sizeof(*data) * (size_t) f.Hd.ParticleLen);

    memcpy(data, qdata, sizeof(*data) * (size_t) f.Hd.ParticleLen);
    for (int32_t i = 0; i < f.Hd.ParticleLen; i++) { data[i] += quant.X0; }

    /* No need to set Acc. */

    /* Deal with partially invalid data. */

    for (int32_t i = 0; i < f.Hd.ParticleLen; i++) {
        if (qdata[i] == quant.NaNFlag) { data[i] = 0xffffffffffffffff; }
    }

    f.Data = data;
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

void setNaNs(
    float *data, float *deltas,
    uint64_t *qdata, uint64_t nanFlag,
    int32_t len
) {
    for (int32_t i = 0; i < len; i++) {
        if (qdata[i] == nanFlag) { data[i] = NAN; }
    }

    if (deltas) {
        for (int32_t i = 0; i < len; i++) {
            if (qdata[i] == nanFlag) { data[i] = NAN; }
        }
    }
}
