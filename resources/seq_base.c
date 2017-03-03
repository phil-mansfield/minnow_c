#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "seq.h"
#include "debug.h"

#define ALPHA 1.25

/**********************/
/* Exported Functions */
/**********************/

ExSeq ExSeq_New(int32_t len) {
    DebugAssert(len >= 0) {
        Panic("ExSeq_New given negative length, %"PRId32".", len);
    }


    int32_t cap = ((len / 4) + (len % 4 != 0))*4;
    ExSeq s = { .Data = NULL, .Len = len, .Cap = cap};
    if (!len) { return s; }

    

    /* The last four bytes are used to store the reference counter. */
    s.Data = malloc(cap*sizeof(*s.Data) + 4);
    AssertAlloc(s.Data);

    memset(s.Data, 0, cap*sizeof(*s.Data) + 4);

    return s;
}

ExSeq ExSeq_FromArray(Example *data, int32_t len) {
    DebugAssert(len >= 0) {
        Panic("ExSeq_FromArray given negative length, %"PRId32".", len);
    }

    ExSeq s = ExSeq_New(len);
    for (int32_t i = 0; i < len; i++) {
        s.Data[i] = data[i];
    }

    return s;
}

ExSeq ExSeq_NewWithCap(int32_t len, int32_t cap) {
    DebugAssert(len >= 0) {
        Panic("ExSeq_NewWithCap given negative length, %"PRId32".", len);
    }
    DebugAssert(cap >= len) {
        Panic("ExSeq_NewWithCap given cap, %"PRId32
              ", smaller than length, %"PRId32".", cap, len);
    }

    ExSeq s = ExSeq_New(cap);
    s.Len = len;
    return s;
}

void ExSeq_Deref(ExSeq s) {
    int32_t *refPtr = (int32_t*)(s.Data + s.Cap);
    if (!refPtr || !*refPtr) {
        ExSeq_Free(s);
        return;
    }
    (*refPtr)--;
}

void ExSeq_Free(ExSeq s) {
    free(s.Data);
    s.Data = NULL; /* To make errors easier to find. */
    return;
}

ExSeq ExSeq_Append(ExSeq s, Example tail) {
    if (s.Len == s.Cap) {
        /* Brainteaser: why do I do this? */
        int32_t *refPtr = (int32_t*)(void*)(s.Data + s.Cap);
        DebugAssert(!s.Data || *refPtr == 0) {
            Panic("%"PRId32" living references pointing to s1 in "
                  "to ExSeq_Join.", *refPtr);
        }

        s.Cap = (int32_t) (ALPHA * (float) (1 + s.Cap));
        s.Cap = ((s.Cap / 4) + (s.Cap % 4 != 0))*4;

        s.Data = realloc(s.Data, s.Cap*sizeof(*s.Data) + 4);
        /* Also resets the reference counter. */
        memset(s.Data + s.Len, 0, sizeof(*s.Data)*(s.Cap - s.Len) + 4);
    }

    s.Data[s.Len] = tail;
    s.Len++;

    return s;
}

ExSeq ExSeq_Join(ExSeq s1, ExSeq s2) {
    if (s1.Len + s2.Len  > s1.Cap) {
        int32_t *refPtr = (int32_t*)(void*)(s1.Data + s1.Cap);
        DebugAssert(!refPtr || *refPtr == 0) {
            Panic("%"PRId32" living references pointing to s1 in "
                  "to ExSeq_Join.", *refPtr);
        }

        s1.Cap = (int32_t) (ALPHA * (float) (s1.Len + s2.Len));
        s1.Cap = ((s1.Cap / 4) + (s1.Cap % 4 != 0))*4;

        s1.Data = realloc(s1.Data, s1.Cap*sizeof(*s1.Data) + 4);
        /* Also resets the reference counter. */
        memset(s1.Data + s1.Len, 0, s1.Cap - s1.Len + 4);
    }

    memcpy(s1.Data + s1.Len, s2.Data, s2.Len * sizeof(*s2.Data));
    s1.Len +=  s2.Len;

    return s1;
}

ExSeq ExSeq_Sub(ExSeq s, int32_t start, int32_t end) {
    DebugAssert(start >= 0) {
        Panic("ExSeq_Sub given negative start index, %"PRId32".", start);
    }
    DebugAssert(end >= start) {
        Panic("ExSeq_Sub given start index, %"PRId32
              " which is smaller than end index %"PRId32".", start, end);
    }
    DebugAssert(end <= s.Cap) {
        Panic("ExSeq_Sub given end index, %"PRId32
              " which is larger than cap %"PRId32".", end, s.Cap);
    }

    ExSeq sub;
    sub.Data = s.Data + start;
    sub.Len = end - start;
    sub.Cap = s.Cap - start;


    int32_t *refPtr = (int32_t*)(s.Data + s.Cap);
    if (!refPtr) { return sub; }
    (*refPtr)++;

    return sub;
}
