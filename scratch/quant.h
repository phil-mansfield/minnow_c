#ifndef MNW_QUANT_H_
#define MNW_QUANT_H_

#include "types.h"

Field quant_Field(QField qf);
QField quant_QField(Field f);
void quant_FreeQField(QField qf);
void quant_FreeField(QField f);

#endif
