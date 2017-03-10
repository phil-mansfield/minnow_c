#ifndef MNW_SEQ_H_
#define MNW_SEQ_H_

#include "base_seq.h"

GENERATE_SEQ_HEADER(double, DSeq)
GENERATE_SEQ_HEADER(float, FSeq)

GENERATE_SEQ_HEADER(int64_t, I64Seq)
GENERATE_SEQ_HEADER(int32_t, I32Seq)
GENERATE_SEQ_HEADER(int16_t, I16Seq)
GENERATE_SEQ_HEADER(int8_t, I8Seq)

GENERATE_SEQ_HEADER(uint64_t, U64Seq)
GENERATE_SEQ_HEADER(uint32_t, U32Seq)
GENERATE_SEQ_HEADER(uint16_t, U16Seq)
GENERATE_SEQ_HEADER(uint8_t, U8Seq)

GENERATE_SEQ_HEADER(FSeq, FSeqSeq)
GENERATE_SEQ_HEADER(U8Seq, U8SeqSeq)
GENERATE_SEQ_HEADER(U64Seq, U64SeqSeq)

#endif /* MNW_SEQ_H_ */
