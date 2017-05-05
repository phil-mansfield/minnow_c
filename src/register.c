#include "register.h"


Register Register_New(void) {
    Register reg;
    memset(&reg, 0, sizeof(reg));
    return reg;
}

void Register_Free(Register reg) {
    U32Seq_Free(reg.Algo);
    for (int32_t i = 0; i < reg.Version.Len; i++) {
        int32_t len = reg.Funcs.Data[i].Len;
        void **ptrs = reg.Funcs.Data[i].Data;
        for (int32_t j = 0; j < len; j++) free(ptrs[j]);
        PtrSeq_Free(reg.Funcs.Data[i]);
    }
    U32SeqSeq_Free(reg.Version);
    PtrSeqSeq_Free(reg.Funcs);
}

void Register_Add(
    Register reg, uint32_t algo, uint32_t version, RegisterFuncs funcs
) {
    (void) reg;
    (void) algo;
    (void) version;
    (void) funcs;
}

Decompressor Register_GetDecompressor(
    Register reg, uint32_t algo, uint32_t version
) {
    (void) reg;
    (void) algo;
    (void) version;

    Decompressor dummy;
    return dummy;
}

Compressor Register_GetCompressor(
    Register reg, uint32_t algo, uint32_t version
) {
    (void) reg;
    (void) algo;
    (void) version;

    Compressor dummy;
    return dummy;
}

void Register_FreeDecompressor(
    Register reg, uint32_t algo, uint32_t version, Decompressor decomp
) {
    (void) reg;
    (void) algo;
    (void) version;
    (void) decomp;
}

void Register_FreeCompressor(
    Register reg, uint32_t algo, uint32_t version, Compressor comp
) {
    (void) reg;
    (void) algo;
    (void) version;
    (void) comp;
}

bool Register_Supports(Register reg, uint32_t algo, uint32_t version) {
    (void) reg;
    (void) algo;
    (void) version;

    return false;
}

uint32_t Register_Newest(Register reg, uint32_t algo) {
    (void) reg;
    (void) algo;
    
    return 0;
}
