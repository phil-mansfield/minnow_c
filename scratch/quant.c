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
    case field_Posn: break;
    case field_Velc: break;
    case field_Ptid: break;
    case field_Unsf: break;
    case field_Unsi: break;
    default: Panic("Unrecognized field code %"PRIx32".", qf.Hd.FieldCode);
    }
}

void quant_FreeField(Field f) {
    switch(f.Hd.FieldCode) {
    case field_Posn: break;
    case field_Velc: break;
    case field_Ptid: break;
    case field_Unsf: break;
    case field_Unsi: break;
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

Field quant_UndoPosition(QField qf) {
    (void) qf;
    
    Field dummy;
    return dummy;
}

QField quant_Velocity(Field f) {
    (void) f;
    
    QField dummy;
    return dummy;
}

Field quant_UndoVelocity(QField qf) {
    (void) qf;
    
    Field dummy;
    return dummy;
}

QField quant_ID(Field f) {
    (void) f;
    
    QField dummy;
    return dummy;
}

Field quant_UndoID(QField qf) {
    (void) qf;
    
    Field dummy;
    return dummy;
}

QField quant_Float(Field f) {
    (void) f;
    
    QField dummy;
    return dummy;
}

Field quant_UndoFloat(QField qf) {
    (void) qf;
    
    Field dummy;
    return dummy;
}

QField quant_Int(Field f) {
    (void) f;
    
    QField dummy;
    return dummy;
}

Field quant_UndoInt(QField qf) {
    (void) qf;
    
    Field dummy;
    return dummy;
}
