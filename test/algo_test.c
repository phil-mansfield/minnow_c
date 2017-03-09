#include "algo.h"
#include "seq.h"
#include <stdbool.h>
#include <stdio.h>

#define LEN(x) ((int)(sizeof(x) / sizeof(x[0])))

bool testCheckParticles();

int main() {
    bool res = true;

    res = res && testCheckParticles();

    return !res;
}


bool testCheckParticles() {
    struct {int32_t pos, vel, ID64, ID32, var[4], varLen; } tests[] = {
        {10, 10, 10, 10, {0}, 0},
        {11, 11, 11, 11, {11}, 1},
        {12, 12, 12, 12, {12, 12}, 2},

        {0, 3, 3, 3, {3, 3, 3}, 3},
        {3, 0, 3, 3, {3, 3, 3}, 3},
        {3, 3, 0, 3, {3, 3, 3}, 3},
        {3, 3, 3, 0, {3, 3, 3}, 3},
        {3, 3, 3, 3, {0, 3, 3}, 3},
        {3, 3, 3, 3, {3, 0, 3}, 3},

        {0, 3, 3, 3, {3, 3, 3}, 3},
        {0, 0, 3, 3, {3, 3, 3}, 3},
        {0, 0, 0, 3, {3, 3, 3}, 3},
        {0, 0, 0, 0, {3, 3, 3}, 3},
        {0, 0, 0, 0, {0, 3, 3}, 3},

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
            p.Position[j] = FSeq_New(tests[i].pos);
            p.Velocity[j] = FSeq_New(tests[i].vel);
        }
        p.ID64 = U64Seq_New(tests[i].ID64);
        p.ID32 = U32Seq_New(tests[i].ID32);

        p.Variables = FSeqSeq_New(tests[i].varLen);
        for (int32_t j = 0; j < tests[i].varLen; j++) {
            p.Variables.Data[j] = FSeq_New(tests[i].var[j]);
        }

        algo_CheckParticles(p);
    }

    return true;
}
