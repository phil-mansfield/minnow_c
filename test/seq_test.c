#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "seq.h"

/* Note: tests which are supossed to Panic are commented out. */

#define LEN(x) ((int) (sizeof(x) / sizeof(*x)))

int32_t refCount(ExSeq s) {
    if (s.Cap == 0) { return 0; }
    return *(int32_t*)(s.Data + s.Cap);
}

bool testNew() {
    bool res = true;

    ExSeq s = ExSeq_New(0);
    if (s.Len != 0 || s.Cap != 0) {
        res = false;
        fprintf(stderr, "Expected New(0) -> {Len: %"PRId32", Cap: %"
                PRId32"}.\n", s.Len, s.Cap);
    } else if (refCount(s) != 0) {
        res = false;
        fprintf(stderr, "Expected ref count = 0 for new seqeunce, got %"
                PRId32".\n", refCount(s));
    }

    ExSeq_Free(s);

    s = ExSeq_New(1 << 20);
    if (s.Len != 1 << 20 || s.Cap != 1 << 20) {
        res = false;
        fprintf(stderr, "Expected New(%"PRId32") -> {Len: %"PRId32", Cap: %"
                PRId32"}.\n", 1 << 20, s.Len, s.Cap);
    } else if (refCount(s) != 0) {
        res = false;
        fprintf(stderr, "Expected ref count = 0 for new seqeunce, got %"
                PRId32".\n", refCount(s));
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
    } else if (refCount(s) != 0) {
        res = false;
        fprintf(stderr, "Expected ref count = 0 for new seqeunce, got %"
                PRId32".\n", refCount(s));
    }

    ExSeq_Free(s);

    s = ExSeq_NewWithCap(10, 1 << 20);
    if (s.Len != 10 || s.Cap != 1 << 20) {
        res = false;
        fprintf(stderr, "Expected New(%"PRId32", %"PRId32") -> {Len: %"
                PRId32", Cap: %"PRId32"}.\n", 10, 1 << 20, s.Len, s.Cap);
    } else if (refCount(s) != 0) {
        res = false;
        fprintf(stderr, "Expected ref count = 0 for new seqeunce, got %"
                PRId32".\n", refCount(s));
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

    struct { int32_t len; Example data[8]; } tests[4] = {
        //{-1, {3}},
        {0, {}},
        {1, {3}},
        {8, {1, 2, 3, 4, 5, 6, 7, 8}}
    };

    for (int i = 0; i < LEN(tests); i++) {
        ExSeq s = ExSeq_FromArray(tests[i].data, tests[i].len);
        
        if (s.Len != tests[i].len || s.Cap != tests[i].len) {
            res = false;
            fprintf(stderr, "Expected test %d in testFromArray() to"
                    "return {Len: %"PRId32", Cap: %"PRId32"}.\n",
                    i, s.Len, s.Cap);
        }
        if (refCount(s) != 0) {
            res = false;
            fprintf(stderr, "Expected ref count = 0 for new seqeunce, got %"
                    PRId32".\n", refCount(s));
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

int main() {
    bool res = true;
    res = res && testNew();
    res = res && testNewWithCap();
    res = res && testFromArray();

    return !res;
}
