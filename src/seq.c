#include "seq.h"
#include "base_seq.c" /* This is done to make autogeneration cleaner. */
#include "stdint.h"

GENERATE_SEQ_BODY(double, DSeq)
GENERATE_SEQ_BODY(float, FSeq)

GENERATE_SEQ_BODY(int64_t, I64Seq)
GENERATE_SEQ_BODY(int32_t, I32Seq)
GENERATE_SEQ_BODY(uint64_t, U64Seq)
GENERATE_SEQ_BODY(uint32_t, U32Seq)

GENERATE_SEQ_BODY(uint8_t, U8Seq)

GENERATE_SEQ_BODY(int, ISeq)
GENERATE_SEQ_BODY(unsigned int, UISeq)
GENERATE_SEQ_BODY(long, LSeq)
GENERATE_SEQ_BODY(long long, LLSeq)
