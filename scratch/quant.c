#include <inttypes.h>

#include "quant.h"
#include "../src/debug.h"

QField quant_Position(Field f);
Field quant_UndoPosition(QField qf);
QField quant_Velocity(Field f);
Field quant_UndoVelocity(QField qf);
QField quant_ID(Field f);
Field quant_UndoID(QField qf);
QField quant_Float(Field f);
Field quant_UndoFloat(QField qf);
QField quant_Int(Field f);
Field quant_UndoInt(QField qf);

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
    case field_Posn: return quant_Position(f);
    case field_Velc: return quant_Velocity(f);
    case field_Ptid: return quant_ID(f);
    case field_Unsf: return quant_Float(f);
    case field_Unsi: return quant_Int(f);
    default: Panic("Unrecognized field code %"PRIx32".", f.Hd.FieldCode);
    }
}

Field quant_Field(QField qf) {
    switch(qf.Hd.FieldCode) {
    case field_Posn: return quant_UndoPosition(qf);
    case field_Velc: return quant_UndoVelocity(qf);
    case field_Ptid: return quant_UndoID(qf);
    case field_Unsf: return quant_UndoFloat(qf);
    case field_Unsi: return quant_UndoInt(qf);
    default: Panic("Unrecognized field code %"PRIx32".", qf.Hd.FieldCode);
    }
}

QField quant_Position(Field f) {
    (void) f;
    
    QField dummy;
    return dummy;
}

QField quant_Velocity(Field f) {
    (void) f;
    
    QField dummy;
    return dummy;
}

QField quant_ID(Field f) {
    (void) f;
    
    QField dummy;
    return dummy;
}

QField quant_Float(Field f) {
    (void) f;
    
    QField dummy;
    return dummy;
}

QField quant_Int(Field f) {
    (void) f;
    
    QField dummy;
    return dummy;
}

/* quant_Undo functions */

Field quant_UndoFloat(QField qf) {
    (void) qf;
    
    Field dummy;
    return dummy;
}

Field quant_UndoPosition(QField qf) {
    (void) qf;
    
    Field dummy;
    return dummy;
}

Field quant_UndoVelocity(QField qf) {
    (void) qf;
    
    Field dummy;
    return dummy;
}

Field quant_UndoID(QField qf) {
    /* Set things up. */
    Field f;
    memset(&f, 0, sizeof(f));
    memcpy(&f.Hd, &qf.Hd, sizeof(f.Hd));

    if (!qf.Data) {
        /* Deal with completely invalid data. */
        uint64_t *x = malloc(sizeof(uint64_t) * (size_t) f.Hd.ParticleLen);
        memset(x, 0xff, sizeof(uint64_t) * (size_t) f.Hd.ParticleLen);
        f.Data = x;
        return f;
    }
    
    /* Dequantize data. */

    IDQuantization quant = *(IDQuantization*)qf.Quant;
    uint64_t *qdata0 = (uint64_t*)qf.Data;
    uint64_t *qdata1 = ((uint64_t*)qf.Data) + f.Hd.ParticleLen;
    uint64_t *qdata2 = ((uint64_t*)qf.Data) + 2*f.Hd.ParticleLen;
    uint64_t *data = malloc(sizeof(uint64_t) * (size_t) f.Hd.ParticleLen);

    uint64_t w = quant.Width;
    for (int32_t i = 0; i < f.Hd.ParticleLen; i++) {
        uint64_t x = qdata0[i] + quant.X0[0];
        if (x > quant.Width) x -= quant.Width;
        uint64_t y = qdata1[i] + quant.X0[1];
        if (y > quant.Width) y -= quant.Width;
        uint64_t z = qdata2[i] + quant.X0[2];
        if (z > quant.Width) z -= quant.Width;
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

Field quant_UndoInt(QField qf) {
    /* Set things up. */
    Field f;
    memset(&f, 0, sizeof(f));
    memcpy(&f.Hd, &qf.Hd, sizeof(f.Hd));

    if (!qf.Data) {
        /* Deal with completely invalid data. */
        uint64_t *x = malloc(sizeof(uint64_t) * (size_t) f.Hd.ParticleLen);
        memset(x, 0xff, sizeof(uint64_t) * (size_t) f.Hd.ParticleLen);
        f.Data = x;
        return f;
    }
    
    /* Dequantize data. */

    IntQuantization quant = *(IntQuantization*)qf.Quant;
    uint64_t *qdata = (uint64_t*)qf.Data;
    uint64_t *data = malloc(sizeof(uint64_t) * (size_t) f.Hd.ParticleLen);

    memcpy(data, qdata, sizeof(uint64_t) * (size_t) f.Hd.ParticleLen);
    for (int32_t i = 0; i < f.Hd.ParticleLen; i++) { data[i] += quant.X0; }

    /* No need to set Acc. */

    /* Deal with partially invalid data. */

    for (int32_t i = 0; i < f.Hd.ParticleLen; i++) {
        if (qdata[i] == quant.NaNFlag) { data[i] = 0xffffffffffffffff; }
    }

    f.Data = data;
    return f;
}
