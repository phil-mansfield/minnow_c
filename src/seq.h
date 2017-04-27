#ifndef MNW_SEQ_H_
#define MNW_SEQ_H_

#include "base_seq.h"

GENERATE_SEQ_HEADER(double, DSeq, DBigSeq)
GENERATE_SEQ_HEADER(float, FSeq, FBigSeq)

GENERATE_SEQ_HEADER(int64_t, I64Seq, I64BigSeq)
GENERATE_SEQ_HEADER(int32_t, I32Seq, I32BigSeq)

GENERATE_SEQ_HEADER(uint64_t, U64Seq, U64BigSeq)
GENERATE_SEQ_HEADER(uint32_t, U32Seq, U32BigSeq)
GENERATE_SEQ_HEADER(uint8_t, U8Seq, U8BigSeq)

GENERATE_SEQ_HEADER(FSeq, FSeqSeq, FBigSeqSeq)

GENERATE_SEQ_HEADER(U64Seq, U64SeqSeq, U64BigSeqSeq)
GENERATE_SEQ_HEADER(U32Seq, U32SeqSeq, U32BigSeqSeq)
GENERATE_SEQ_HEADER(U8Seq, U8SeqSeq, U8BigSeqSeq)

GENERATE_SEQ_HEADER(void*, PtrSeq, PtrBigSeq)
GENERATE_SEQ_HEADER(PtrSeq, PtrSeqSeq, PtrBigSeqSeq)

#endif /* MNW_SEQ_H_ */
