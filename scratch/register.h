#ifndef MNW_REGISTER_H_
#define MNW_REGISTER_H_

#include <stdbool.h>

#include "types.h"
#include "../src/seq.h"

typedef struct Register {
    U32Seq Algo;
    U32SeqSeq Version;
    PtrSeqSeq DFunc;
    PtrSeqSeq CFunc;
} Register;

Register Register_New(void);
void Register_Free(Register reg);

void Register_Add(
    Register reg, uint32_t algo, uint32_t version, CFunc cf, DFunc df
);

void Register_Get(
    Register reg, uint32_t algo, uint32_t version, CFunc *cf, DFunc *df
);

bool Register_Supports(Register reg, uint32_t algo);
uint32_t Register_Newest(Register reg, uint32_t algo);

#endif
