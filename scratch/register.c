#include "register.h"


Register Register_New(void) {
    Register reg;
    memset(&reg, 0, sizeof(reg));
    return reg;
}

void Register_Free(Register reg) {
    U32Seq_Free(reg.Algo);
    for (int32_t i = 0; i < reg.Version.Len; i++) {
        U32Seq_Free(reg.Version.Data[i]);
        PtrSeq_Free(reg.DFunc.Data[i]);
        PtrSeq_Free(reg.CFunc.Data[i]);
    }
    U32SeqSeq_Free(reg.Version);
    PtrSeqSeq_Free(reg.DFunc);
    PtrSeqSeq_Free(reg.CFunc);
}

void Register_Add(
    Register reg, uint32_t algo, uint32_t version, CFunc cf, DFunc df
) {
    (void) reg;
    (void) algo;
    (void) version;
    (void) cf;
    (void) df;
    
}

void Register_Get(
    Register reg, uint32_t algo, uint32_t version, CFunc *cf, DFunc *df
) {
    (void) reg;
    (void) algo;
    (void) version;
    (void) cf;
    (void) df;
}

bool Register_Supports(Register reg, uint32_t algo) {
    (void) reg;
    (void) algo;

    return false;
}

uint32_t Register_Newest(Register reg, uint32_t algo) {
    (void) reg;
    (void) algo;
    
    return 0;
}
