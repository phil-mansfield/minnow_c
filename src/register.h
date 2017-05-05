#ifndef MNW_REGISTER_H_
#define MNW_REGISTER_H_

#include <stdbool.h>

#include "types.h"
#include "seq.h"

typedef struct Register {
    U32Seq Algo;
    U32SeqSeq Version;
    PtrSeqSeq Funcs;
} Register;

typedef struct RegisterFuncs {
    CFunc CFunc;
    void *(*CAlloc)(void);
    void (*CFree)(void *);
    DFunc DFunc;
    void *(*DAlloc)(void);
    void (*DFree)(void *);
} RegisterFuncs;

Register Register_New(void);

void Register_Free(Register reg);

void Register_Add(
    Register reg, uint32_t algo, uint32_t version, RegisterFuncs funcs
);

Compressor Register_GetCompressor(
    Register reg, uint32_t algo, uint32_t version
);

Decompressor Register_GetDecompressor(
    Register reg, uint32_t algo, uint32_t version
);

void Register_FreeDecompressor(
    Register reg, uint32_t algo, uint32_t version, Decompressor decomp
);

void Register_FreeCompressor(
    Register reg, uint32_t algo, uint32_t version, Compressor comp
);

bool Register_Supports(Register reg, uint32_t algo, uint32_t version);
uint32_t Register_Newest(Register reg, uint32_t algo);

#endif
