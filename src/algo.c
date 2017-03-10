#include "algo.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include "debug.h"


void algo_CheckParticles(algo_Particles p) {
    int32_t velLen = p.Velocity[0].Len;
    int32_t posLen = p.Position[0].Len;
    for (int i = 1; i < 3; i++) {
        if (p.Velocity[i].Len != velLen) {
            Panic("The lengths of the three velocity sequences are (%"PRId32
                  ", %"PRId32", %"PRId32"), but they all need to be the "
                  "same length.", p.Velocity[0].Len, p.Velocity[1].Len,
                  p.Velocity[2].Len);
        }
        if (p.Position[i].Len != posLen) {
            Panic("The lengths of the three position sequences are (%"PRId32
                  ", %"PRId32", %"PRId32"), but they all need to be the "
                  "same length.", p.Position[0].Len, p.Position[1].Len,
                  p.Position[2].Len);
        }
    }

    int32_t len = posLen;
    char *anchorName = "position";

    if (len == 0) {
        len = velLen;
        anchorName = "velocity";
    } else if (len != velLen && velLen != 0) {
        Panic("The length of the velocity seqeunce is %"PRId32", but the "
              "length of the position sequence is %"PRId32".", velLen, posLen);
    }

    if (len == 0) {
        len = p.ID64.Len;
        anchorName = "ID64";
    } else if (p.ID64.Len != len && p.ID64.Len != 0) {
        Panic("The length of the ID64 sequence is %"PRId32", but the length of "
              "the %s sequence is %"PRId32".", p.ID64.Len, anchorName, len);
    }

    if (len == 0) {
        len = p.ID32.Len;
        anchorName = "ID32";
    } else if (p.ID32.Len != len && p.ID32.Len != 0) {
        Panic("The length of the ID32 sequence is %"PRId32", but the length of "
              "the %s sequence is %"PRId32".", p.ID32.Len, anchorName, len);
    }

    for (int32_t i = 0; i < p.FVars.Len; i++) {
        if (len == 0) {
            len = p.FVars.Data[i].Len;
            char anchorNameBuf[128];
            sprintf(anchorNameBuf, "FVars[%"PRId32"]", i);
            anchorName = anchorNameBuf;
        } else if (p.FVars.Data[i].Len != len &&
                   p.FVars.Data[i].Len != 0) {
            Panic("The length of the FVars[%"PRId32"] sequence is %"PRId32
                  ", but the length of the %s sequence is %"PRId32".",
                  i, p.FVars.Data[i].Len, anchorName, len);
        }
    }

    for (int32_t i = 0; i < p.U64Vars.Len; i++) {
        if (len == 0) {
            len = p.U64Vars.Data[i].Len;
            char anchorNameBuf[128];
            sprintf(anchorNameBuf, "U64Vars[%"PRId32"]", i);
            anchorName = anchorNameBuf;
        } else if (p.U64Vars.Data[i].Len != len &&
                   p.U64Vars.Data[i].Len != 0) {
            Panic("The length of the U64Vars[%"PRId32"] sequence is %"PRId32
                  ", but the length of the %s sequence is %"PRId32".",
                  i, p.U64Vars.Data[i].Len, anchorName, len);
        }
    }

    if (len == 0) {
        Panic("All input seqeunces are of length zero.%s", "");
    }
}

algo_Particles algo_ExtendBufferParticles(
    algo_CompressedParticles ref, algo_Particles buf
) {
    (void) ref;
    (void) buf;
    return buf;
}
