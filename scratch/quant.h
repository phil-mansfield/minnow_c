#ifndef MNW_QUANT_H_
#define MNW_QUANT_H_

#include "types.h"

Field quant_UndoPosition(uint64_t *data, Quantization quant, int32_t pLen);
Field quant_UndoVelocity(uint64_t *data, Quantization quant, int32_t pLen);
Field quant_UndoID(uint64_t *data, Quantization quant, int32_t pLen);
Field quant_UndoFloat(uint64_t *data, Quantization quant, int32_t pLen);
Field quant_UndoInt(uint64_t *data, Quantization quant, int32_t pLen);

QField quant_Position(uint64_t *data, Accuracy acc, int32_t pLen);
QField quant_Velocity(uint64_t *data, Accuracy acc, int32_t pLen);
QField quant_ID(uint64_t *data, Accuracy acc, int32_t pLen);
QField quant_Float(uint64_t *data, Accuracy acc, int32_t pLen);
QField quant_Int(uint64_t *data, Accuracy acc, int32_t pLen);

#endif
