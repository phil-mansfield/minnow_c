#include "algo.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include "debug.h"

void Particles_Free(algo_Particles p) {
    (void) p;
}

algo_Particles Particles_Match(
    algo_QuantizedParticles ref, algo_Particles buf
) {
    (void) ref;
    return buf;
}

void QuantizedParticles_Free(algo_QuantizedParticles p) {
    (void) p;
}


algo_QuantizedParticles QuantizedParticles_Match(
    algo_Particles ref, algo_QuantizedParticles buf
) {
    (void) ref;
    return buf;
}

void CompressedParticles_Free(algo_CompressedParticles p) {
    (void) p;
}

algo_QuantizedParticles algo_Quantize(
    algo_Particles p, algo_QuantizedParticles buf
) {
    (void) p;
    return buf;
}

algo_Particles algo_UndoQuantize(
    algo_QuantizedParticles p, algo_Particles buf
) {
    (void) p;
    return buf;
}

void algo_CheckQuantizedParticles(algo_QuantizedParticles p) {
    int32_t len = p.X[0].Len;
    if (len <= 0) {
        Panic("X sequences have been left empty.%s", "");
    }

    if (p.X[1].Len != len || p.X[2].Len != len) {
        Panic("The lengths of the three position sequences are (%"PRId32
              ", %"PRId32", %"PRId32"), but they all need to be the "
              "same length.", p.X[0].Len, p.X[1].Len,
              p.X[2].Len);
    }

    if (p.V[0].Len != p.V[1].Len ||  p.V[0].Len != p.V[2].Len) {
        Panic("The lengths of the three velocity sequences are (%"PRId32
              ", %"PRId32", %"PRId32"), but they all need to be the "
              "same length.", p.V[0].Len, p.V[1].Len,
              p.V[2].Len);
    }

    if (p.ID[0].Len != p.ID[1].Len ||  p.ID[0].Len != p.ID[2].Len) {
        Panic("The lengths of the three ID sequences are (%"PRId32
              ", %"PRId32", %"PRId32"), but they all need to be the "
              "same length.", p.ID[0].Len, p.ID[1].Len,
              p.ID[2].Len);
    }

    if (p.V[0].Len != len && p.V[0].Len != 0) {
        Panic("The length of the velocity sequence is %"PRId32", but the "
              "length of the position sequence is %"PRId32".", p.V[0].Len, len);
    }

    if (p.ID[0].Len != len && p.ID[0].Len != 0) {
        Panic("The length of the ID sequence is %"PRId32", but the length of "
              "the position sequence is %"PRId32".", p.ID[0].Len, len);
    }

    for (int32_t i = 0; i < p.FVars.Len; i++) {
        if (p.FVars.Data[i].Len != len &&
            p.FVars.Data[i].Len != 0) {
            Panic("The length of the FVars[%"PRId32"] sequence is %"PRId32
                  ", but the length of the position sequence is %"PRId32".",
                  i, p.FVars.Data[i].Len, len);
        }
    }

    for (int32_t i = 0; i < p.U64Vars.Len; i++) {
        if (p.U64Vars.Data[i].Len != len &&
            p.U64Vars.Data[i].Len != 0) {
            Panic("The length of the U64Vars[%"PRId32"] sequence is %"PRId32
                  ", but the length of the position sequence is %"PRId32".",
                  i, p.U64Vars.Data[i].Len, len);
        }
    }
}

void algo_CheckParticles(algo_Particles p) {
    int32_t len = p.X[0].Len;
    if (len <= 0) {
        Panic("X sequences have been left empty.%s", "");
    }

    if (p.X[1].Len != len || p.X[2].Len != len) {
        Panic("The lengths of the three position sequences are (%"PRId32
              ", %"PRId32", %"PRId32"), but they all need to be the "
              "same length.", p.X[0].Len, p.X[1].Len,
              p.X[2].Len);
    }

    if (p.V[0].Len != p.V[1].Len ||  p.V[0].Len != p.V[2].Len) {
        Panic("The lengths of the three velocity sequences are (%"PRId32
              ", %"PRId32", %"PRId32"), but they all need to be the "
              "same length.", p.V[0].Len, p.V[1].Len,
              p.V[2].Len);
    }

    if (p.V[0].Len != len && p.V[0].Len != 0) {
        Panic("The length of the velocity sequence is %"PRId32", but the "
              "length of the position sequence is %"PRId32".", p.V[0].Len, len);
    }

    if (p.ID.Len != len && p.ID.Len != 0) {
        Panic("The length of the ID sequence is %"PRId32", but the length of "
              "the position sequence is %"PRId32".", p.ID.Len, len);
    }

    for (int32_t i = 0; i < p.FVars.Len; i++) {
        if (p.FVars.Data[i].Len != len &&
            p.FVars.Data[i].Len != 0) {
            Panic("The length of the FVars[%"PRId32"] sequence is %"PRId32
                  ", but the length of the position sequence is %"PRId32".",
                  i, p.FVars.Data[i].Len, len);
        }
    }

    for (int32_t i = 0; i < p.U64Vars.Len; i++) {
        if (p.U64Vars.Data[i].Len != len &&
            p.U64Vars.Data[i].Len != 0) {
            Panic("The length of the U64Vars[%"PRId32"] sequence is %"PRId32
                  ", but the length of the position sequence is %"PRId32".",
                  i, p.U64Vars.Data[i].Len, len);
        }
    }
}
