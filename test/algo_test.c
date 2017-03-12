#include "algo.h"
#include "seq.h"
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

#define LEN(x) ((int)(sizeof(x) / sizeof(x[0])))

bool testCheckParticles();

int main() {
    bool res = true;

    res = res && testCheckParticles();

    return !res;
}


bool testCheckParticles() {
    struct {
        int32_t pos, vel, ID, fVar[3], fVarLen, u64Var[3], u64VarLen;
    } tests[] = {
        {10, 10, 10, {0}, 0, {0}, 0},
        {11, 11, 11, {11}, 1, {11}, 1},
        {12, 12, 12, {12, 12}, 2, {12, 12}, 2},

        {3, 3, 3, {3, 3, 3}, 3, {3, 3, 3}, 3},
        {3, 0, 3, {3, 3, 3}, 3, {3, 3, 3}, 3},
        {3, 3, 0, {3, 3, 3}, 3, {3, 3, 3}, 3},
        {3, 3, 3, {3, 3, 3}, 3, {3, 3, 3}, 3},
        {3, 3, 3, {0, 3, 3}, 3, {3, 3, 3}, 3},
        {3, 3, 3, {3, 0, 3}, 3, {0, 3, 3}, 3},
        {3, 3, 3, {3, 0, 3}, 3, {3, 0, 3}, 3},

        {3, 3, 3, {3, 3, 3}, 3, {3, 3, 3}, 3},
        {3, 0, 3, {3, 3, 3}, 3, {3, 3, 3}, 3},
        {3, 0, 0, {3, 3, 3}, 3, {3, 3, 3}, 3},
        {3, 0, 0, {3, 3, 3}, 3, {3, 3, 3}, 3},
        {3, 0, 0, {0, 3, 3}, 3, {3, 3, 3}, 3},
        {3, 0, 0, {0, 0, 0}, 0, {3, 3, 3}, 3},
        {3, 0, 0, {0, 0, 0}, 0, {0, 3, 3}, 3},
        {3, 0, 0, {0, 0, 0}, 0, {0, 0, 3}, 3},

        /* All the following tests should crash: */
        //{0, 0, 0, 0, {0, 0, 0}, 3},
        //{3, 4, 0, 0, {0, 0, 0}, 3},
        //{3, 0, 4, 0, {0, 0, 0}, 3},
        //{3, 0, 0, 4, {0, 0, 0}, 3},
        //{3, 0, 0, 0, {4, 0, 0}, 3},
        //{3, 0, 0, 0, {0, 4, 0}, 3},
        //{0, 3, 4, 0, {0, 0, 0}, 3},
        //{0, 3, 0, 4, {0, 0, 0}, 3},
        //{0, 3, 0, 0, {4, 0, 0}, 3},
        //{0, 3, 0, 0, {0, 4, 0}, 3},
        //{0, 0, 3, 4, {0, 0, 0}, 3},
        //{0, 0, 3, 0, {4, 0, 0}, 3},
        //{0, 0, 3, 0, {0, 4, 0}, 3},
        //{0, 0, 0, 3, {4, 0, 0}, 3},
        //{0, 0, 0, 3, {0, 4, 0}, 3},
        //{0, 0, 0, 0, {3, 4, 0}, 3},
    };
	
    for (int i = 0; i < LEN(tests); i++){
        algo_Particles p;
        for (int j = 0; j < 3; j++) {
            p.X[j] = FSeq_New(tests[i].pos);
            p.V[j] = FSeq_New(tests[i].vel);
        }
        p.ID = U64Seq_New(tests[i].ID);

        p.FVars = FSeqSeq_New(tests[i].fVarLen);
        for (int32_t j = 0; j < tests[i].fVarLen; j++) {
            p.FVars.Data[j] = FSeq_New(tests[i].fVar[j]);
        }
        p.U64Vars = U64SeqSeq_New(tests[i].u64VarLen);
        for (int32_t j = 0; j < tests[i].u64VarLen; j++) {
            p.U64Vars.Data[j] = U64Seq_New(tests[i].u64Var[j]);
        }

        algo_CheckParticles(p);
    }

    return true;
}
