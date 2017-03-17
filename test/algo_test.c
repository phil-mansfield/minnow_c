#include "algo.h"
#include "seq.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

#define LEN(x) ((int)(sizeof(x) / sizeof(x[0])))

bool testCheckParticles();
bool testSetQuantizedRanges();
bool almostEqual(float x, float y, float eps);

int main() {
    bool res = true;

    res = res && testCheckParticles();
    res = res && testSetQuantizedRanges();

    return !res;
}

bool almostEqual(float x, float y, float eps) {
    return x + eps > y && x - eps < y;
}

bool testCheckParticles() {
    struct {
        int32_t pos, vel, ID, fVar[3], fVarLen, u64Var[3], u64VarLen;
    } tests[] = {
	    {10, 0, 0, {0}, 0, {0}, 0},
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
    };
	
    for (int i = 0; i < LEN(tests); i++){
        algo_Particles p;
        for (int j = 0; j < 3; j++) {
            p.X[j] = FSeq_New(tests[i].pos);
            p.V[j] = FSeq_New(tests[i].vel);
        }
        p.XWidth = 75;
        p.XAcc.Delta = 0;
        p.XAcc.Deltas = FSeq_New(tests[i].pos);
        p.VAcc.Delta = tests[i].vel;
        p.VAcc.Deltas = FSeq_Empty();

        p.ID = U64Seq_New(tests[i].ID);
        p.IDWidth = 10;

        p.FVars = FSeqSeq_New(tests[i].fVarLen);
        p.FVarsAcc = calloc((size_t) tests[i].fVarLen, sizeof(*p.FVarsAcc));
        for (int32_t j = 0; j < tests[i].fVarLen; j++) {
            p.FVars.Data[j] = FSeq_New(tests[i].fVar[j]);
            p.FVarsAcc[j].Delta = tests[i].fVar[j];
        }

        p.U64Vars = U64SeqSeq_New(tests[i].u64VarLen);
        for (int32_t j = 0; j < tests[i].u64VarLen; j++) {
            p.U64Vars.Data[j] = U64Seq_New(tests[i].u64Var[j]);
        }

        Particles_Check(p);
        Particles_Free(p);
    }

    return true;
}

bool testFVarRange();
bool testVRange();
bool testXRange();
bool testIDRange();

bool testSetQuantizedRanges() {
    return testFVarRange() && testVRange() && testXRange() && testIDRange();
}


bool testFVarRange() {
    bool res = true;

    /* Uniform delta tests. */

    struct {
        float delta, var[8];
        int32_t len;
        float x0, x1;
        uint8_t depth;
    } tests[] = {
        {2, {3}, 1, 3, 5, 0},
        {2, {3, 4}, 2, 3, 5, 0},
        {2, {3, 5}, 2, 3, 7, 1},
        {0.5, {1, 2, 3, 4, 5, 6, 8}, 7, 1, 9, 4}
    };

	algo_QuantizedParticles q;
	memset(&q, 0, sizeof(q));

    for (int i = 0; i < LEN(tests); i++) {

        algo_Particles p;
        memset(&p, 0, sizeof(p));

        for (int j = 0; j < 3; j++) {
            p.X[j] = FSeq_New(tests[i].len);
        }
        p.XWidth = 10;
        p.XAcc.Delta = 1;

        p.FVarsAcc = calloc(1, sizeof(*p.FVarsAcc));
        p.FVarsAcc[0].Delta = tests[i].delta;

        p.FVars = FSeqSeq_New(1);
        FSeq var = FSeq_New(tests[i].len);
        p.FVars.Data[0] = var;
        for (int32_t j = 0; j < var.Len; j++) {
            var.Data[j] = tests[i].var[j];
        }

        q = algo_Quantize(p, q);
        algo_QuantizedRange range = q.FVarsRange[0];

        if (range.Depth != tests[i].depth) {
            fprintf(stderr, "In test %d of testFVarRange, expected Depth = %"
                    PRIu8", but got %"PRIu8".\n",
                    i, tests[i].depth, range.Depth);
            res = false;
        }

        if (!almostEqual(range.X0, tests[i].x0, (float)1e-4)) {
            fprintf(stderr, "In test %d of testFVarRange, expected X0 = %g, "
                    "but got %g.\n", i, tests[i].x0, range.X0);
            res = false;
        }

        if (!almostEqual(range.X1, tests[i].x1, (float)1e-4)) {
            fprintf(stderr, "In test %d of testFVarRange, expected X1 = %g, "
                    "but got %g.\n", i, tests[i].x1, range.X1);
            res = false;
        }

		Particles_Free(p);
    }

    /* Non-uniform delta tests. */

    struct {
        float deltas[8], var[8];
        int32_t len;
        float x0, x1;
        uint8_t depths[8];
    } testsNU[] = {
        /* Redo uniform tests to make sure nothing has been broken. */
        {{2}, {3}, 1, 3, 5, {0}},
        {{2, 2}, {3, 4}, 2, 3, 5, {0, 0}},
        {{2, 2}, {3, 5}, 2, 3, 7, {1, 1}},
        {{0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5},
         {1, 2, 3, 4, 5, 6, 8}, 7, 1, 9,
         {4, 4, 4, 4, 4, 4, 4}},
        {{2, 1}, {3, 4.75}, 2, 3, 5, {0, 1}},
        {{2, 1.5}, {3, 4.75}, 2, 3, 5, {0, 1}},
        {{2, 1.5}, {3, 5.75}, 2, 3, 6, {1, 1}}
    };

    for (int i = 0; i < LEN(testsNU); i++) {

        algo_Particles p;
        memset(&p, 0, sizeof(p));

        for (int j = 0; j < 3; j++) {
            p.X[j] = FSeq_New(testsNU[i].len);
        }
        p.XWidth = 10;
        p.XAcc.Delta = 1;

        p.FVarsAcc = calloc(1, sizeof(*p.FVarsAcc));
        p.FVarsAcc[0].Deltas = FSeq_FromArray(
            testsNU[i].deltas, testsNU[i].len
        );

        p.FVars = FSeqSeq_New(1);
        FSeq var = FSeq_New(testsNU[i].len);
        p.FVars.Data[0] = var;
        for (int32_t j = 0; j < var.Len; j++) {
            var.Data[j] = testsNU[i].var[j];
        }

        q = algo_Quantize(p, q);
        algo_QuantizedRange range = q.FVarsRange[0];

        for (int32_t j = 0; j < range.Depths.Len; j++) {
            if (range.Depths.Data[j] != testsNU[i].depths[j]) {
                fprintf(stderr, "In test %d of testFVarRange, expected "
                        "Depth[%d] = %"PRIu8", but got %"PRIu8".\n",
                        i, j, testsNU[i].depths[j], range.Depths.Data[j]);
                res = false;
            }
        }

        if (!almostEqual(range.X0, testsNU[i].x0, (float)1e-4)) {
            fprintf(stderr, "In test %d of testFVarRange, expected X0 = %g, "
                    "but got %g.\n", i, testsNU[i].x0, range.X0);
            res = false;
        }

        if (!almostEqual(range.X1, testsNU[i].x1, (float)1e-4)) {
            fprintf(stderr, "In test %d of testFVarRange, expected X1 = %g, "
                    "but got %g.\n", i, testsNU[i].x1, range.X1);
            res = false;
        }

		Particles_Free(p);
    }
	QuantizedParticles_Free(q);
	
    return res;
}

bool testVRange() {
    bool res = true;
    return res;
}

bool testXRange() {
    bool res = true;
    return res;
}

bool testIDRange() {
    bool res = true;
    return res;
}
