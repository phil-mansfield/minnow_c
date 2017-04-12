#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "base_seq.h"
#include "debug.h"

#define ALPHA 1.25

/**********************/
/* Exported Functions */
/**********************/

ExSeq ExSeq_Empty() {
    ExSeq s = {NULL, 0, 0};
    return s;
}

ExSeq ExSeq_New(int32_t len) {
    DebugAssert(len >= 0) {
        Panic("ExSeq_New given negative length, %"PRId32".", len);
    }

    int32_t cap = ((len / 8) + (len % 8 != 0))*8;
    ExSeq s = { .Data = NULL, .Len = len, .Cap = cap};
    if (!len) { return s; }    

    /* The last four bytes are used to store the underlying cap size. */
    s.Data = malloc((size_t)cap*sizeof(*s.Data) + 4);
    AssertAlloc(s.Data);

    memset(s.Data, 0, (size_t)cap*sizeof(*s.Data) + 4);
    int32_t *capPtr = (int32_t*)(void*)(s.Data + s.Cap);
    *capPtr = s.Cap;

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

void ExSeq_Free(ExSeq s) {
    if ( s.Data == NULL) { return; }
  
    int32_t *capPtr = (int32_t*)(void*)(s.Data + s.Cap);
    DebugAssert(!capPtr || *capPtr >= s.Cap) {
	    Panic("*capPtr = %"PRId32", but s.Cap = %"PRId32".", *capPtr, s.Cap);
    }
    free(s.Data - (*capPtr - s.Cap));
    s.Data = NULL; /* To make errors easier to find. */
    return;
}

ExSeq ExSeq_Append(ExSeq s, Example tail) {
    if (s.Len == s.Cap) {
        int32_t *capPtr = (int32_t*)(void*)(s.Data + s.Cap);
        DebugAssert(!capPtr || *capPtr == s.Cap) {
            Panic("Appending to a subsequence.%s", "");
        }

        s.Cap = (int32_t) (ALPHA * (float) (1 + s.Cap));
        s.Cap = ((s.Cap / 8) + (s.Cap % 8 != 0))*8;

        s.Data = realloc(s.Data, (size_t)s.Cap*sizeof(*s.Data) + 4);
        AssertAlloc(s.Data);
        
        memset(s.Data + s.Len, 0, sizeof(*s.Data)*(size_t)(s.Cap - s.Len) + 4);
        capPtr = (int32_t*)(void*)(s.Data + s.Cap);
        *capPtr = s.Cap;
    }

    s.Data[s.Len] = tail;
    s.Len++;

    return s;
}

ExSeq ExSeq_Join(ExSeq s1, ExSeq s2) {
    if (s1.Len + s2.Len  > s1.Cap) {
        int32_t *capPtr = (int32_t*)(void*)(s1.Data + s1.Cap);
        DebugAssert(!capPtr || *capPtr == s1.Cap) {
            Panic("Appending to a subsequence.%s", "");
        }

        s1.Cap = (int32_t) (ALPHA * (float) (s1.Len + s2.Len));
        s1.Cap = ((s1.Cap / 8) + (s1.Cap % 8 != 0))*8;

        s1.Data = realloc(s1.Data, (size_t)s1.Cap*sizeof(*s1.Data) + 4);
        AssertAlloc(s1.Data);

        memset(s1.Data + s1.Len, 0, (size_t)(s1.Cap - s1.Len + 4));
        capPtr = (int32_t*)(void*)(s1.Data + s1.Cap);
        *capPtr = s1.Cap;
    }

    memcpy(s1.Data + s1.Len, s2.Data, (size_t)s2.Len * sizeof(*s2.Data));
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

    return sub;
}

ExSeq ExSeq_Extend(ExSeq s, int32_t n) {
    DebugAssert(n >= 0) {
        Panic("ExSeq_Extend given negative cap size, %"PRId32".", n);
    }

    if (s.Cap >= n) { 
        return s;
    }

    s.Cap = ((n / 8) + (n % 8 != 0))*8;
    s.Data = realloc(s.Data, (size_t)s.Cap*sizeof(*s.Data) + 4);
    AssertAlloc(s.Data);
    memset(s.Data + s.Len, 0, sizeof(*s.Data)*(size_t)(s.Cap - s.Len) + 4);

    int32_t *capPtr = (int32_t*)(void*)(s.Data + s.Cap);
    *capPtr = s.Cap;
    return s;
}

ExBigSeq ExBigSeq_Empty() {
    ExBigSeq s = {NULL, 0, 0};
    return s;
}

ExBigSeq ExBigSeq_New(int64_t len) {
    DebugAssert(len >= 0) {
        Panic("ExBigSeq_New given negative length, %"PRId64".", len);
    }

    int64_t cap = ((len / 8) + (len % 8 != 0))*8;
    ExBigSeq s = { .Data = NULL, .Len = len, .Cap = cap};
    if (!len) { return s; }    

    /* The last four bytes are used to store the underlying cap size. */
    s.Data = malloc((size_t)cap*sizeof(*s.Data) + 4);
    AssertAlloc(s.Data);

    memset(s.Data, 0, (size_t)cap*sizeof(*s.Data) + 4);
    int64_t *capPtr = (int64_t*)(void*)(s.Data + s.Cap);
    *capPtr = s.Cap;

    return s;
}

ExBigSeq ExBigSeq_FromArray(Example *data, int64_t len) {
    DebugAssert(len >= 0) {
        Panic("ExBigSeq_FromArray given negative length, %"PRId64".", len);
    }

    ExBigSeq s = ExBigSeq_New(len);
    for (int64_t i = 0; i < len; i++) {
        s.Data[i] = data[i];
    }
    return s;
}

ExBigSeq ExBigSeq_NewWithCap(int64_t len, int64_t cap) {
    DebugAssert(len >= 0) {
        Panic("ExBigSeq_NewWithCap given negative length, %"PRId64".", len);
    }
    DebugAssert(cap >= len) {
        Panic("ExBigSeq_NewWithCap given cap, %"PRId64
              ", smaller than length, %"PRId64".", cap, len);
    }

    ExBigSeq s = ExBigSeq_New(cap);
    s.Len = len;
    return s;
}

void ExBigSeq_Free(ExBigSeq s) {
    if ( s.Data == NULL) { return; }
  
    int64_t *capPtr = (int64_t*)(void*)(s.Data + s.Cap);
    DebugAssert(!capPtr || *capPtr >= s.Cap) {
	    Panic("*capPtr = %"PRId64", but s.Cap = %"PRId64".", *capPtr, s.Cap);
    }
    free(s.Data - (*capPtr - s.Cap));
    s.Data = NULL; /* To make errors easier to find. */
    return;
}

ExBigSeq ExBigSeq_Append(ExBigSeq s, Example tail) {
    if (s.Len == s.Cap) {
        int64_t *capPtr = (int64_t*)(void*)(s.Data + s.Cap);
        DebugAssert(!capPtr || *capPtr == s.Cap) {
            Panic("Appending to a subsequence.%s", "");
        }

        s.Cap = (int64_t) (ALPHA * (float) (1 + s.Cap));
        s.Cap = ((s.Cap / 8) + (s.Cap % 8 != 0))*8;

        s.Data = realloc(s.Data, (size_t)s.Cap*sizeof(*s.Data) + 4);
        AssertAlloc(s.Data);
        
        memset(s.Data + s.Len, 0, sizeof(*s.Data)*(size_t)(s.Cap - s.Len) + 4);
        capPtr = (int64_t*)(void*)(s.Data + s.Cap);
        *capPtr = s.Cap;
    }

    s.Data[s.Len] = tail;
    s.Len++;

    return s;
}

ExBigSeq ExBigSeq_Join(ExBigSeq s1, ExBigSeq s2) {
    if (s1.Len + s2.Len  > s1.Cap) {
        int64_t *capPtr = (int64_t*)(void*)(s1.Data + s1.Cap);
        DebugAssert(!capPtr || *capPtr == s1.Cap) {
            Panic("Appending to a subsequence.%s", "");
        }

        s1.Cap = (int64_t) (ALPHA * (float) (s1.Len + s2.Len));
        s1.Cap = ((s1.Cap / 8) + (s1.Cap % 8 != 0))*8;

        s1.Data = realloc(s1.Data, (size_t)s1.Cap*sizeof(*s1.Data) + 4);
        AssertAlloc(s1.Data);

        memset(s1.Data + s1.Len, 0, (size_t)(s1.Cap - s1.Len + 4));
        capPtr = (int64_t*)(void*)(s1.Data + s1.Cap);
        *capPtr = s1.Cap;
    }

    memcpy(s1.Data + s1.Len, s2.Data, (size_t)s2.Len * sizeof(*s2.Data));
    s1.Len +=  s2.Len;

    return s1;
}

ExBigSeq ExBigSeq_Sub(ExBigSeq s, int64_t start, int64_t end) {
    DebugAssert(start >= 0) {
        Panic("ExBigSeq_Sub given negative start index, %"PRId64".", start);
    }
    DebugAssert(end >= start) {
        Panic("ExBigSeq_Sub given start index, %"PRId64
              " which is smaller than end index %"PRId64".", start, end);
    }
    DebugAssert(end <= s.Cap) {
        Panic("ExBigSeq_Sub given end index, %"PRId64
              " which is larger than cap %"PRId64".", end, s.Cap);
    }

    ExBigSeq sub;
    sub.Data = s.Data + start;
    sub.Len = end - start;
    sub.Cap = s.Cap - start;

    return sub;
}

ExBigSeq ExBigSeq_Extend(ExBigSeq s, int64_t n) {
    DebugAssert(n >= 0) {
        Panic("ExBigSeq_Extend given negative cap size, %"PRId64".", n);
    }

    if (s.Cap >= n) { 
        return s;
    }

    s.Cap = ((n / 8) + (n % 8 != 0))*8;
    s.Data = realloc(s.Data, (size_t)s.Cap*sizeof(*s.Data) + 4);
    AssertAlloc(s.Data);
    memset(s.Data + s.Len, 0, sizeof(*s.Data)*(size_t)(s.Cap - s.Len) + 4);

    int64_t *capPtr = (int64_t*)(void*)(s.Data + s.Cap);
    *capPtr = s.Cap;
    return s;
}


#define GENERATE_SEQ_BODY(type, seqType, bigSeqType) \
    seqType seqType##_Empty() { \
        seqType s = {NULL, 0, 0}; \
        return s; \
    } \
    seqType seqType##_New(int32_t len) { \
        DebugAssert(len >= 0) { \
            Panic(""#seqType"_New given negative length, %"PRId32".", len); \
        } \
        int32_t cap = ((len / 8) + (len % 8 != 0))*8; \
        seqType s = { .Data = NULL, .Len = len, .Cap = cap}; \
        if (!len) { return s; }     \
        s.Data = malloc((size_t)cap*sizeof(*s.Data) + 4); \
        AssertAlloc(s.Data); \
        memset(s.Data, 0, (size_t)cap*sizeof(*s.Data) + 4); \
        int32_t *capPtr = (int32_t*)(void*)(s.Data + s.Cap); \
        *capPtr = s.Cap; \
        return s; \
    } \
    seqType seqType##_FromArray(type *data, int32_t len) { \
        DebugAssert(len >= 0) { \
            Panic(""#seqType"_FromArray given negative length, %"PRId32".", len); \
        } \
        seqType s = seqType##_New(len); \
        for (int32_t i = 0; i < len; i++) { \
            s.Data[i] = data[i]; \
        } \
        return s; \
    } \
    seqType seqType##_NewWithCap(int32_t len, int32_t cap) { \
        DebugAssert(len >= 0) { \
            Panic(""#seqType"_NewWithCap given negative length, %"PRId32".", len); \
        } \
        DebugAssert(cap >= len) { \
            Panic(""#seqType"_NewWithCap given cap, %"PRId32 \
                  ", smaller than length, %"PRId32".", cap, len); \
        } \
        seqType s = seqType##_New(cap); \
        s.Len = len; \
        return s; \
    } \
    void seqType##_Free(seqType s) { \
        if ( s.Data == NULL) { return; } \
        int32_t *capPtr = (int32_t*)(void*)(s.Data + s.Cap); \
        DebugAssert(!capPtr || *capPtr >= s.Cap) { \
    	    Panic("*capPtr = %"PRId32", but s.Cap = %"PRId32".", *capPtr, s.Cap); \
        } \
        free(s.Data - (*capPtr - s.Cap)); \
        s.Data = NULL; /* To make errors easier to find. */ \
        return; \
    } \
    seqType seqType##_Append(seqType s, type tail) { \
        if (s.Len == s.Cap) { \
            int32_t *capPtr = (int32_t*)(void*)(s.Data + s.Cap); \
            DebugAssert(!capPtr || *capPtr == s.Cap) { \
                Panic("Appending to a subsequence.%s", ""); \
            } \
            s.Cap = (int32_t) (ALPHA * (float) (1 + s.Cap)); \
            s.Cap = ((s.Cap / 8) + (s.Cap % 8 != 0))*8; \
            s.Data = realloc(s.Data, (size_t)s.Cap*sizeof(*s.Data) + 4); \
            AssertAlloc(s.Data); \
            memset(s.Data + s.Len, 0, sizeof(*s.Data)*(size_t)(s.Cap - s.Len) + 4); \
            capPtr = (int32_t*)(void*)(s.Data + s.Cap); \
            *capPtr = s.Cap; \
        } \
        s.Data[s.Len] = tail; \
        s.Len++; \
        return s; \
    } \
    seqType seqType##_Join(seqType s1, seqType s2) { \
        if (s1.Len + s2.Len  > s1.Cap) { \
            int32_t *capPtr = (int32_t*)(void*)(s1.Data + s1.Cap); \
            DebugAssert(!capPtr || *capPtr == s1.Cap) { \
                Panic("Appending to a subsequence.%s", ""); \
            } \
            s1.Cap = (int32_t) (ALPHA * (float) (s1.Len + s2.Len)); \
            s1.Cap = ((s1.Cap / 8) + (s1.Cap % 8 != 0))*8; \
            s1.Data = realloc(s1.Data, (size_t)s1.Cap*sizeof(*s1.Data) + 4); \
            AssertAlloc(s1.Data); \
            memset(s1.Data + s1.Len, 0, (size_t)(s1.Cap - s1.Len + 4)); \
            capPtr = (int32_t*)(void*)(s1.Data + s1.Cap); \
            *capPtr = s1.Cap; \
        } \
        memcpy(s1.Data + s1.Len, s2.Data, (size_t)s2.Len * sizeof(*s2.Data)); \
        s1.Len +=  s2.Len; \
        return s1; \
    } \
    seqType seqType##_Sub(seqType s, int32_t start, int32_t end) { \
        DebugAssert(start >= 0) { \
            Panic(""#seqType"_Sub given negative start index, %"PRId32".", start); \
        } \
        DebugAssert(end >= start) { \
            Panic(""#seqType"_Sub given start index, %"PRId32 \
                  " which is smaller than end index %"PRId32".", start, end); \
        } \
        DebugAssert(end <= s.Cap) { \
            Panic(""#seqType"_Sub given end index, %"PRId32 \
                  " which is larger than cap %"PRId32".", end, s.Cap); \
        } \
        seqType sub; \
        sub.Data = s.Data + start; \
        sub.Len = end - start; \
        sub.Cap = s.Cap - start; \
        return sub; \
    } \
    seqType seqType##_Extend(seqType s, int32_t n) { \
        DebugAssert(n >= 0) { \
            Panic(""#seqType"_Extend given negative cap size, %"PRId32".", n); \
        } \
        if (s.Cap >= n) {  \
            return s; \
        } \
        s.Cap = ((n / 8) + (n % 8 != 0))*8; \
        s.Data = realloc(s.Data, (size_t)s.Cap*sizeof(*s.Data) + 4); \
        AssertAlloc(s.Data); \
        memset(s.Data + s.Len, 0, sizeof(*s.Data)*(size_t)(s.Cap - s.Len) + 4); \
        int32_t *capPtr = (int32_t*)(void*)(s.Data + s.Cap); \
        *capPtr = s.Cap; \
        return s; \
    } \
    bigSeqType bigSeqType##_Empty() { \
        bigSeqType s = {NULL, 0, 0}; \
        return s; \
    } \
    bigSeqType bigSeqType##_New(int64_t len) { \
        DebugAssert(len >= 0) { \
            Panic(""#bigSeqType"_New given negative length, %"PRId64".", len); \
        } \
        int64_t cap = ((len / 8) + (len % 8 != 0))*8; \
        bigSeqType s = { .Data = NULL, .Len = len, .Cap = cap}; \
        if (!len) { return s; }     \
        s.Data = malloc((size_t)cap*sizeof(*s.Data) + 4); \
        AssertAlloc(s.Data); \
        memset(s.Data, 0, (size_t)cap*sizeof(*s.Data) + 4); \
        int64_t *capPtr = (int64_t*)(void*)(s.Data + s.Cap); \
        *capPtr = s.Cap; \
        return s; \
    } \
    bigSeqType bigSeqType##_FromArray(type *data, int64_t len) { \
        DebugAssert(len >= 0) { \
            Panic(""#bigSeqType"_FromArray given negative length, %"PRId64".", len); \
        } \
        bigSeqType s = bigSeqType##_New(len); \
        for (int64_t i = 0; i < len; i++) { \
            s.Data[i] = data[i]; \
        } \
        return s; \
    } \
    bigSeqType bigSeqType##_NewWithCap(int64_t len, int64_t cap) { \
        DebugAssert(len >= 0) { \
            Panic(""#bigSeqType"_NewWithCap given negative length, %"PRId64".", len); \
        } \
        DebugAssert(cap >= len) { \
            Panic(""#bigSeqType"_NewWithCap given cap, %"PRId64 \
                  ", smaller than length, %"PRId64".", cap, len); \
        } \
        bigSeqType s = bigSeqType##_New(cap); \
        s.Len = len; \
        return s; \
    } \
    void bigSeqType##_Free(bigSeqType s) { \
        if ( s.Data == NULL) { return; } \
        int64_t *capPtr = (int64_t*)(void*)(s.Data + s.Cap); \
        DebugAssert(!capPtr || *capPtr >= s.Cap) { \
    	    Panic("*capPtr = %"PRId64", but s.Cap = %"PRId64".", *capPtr, s.Cap); \
        } \
        free(s.Data - (*capPtr - s.Cap)); \
        s.Data = NULL; /* To make errors easier to find. */ \
        return; \
    } \
    bigSeqType bigSeqType##_Append(bigSeqType s, type tail) { \
        if (s.Len == s.Cap) { \
            int64_t *capPtr = (int64_t*)(void*)(s.Data + s.Cap); \
            DebugAssert(!capPtr || *capPtr == s.Cap) { \
                Panic("Appending to a subsequence.%s", ""); \
            } \
            s.Cap = (int64_t) (ALPHA * (float) (1 + s.Cap)); \
            s.Cap = ((s.Cap / 8) + (s.Cap % 8 != 0))*8; \
            s.Data = realloc(s.Data, (size_t)s.Cap*sizeof(*s.Data) + 4); \
            AssertAlloc(s.Data); \
            memset(s.Data + s.Len, 0, sizeof(*s.Data)*(size_t)(s.Cap - s.Len) + 4); \
            capPtr = (int64_t*)(void*)(s.Data + s.Cap); \
            *capPtr = s.Cap; \
        } \
        s.Data[s.Len] = tail; \
        s.Len++; \
        return s; \
    } \
    bigSeqType bigSeqType##_Join(bigSeqType s1, bigSeqType s2) { \
        if (s1.Len + s2.Len  > s1.Cap) { \
            int64_t *capPtr = (int64_t*)(void*)(s1.Data + s1.Cap); \
            DebugAssert(!capPtr || *capPtr == s1.Cap) { \
                Panic("Appending to a subsequence.%s", ""); \
            } \
            s1.Cap = (int64_t) (ALPHA * (float) (s1.Len + s2.Len)); \
            s1.Cap = ((s1.Cap / 8) + (s1.Cap % 8 != 0))*8; \
            s1.Data = realloc(s1.Data, (size_t)s1.Cap*sizeof(*s1.Data) + 4); \
            AssertAlloc(s1.Data); \
            memset(s1.Data + s1.Len, 0, (size_t)(s1.Cap - s1.Len + 4)); \
            capPtr = (int64_t*)(void*)(s1.Data + s1.Cap); \
            *capPtr = s1.Cap; \
        } \
        memcpy(s1.Data + s1.Len, s2.Data, (size_t)s2.Len * sizeof(*s2.Data)); \
        s1.Len +=  s2.Len; \
        return s1; \
    } \
    bigSeqType bigSeqType##_Sub(bigSeqType s, int64_t start, int64_t end) { \
        DebugAssert(start >= 0) { \
            Panic(""#bigSeqType"_Sub given negative start index, %"PRId64".", start); \
        } \
        DebugAssert(end >= start) { \
            Panic(""#bigSeqType"_Sub given start index, %"PRId64 \
                  " which is smaller than end index %"PRId64".", start, end); \
        } \
        DebugAssert(end <= s.Cap) { \
            Panic(""#bigSeqType"_Sub given end index, %"PRId64 \
                  " which is larger than cap %"PRId64".", end, s.Cap); \
        } \
        bigSeqType sub; \
        sub.Data = s.Data + start; \
        sub.Len = end - start; \
        sub.Cap = s.Cap - start; \
        return sub; \
    } \
    bigSeqType bigSeqType##_Extend(bigSeqType s, int64_t n) { \
        DebugAssert(n >= 0) { \
            Panic(""#bigSeqType"_Extend given negative cap size, %"PRId64".", n); \
        } \
        if (s.Cap >= n) {  \
            return s; \
        } \
        s.Cap = ((n / 8) + (n % 8 != 0))*8; \
        s.Data = realloc(s.Data, (size_t)s.Cap*sizeof(*s.Data) + 4); \
        AssertAlloc(s.Data); \
        memset(s.Data + s.Len, 0, sizeof(*s.Data)*(size_t)(s.Cap - s.Len) + 4); \
        int64_t *capPtr = (int64_t*)(void*)(s.Data + s.Cap); \
        *capPtr = s.Cap; \
        return s; \
    }

