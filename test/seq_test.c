#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "seq.h"

/* Note: tests which are supossed to Panic are commented out. */

#define LEN(x) ((int) (sizeof(x) / sizeof(*x)))

int32_t refCount(ExSeq s);
bool testNew();
bool testNewWithCap();
bool testFromArray();
bool testSub();
bool testAppend();
bool testJoin();
bool testExtend();

int main() {
    bool res = true;
    /* Note that this short-circuits later tests if earlier tests fail. */
    res = res && testNew();
    res = res && testNewWithCap();
    res = res && testFromArray();
    res = res && testSub();
    res = res && testAppend();
    res = res && testJoin();
    res = res && testExtend();

    return !res;
}


bool testNew() {
    bool res = true;

    ExSeq s = ExSeq_New(0);
    if (s.Len != 0 || s.Cap != 0) {
        res = false;
        fprintf(stderr, "New(0) -> {Len: %"PRId32", Cap: %"
                PRId32"}.\n", s.Len, s.Cap);
    }

    ExSeq_Free(s);

    s = ExSeq_New(3);
    if (s.Len != 3 || s.Cap < 3) {
        res = false;
        fprintf(stderr, "New(3) -> {Len: %"PRId32", Cap: %"
                PRId32"}.\n", s.Len, s.Cap);
    }

    ExSeq_Free(s);

    s = ExSeq_New(1 << 20);
    if (s.Len != 1 << 20 || s.Cap != 1 << 20) {
        res = false;
        fprintf(stderr, "New(%"PRId32") -> {Len: %"PRId32", Cap: %"
                PRId32"}.\n", 1 << 20, s.Len, s.Cap);
    }

    for (int32_t i = 0; i < s.Len; i++) {
        if (s.Data[i] != 0) {
            res = false;
            fprintf(stderr, "Element %"PRId32" of New(%"PRId32
                    ") set to %g, not 0.\n", i, 1 << 20, s.Data[i]);
            break;
        }
    }

    ExSeq_Free(s);

    /* Calls that should crash: */

    // ExSeq_New(-1);

    return res;
}

bool testNewWithCap() {
    bool res = true;

    ExSeq s = ExSeq_NewWithCap(0, 0);
    if (s.Len != 0 || s.Cap != 0) {
        res = false;
        fprintf(stderr, "Expected NewWithCap(0, 0) -> {Len: %"PRId32", Cap: %"
                PRId32"}.\n", s.Len, s.Cap);
    }

    ExSeq_Free(s);

    s = ExSeq_NewWithCap(0, 3);
    if (s.Len != 0 || s.Cap < 3) {
        res = false;
        fprintf(stderr, "Expected NewWithCap(0, 3) -> {Len: %"PRId32", Cap: %"
                PRId32"}.\n", s.Len, s.Cap);
    }

    ExSeq_Free(s);

    s = ExSeq_NewWithCap(10, 1 << 20);
    if (s.Len != 10 || s.Cap != 1 << 20) {
        res = false;
        fprintf(stderr, "Expected New(%"PRId32", %"PRId32") -> {Len: %"
                PRId32", Cap: %"PRId32"}.\n", 10, 1 << 20, s.Len, s.Cap);
    }

    for (int32_t i = 0; i < s.Cap; i++) {
        if (s.Data[i] != 0) {
            res = false;
            fprintf(stderr, "Element %"PRId32" of NewWithCap(%"PRId32", %"
                    PRId32") set to %g, not 0.\n", i, 10, 1 << 20, s.Data[i]);
            break;
        }
    }

    ExSeq_Free(s);

    // ExSeq_NewWithCap(-1, 10);
    // ExSeq_NewWithCap(20, 10);

    return res;
}

bool testFromArray() {
    bool res = true;

    struct { double data[8]; int32_t len; } tests[4] = {
        //{{3}, -1},
        {{0}, 0},
        {{3}, 1},
        {{1, 2, 3, 4, 5, 6, 7, 8}, 8}
    };

    for (int i = 0; i < LEN(tests); i++) {
        ExSeq s = ExSeq_FromArray(tests[i].data, tests[i].len);
        
        if (s.Len != tests[i].len || s.Cap < tests[i].len) {
            res = false;
            fprintf(stderr, "Expected test %d in testFromArray() to "
                    "return {Len: %"PRId32", Cap: %"PRId32"}.\n",
                    i, s.Len, s.Cap);
        }
        for (int32_t j = 0; j < s.Len; j++) {
            if (s.Data[j] != tests[i].data[j]) {
                res = false;
                fprintf(stderr, "Element %"PRId32" of test %d in "
                        "testFromArray() is %g, not %g.\n", j, i,
                        s.Data[j], tests[i].data[j]);
                break;
            }
        }

        ExSeq_Free(s);
    }
    
    return res;
}

bool testSub() {
    double data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    ExSeq s = ExSeq_FromArray(data, LEN(data));
    s.Len = 4;

    struct {int32_t start, end, subLen, subCap;
        double sub[8]; ExSeq out;} tests[4] = {
        //{-1, 0, 0, 0, {0}, ExSeq_Empty()},
        //{0, 9, 0, 0, {0}, ExSeq_Empty()},
        //{3, 0, 0, 0, {0}, ExSeq_Empty()},
        {0, 4, 4, 8, {1, 2, 3, 4}, ExSeq_Empty()},
        {0, 0, 0, 8, {0}, ExSeq_Empty()},
        {0, 8, 8, 8, {1, 2, 3, 4, 5, 6, 7, 8}, ExSeq_Empty()},
        {2, 5, 3, 6, {3, 4, 5}, ExSeq_Empty()},
    };

    bool res = true;

    for (int i = 0; i < LEN(tests); i++) {
        ExSeq out = ExSeq_Sub(s, tests[i].start, tests[i].end);

        if (out.Len != tests[i].subLen) {
            res = false;
            fprintf(stderr, "Expected test %d of testSub() to have Len=%"
                    PRId32", but got Len=%"PRId32".\n",
                    i, tests[i].subLen, out.Len);
        }

        for (int32_t j = 0; j < out.Len; j++) {
            if (out.Data[j] !=  tests[i].sub[j]) {
                fprintf(stderr, "Expected test %d of testSub() to have %g at"
                        "index %"PRId32", but got %g.\n",
                        i, tests[i].sub[j], j, out.Data[j]);
                break;
            }
        }
        
        tests[i].out = out;
    }
    
    ExSeq_Free(tests[3].out);

    return res;
}

bool testAppend() {
    struct { int n, baseLen; } tests[7] = {
        {0, 8},
        {1, 8},
        {2, 8},
        {4, 8},
        {1 << 10, 8},
        {1 << 10, 1},
        {1 << 10, 0},
    };
    bool res = true;

    for (int i = 0; i < LEN(tests); i++) {
        double base[8] = {3, 3, 3, 3, 3, 3, 3, 3};
        ExSeq s = ExSeq_FromArray(base, tests[i].baseLen);

        for (int j = 0; j < tests[i].n; j++) {
            s = ExSeq_Append(s, 4);
        }

        if (s.Len != tests[i].n + tests[i].baseLen) {
            fprintf(stderr, "Expected test %d of testAppend() to have length "
                    "%d, but got %"PRId32".\n", i, 8 + tests[i].n, s.Len);
            res = false;
        }

        for (int j = 0; j < s.Len; j++) {
            if(j < tests[i].baseLen && s.Data[j] != 3) {
                fprintf(stderr, "Expected element %d of test %d in testAppend "
                        "to be %g, but got %g.\n", j, i, 3.0, s.Data[j]);
                res = false;
                break;
            }

            if(j >= tests[i].baseLen && s.Data[j] != 4) {
                fprintf(stderr, "Expected element %d of test %d in testAppend "
                        "to be %g, but got %g.\n", j, i, 4.0, s.Data[j]);
                res = false;
                break;
            }
        }

        ExSeq_Free(s);
    }

    return res;
}

bool testJoin() {
    struct {
        double data1[8], data2[8];
        int32_t n1, n2;
        int32_t cap1;
    } tests[11] = {
        { {0}, {0}, 0, 0, 0 },
        { {1, 1}, {0}, 2, 0, 2 },

        { {1}, {2}, 1, 1, 1 },
        { {1, 1, 1}, {2, 2}, 3, 2, 3 },
        { {1, 1}, {2, 2, 2}, 2, 3, 2 },

        { {0}, {2, 2}, 0, 2, 0},

        { {0}, {0}, 0, 0, 8 },
        { {0}, {2, 2}, 0, 2, 8},

        { {1, 1}, {0}, 2, 0, 8 },
        { {1, 1}, {2, 2}, 2, 2, 8},

        { {1, 1}, {2, 2, 2}, 2, 3, 4},
    };

    bool res = true;

    for (int i = 0; i < LEN(tests); i++) {
        ExSeq s1 = ExSeq_NewWithCap(tests[i].n1, tests[i].cap1);
        for (int32_t j = 0; j < s1.Len; j++) {
            s1.Data[j] = tests[i].data1[j];
        }
        ExSeq s2 = ExSeq_FromArray(tests[i].data2, tests[i].n2);

        s1 = ExSeq_Join(s1, s2);
        
        if (s1.Len != tests[i].n1 + tests[i].n2 ) {
            fprintf(stderr, "For test %d of testJoin(), expected length %"
                    PRId32", but got %"PRId32".\n", i, 
                    tests[i].n1 + tests[i].n2, s1.Len);
            res = false;
        }

        int32_t j = 0;
        for (; j < tests[j].n1; j++) {
            if (s1.Data[j] != 1) {
                fprintf(stderr, "For index %"PRId32" of test %d of testJoin(),"
                        "expected %g, but got %g.\n", j, i, 1.0, s1.Data[j]);
            }
        }

        ExSeq_Free(s1);
        ExSeq_Free(s2);
    }

    return res;
}

bool testExtend() {
    bool res = true;

    struct {int32_t len, cap, newCap; } tests[] = {
        //{0, 0, -1},
        {0, 0, 0},
        {1, 1, 0},
        {1, 4, 0},
        {4, 4, 0},
        {4, 4, 4},
        {2, 4, 4},
        {2, 3, 4},
        {2, 1<<10, 1<<14},
    };

    for (int i = 0; i < LEN(tests); i++) {
        ExSeq s = ExSeq_NewWithCap(tests[i].len, tests[i].cap);
        s = ExSeq_Extend(s, tests[i].newCap);

        if (s.Cap < tests[i].newCap) {
            res = false;
            fprintf(stderr, "Test %d of testExtend() gives cap size of %"
                    PRId32".", i, s.Cap);
        }

        ExSeq_Free(s);
    }

    return res;
}
