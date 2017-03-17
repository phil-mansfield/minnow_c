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
            fprintf(stderr, "In test %d of part 1 of testFVarRange, expected "
                    "Depth = %"PRIu8", but got %"PRIu8".\n",
                    i, tests[i].depth, range.Depth);
            res = false;
        }

        if (!almostEqual(range.X0, tests[i].x0, (float)1e-4)) {
            fprintf(stderr, "In test %d of part 1 of testFVarRange, expected "
                    "X0 = %g, but got %g.\n", i, tests[i].x0, range.X0);
            res = false;
        }

        if (!almostEqual(range.X1, tests[i].x1, (float)1e-4)) {
            fprintf(stderr, "In test %d of part 1 of testFVarRange, expected "
                    "X1 = %g, but got %g.\n", i, tests[i].x1, range.X1);
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
                fprintf(stderr, "In test %d of part 2 of testFVarRange, "
                        "expected Depth[%d] = %"PRIu8", but got %"PRIu8".\n",
                        i, j, testsNU[i].depths[j], range.Depths.Data[j]);
                res = false;
            }
        }

        if (!almostEqual(range.X0, testsNU[i].x0, (float)1e-4)) {
            fprintf(stderr, "In test %d of part 2 of testFVarRange, expected "
                    "X0 = %g, but got %g.\n", i, testsNU[i].x0, range.X0);
            res = false;
        }

        if (!almostEqual(range.X1, testsNU[i].x1, (float)1e-4)) {
            fprintf(stderr, "In test %d of part 2 of testFVarRange, expected "
                    "X1 = %g, but got %g.\n", i, testsNU[i].x1, range.X1);
            res = false;
        }

		Particles_Free(p);
    }
	QuantizedParticles_Free(q);
	
    return res;
}

bool testVRange() {
    bool res = true;

    struct {
        float delta, v[8][3];
        int32_t len;
        float x0[3], x1[3];
        uint8_t depth;
    } tests[] = {
        {2, {{3}, {3}, {3}}, 1, {3, 3, 3}, {5, 5, 5}, 0},
        {2, {{3, 4}, {3, 4}, {3, 4}}, 2, {3, 3, 3}, {5, 5, 5}, 0},
        {2, {{3, 5}, {3, 5}, {3, 5}}, 2, {3, 3, 3}, {7, 7, 7}, 1},
        {2, {{3, 4}, {4, 5}, {3, 4}}, 2, {3, 4, 3}, {5, 6, 5}, 0},
        {2, {{3, 4}, {4, 6}, {3, 4}}, 2, {3, 4, 3}, {7, 8, 7}, 1},
    };

	algo_QuantizedParticles q;
	memset(&q, 0, sizeof(q));

    for (int i = 0; i < LEN(tests); i++) {

        algo_Particles p;
        memset(&p, 0, sizeof(p));

        for (int j = 0; j < 3; j++) {
            p.X[j] = FSeq_New(tests[i].len);
            p.V[j] = FSeq_FromArray(tests[i].v[j], tests[i].len);
        }
        p.XWidth = 10;
        p.XAcc.Delta = 1;
        p.VAcc.Delta = tests[i].delta;

        q = algo_Quantize(p, q);
        algo_QuantizedVectorRange range = q.VRange;

        if (range.Depth != tests[i].depth) {
            fprintf(stderr, "In test %d of part 1 of testVRange, expected "
                    "Depth = %"PRIu8", but got %"PRIu8".\n",
                    i, tests[i].depth, range.Depth);
            res = false;
        }

        if (!almostEqual(range.X0[0], tests[i].x0[0], (float)1e-4) ||
            !almostEqual(range.X0[1], tests[i].x0[1], (float)1e-4) ||
            !almostEqual(range.X0[2], tests[i].x0[2], (float)1e-4)) {
            fprintf(stderr, "In test %d of part 1 of testVRange, expected "
                    "X0 = (%g, %g %g), but got (%g, %g %g).\n",
                    i, tests[i].x0[0], tests[i].x0[1], tests[i].x0[2],
                    range.X0[0], range.X0[1], range.X0[2]);
            res = false;
        }

        if (!almostEqual(range.X1[0], tests[i].x1[0], (float)1e-4) ||
            !almostEqual(range.X1[1], tests[i].x1[1], (float)1e-4) ||
            !almostEqual(range.X1[2], tests[i].x1[2], (float)1e-4)) {
            fprintf(stderr, "In test %d of part 1 of testVRange, expected "
                    "X1 = (%g, %g %g), but got (%g, %g, %g).\n",
                    i, tests[i].x1[0], tests[i].x1[1], tests[i].x1[2],
                    range.X1[0], range.X1[1], range.X1[2]);
            res = false;
        }

		Particles_Free(p);
    }

    struct {
        float deltas[8], v[8][3];
        int32_t len;
        float x0[3], x1[3];
        uint8_t depths[8];
    } testsNU[] = {
        {{2}, {{3}, {3}, {3}}, 1, {3, 3, 3}, {5, 5, 5}, {0}},
        {{2, 2}, {{3, 5}, {3, 5}, {3, 5}}, 2, {3, 3, 3}, {7, 7, 7}, {1, 1}},
        {{2, 2}, {{3, 4}, {3, 4}, {3, 4}}, 2, {3, 3, 3}, {5, 5, 5}, {0, 0}},
        {{2, 2}, {{3, 5}, {3, 5}, {3, 5}}, 2, {3, 3, 3}, {7, 7, 7}, {1, 1}},
        {{2, 2}, {{3, 4}, {4, 5}, {3, 4}}, 2, {3, 4, 3}, {5, 6, 5}, {0, 0}},
        {{2, 2}, {{3, 4}, {4, 6}, {3, 4}}, 2, {3, 4, 3}, {7, 8, 7}, {1, 1}},
        
        {{2, 1}, {{3, 4.75}, {3, 4.75}, {3, 4.75}}, 2,
         {3, 3, 3}, {5, 5, 5}, {0, 1}},

        {{2, 1.5}, {{3, 4.75}, {3, 4.75}, {3, 4.75}}, 2,
         {3, 3, 3}, {5, 5, 5}, {0, 1}},
        {{2, 1.5}, {{3, 5.75}, {3, 5.75}, {3, 5.75}}, 2,
         {3, 3, 3}, {6, 6, 6}, {1, 1}}
    };

    for (int i = 0; i < LEN(testsNU); i++) {
        algo_Particles p;
        memset(&p, 0, sizeof(p));

        for (int j = 0; j < 3; j++) {
            p.X[j] = FSeq_New(testsNU[i].len);
            p.V[j] = FSeq_FromArray(testsNU[i].v[j], testsNU[i].len);
        }
        p.XWidth = 10;
        p.XAcc.Delta = 1;
        p.VAcc.Deltas = FSeq_FromArray(testsNU[i].deltas, testsNU[i].len);

        q = algo_Quantize(p, q);
        algo_QuantizedVectorRange range = q.VRange;

        for (int32_t j = 0; j < range.Depths.Len; j++) {
            if (range.Depths.Data[j] != testsNU[i].depths[j]) {
                fprintf(stderr, "In test %d of part 2 of testVRange, expected"
                        "Depths[%"PRId32"] = %"PRIu8", but got %"PRIu8".\n",
                        i, j, testsNU[i].depths[j], range.Depths.Data[j]);
            }
        }

        if (!almostEqual(range.X0[0], testsNU[i].x0[0], (float)1e-4) ||
            !almostEqual(range.X0[1], testsNU[i].x0[1], (float)1e-4) ||
            !almostEqual(range.X0[2], testsNU[i].x0[2], (float)1e-4)) {
            fprintf(stderr, "In test %d of part 2 of testVRange, expected X0 = "
                    "(%g, %g %g), but got (%g, %g %g).\n",
                    i, testsNU[i].x0[0], testsNU[i].x0[1], testsNU[i].x0[2],
                    range.X0[0], range.X0[1], range.X0[2]);
            res = false;
        }

        if (!almostEqual(range.X1[0], testsNU[i].x1[0], (float)1e-4) ||
            !almostEqual(range.X1[1], testsNU[i].x1[1], (float)1e-4) ||
            !almostEqual(range.X1[2], testsNU[i].x1[2], (float)1e-4)) {
            fprintf(stderr, "In test %d of part 2 of testVRange, expected X1 = "
                    "(%g, %g %g), but got (%g, %g, %g).\n",
                    i, testsNU[i].x1[0], testsNU[i].x1[1], testsNU[i].x1[2],
                    range.X1[0], range.X1[1], range.X1[2]);
            res = false;
        }

		Particles_Free(p);
    }
	QuantizedParticles_Free(q);

    return res;
}

bool testXRange() {
    bool res = true;
    
    struct {
        float delta, x[8][3];
        int32_t len;
        float width, x0[3], x1[3];
        uint8_t depth;
    } tests[] = {
        {2, {{3}, {3}, {3}}, 1, 16, {2, 2, 2}, {4, 4, 4}, 0},
        {2, {{3, 5}, {3, 5}, {3, 5}}, 2, 16, {2, 2, 2}, {6, 6, 6}, 1},
        {2, {{3, 4}, {3, 4}, {3, 4}}, 2, 16, {2, 2, 2}, {4, 4, 4}, 0},
        {2, {{3, 5}, {3, 3.5}, {3, 3.5}}, 2, 16, {2, 2, 2}, {6, 6, 6}, 1},
        {2, {{3, 3.5}, {4, 4.5}, {3, 3.5}}, 2, 16, {2, 4, 2}, {4, 6, 4}, 0},
    };

	algo_QuantizedParticles q;
	memset(&q, 0, sizeof(q));

    for (int i = 0; i < LEN(tests); i++) {

        algo_Particles p;
        memset(&p, 0, sizeof(p));

        for (int j = 0; j < 3; j++) {
            p.X[j] = FSeq_FromArray(tests[i].x[j], tests[i].len);
        }
        p.XWidth = tests[i].width;
        p.XAcc.Delta = tests[i].delta;

        q = algo_Quantize(p, q);
        algo_QuantizedVectorRange range = q.XRange;

        if (range.Depth != tests[i].depth) {
            fprintf(stderr, "In test %d of part 1 of testXRange, expected "
                    "Depth = %"PRIu8", but got %"PRIu8".\n",
                    i, tests[i].depth, range.Depth);
            res = false;
        }

        if (!almostEqual(range.X0[0], tests[i].x0[0], (float)1e-4) ||
            !almostEqual(range.X0[1], tests[i].x0[1], (float)1e-4) ||
            !almostEqual(range.X0[2], tests[i].x0[2], (float)1e-4)) {
            fprintf(stderr, "In test %d of part 1 of testXRange, expected "
                    "X0 = (%g, %g %g), but got (%g, %g %g).\n",
                    i, tests[i].x0[0], tests[i].x0[1], tests[i].x0[2],
                    range.X0[0], range.X0[1], range.X0[2]);
            res = false;
        }

        if (!almostEqual(range.X1[0], tests[i].x1[0], (float)1e-4) ||
            !almostEqual(range.X1[1], tests[i].x1[1], (float)1e-4) ||
            !almostEqual(range.X1[2], tests[i].x1[2], (float)1e-4)) {
            fprintf(stderr, "In test %d of part 1 of testXRange, expected "
                    "X1 = (%g, %g %g), but got (%g, %g, %g).\n",
                    i, tests[i].x1[0], tests[i].x1[1], tests[i].x1[2],
                    range.X1[0], range.X1[1], range.X1[2]);
            res = false;
        }

		Particles_Free(p);
    }

    return res;
}

bool testIDRange() {
    bool res = true;
    return res;
}
