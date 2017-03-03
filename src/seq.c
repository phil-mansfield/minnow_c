#include "seq.h"
#include "base_seq.c" /* This is done to make autogeneration cleaner. */
#include "stdint.h"

GENERATE_SEQ_BODY(double, DSeq)
GENERATE_SEQ_BODY(float, FSeq)

GENERATE_SEQ_BODY(int64_t, I64Seq)
GENERATE_SEQ_BODY(int32_t, I32Seq)
GENERATE_SEQ_BODY(int16_t, I16Seq)
GENERATE_SEQ_BODY(int8_t, I8Seq)

GENERATE_SEQ_BODY(uint64_t, U64Seq)
GENERATE_SEQ_BODY(uint32_t, U32Seq)
GENERATE_SEQ_BODY(uint16_t, U16Seq)
GENERATE_SEQ_BODY(uint8_t, U8Seq)
