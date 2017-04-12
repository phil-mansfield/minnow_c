#include "seq.h"
#include "base_seq.c" /* This is done to make autogeneration cleaner. */
#include "stdint.h"

GENERATE_SEQ_BODY(double, DSeq, DBigSeq)
GENERATE_SEQ_BODY(float, FSeq, FBigSeq)

GENERATE_SEQ_BODY(int64_t, I64Seq, I64BigSeq)
GENERATE_SEQ_BODY(int32_t, I32Seq, I32BigSeq)

GENERATE_SEQ_BODY(uint64_t, U64Seq, U64BigSeq)
GENERATE_SEQ_BODY(uint32_t, U32Seq, U32BigSeq)
GENERATE_SEQ_BODY(uint8_t, U8Seq, U8BigSeq)

GENERATE_SEQ_BODY(FSeq, FSeqSeq, FBigSeqSeq)

GENERATE_SEQ_BODY(U64Seq, U64SeqSeq, U64BigSeqSeq)
GENERATE_SEQ_BODY(U32Seq, U32SeqSeq, U32BigSeqSeq)
GENERATE_SEQ_BODY(U8Seq, U8SeqSeq, U8BigSeqSeq)
