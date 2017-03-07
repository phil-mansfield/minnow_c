#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "compress_util.h"
#include "seq.h"
#include "rand.h"

#define LEN(x) (int) (sizeof(x) / sizeof(x[0]))

bool testU32TransposeBytes();
bool testU8DeltaEncode();
bool testBinIndex();
bool testUndoBinIndex();
bool testUniformBinIndex();
bool testUndoUniformBinIndex();

bool U8SeqEqual(U8Seq s1, U8Seq s2);
bool U32SeqEqual(U32Seq s1, U32Seq s2);

void U8SeqPrint(U8Seq s);
void U32SeqPrint(U32Seq s);

int main() {
    bool res = true;

    res = res && testU32TransposeBytes();
    res = res && testU8DeltaEncode();
    res = res && testBinIndex();
    res = res && testUndoBinIndex();
    res = res && testUniformBinIndex();
    res = res && testUndoUniformBinIndex();

    return !res;
}


bool testU32TransposeBytes() {
    bool res = true;

    /* manual tests */

    struct {
        uint32_t ints[4]; uint8_t bytes[16]; int32_t intLen, bufLen;
    } tests[] = {
        {{0}, {0}, 0, 0},
        {{0}, {0}, 0, 10},
        {{0xaabbccdd}, {0xdd, 0xcc, 0xbb, 0xaa}, 1, 0},
        {{0xaabbccdd}, {0xdd, 0xcc, 0xbb, 0xaa}, 1, 10},
        {{0xaabbccdd}, {0xdd, 0xcc, 0xbb, 0xaa}, 1, 2},
        {{0xaabbccdd}, {0xdd, 0xcc, 0xbb, 0xaa}, 1, 0},
        {{0xaabbccdd, 0x00112233, 0x44556677},
         {0xdd, 0x33, 0x77, 0xcc, 0x22, 0x66,
          0xbb, 0x11, 0x55, 0xaa, 0x00, 0x44}, 3, 0},
        {{0xaabbccdd, 0x00112233, 0x44556677},
         {0xdd, 0x33, 0x77, 0xcc, 0x22, 0x66,
          0xbb, 0x11, 0x55, 0xaa, 0x00, 0x44}, 3, 2},
        {{0xaabbccdd, 0x00112233, 0x44556677},
         {0xdd, 0x33, 0x77, 0xcc, 0x22, 0x66,
          0xbb, 0x11, 0x55, 0xaa, 0x00, 0x44}, 3, 10}
    };

    for (int i = 0; i < LEN(tests); i++) {
        U8Seq byteBuf = U8Seq_New(tests[i].bufLen);
        U32Seq intBuf = U32Seq_New(tests[i].bufLen);
        U8Seq byteInput = U8Seq_FromArray(tests[i].bytes, tests[i].intLen*4);
        U32Seq intInput = U32Seq_FromArray(tests[i].ints, tests[i].intLen);
        for (int32_t j = 0; j < byteBuf.Len; j++) {
            byteBuf.Data[j] = 8;
        }
        for (int32_t j = 0; j < intBuf.Len; j++) {
            intBuf.Data[j] = 8;
        }

        U8Seq byteOutput = util_U32TransposeBytes(intInput, byteBuf);
        U32Seq intOutput = util_U32UndoTransposeBytes(byteInput, intBuf);

        if (!U8SeqEqual(byteOutput, byteInput)) {
            fprintf(stderr, "In test %d of testTransposeBytes(), expected "
                    "output of TranposeBytes() to be ", i);
            U8SeqPrint(byteInput);
            fprintf(stderr, ", but got ");
            U8SeqPrint(byteOutput);
            fprintf(stderr, ".\n");
            res = false;
        } 
        if (!U32SeqEqual(intOutput, intInput)) {
            fprintf(stderr, "In test %d of testTransposeBytes(), expected "
                    "output of UndoTranposeBytes() to be ", i);
            U32SeqPrint(intInput);
            fprintf(stderr, ", but got ");
            U32SeqPrint(intOutput);
            fprintf(stderr, ".\n");
            res = false;
        } 

        U8Seq_Free(byteOutput);
        U8Seq_Free(byteInput);
        U32Seq_Free(intOutput);
        U32Seq_Free(intInput);
    }

    /* stress test */

    U32Seq s = U32Seq_New(1 << 14);
    rand_State *state = rand_Seed(0, 1);

    for (int32_t i = 0; i < s.Len; i++) {
        s.Data[i] = (uint32_t) rand_Uint64(state);
    }
    U8Seq t = util_U32TransposeBytes(s, U8Seq_Empty());
    U32Seq ss = util_U32UndoTransposeBytes(t, U32Seq_Empty());

    if (!U32SeqEqual(s, ss)) {
        fprintf(stderr, "util_TransposeBytesstress test failed.\n");
        res = false;
    }

    U32Seq_Free(s);
    U8Seq_Free(t);
    U32Seq_Free(ss);
    free(state);

    return res;
}

bool testU8DeltaEncode() {
    bool res = true;

    // bufFlag = 0 -> empty buffer
    // bufFlag = 1 -> buffer that's too small
    // bufFlag = 2 -> buffer that's too big
    // bufFlag = 3 -> Use self as buffer
    struct { uint8_t x[6], delta[6]; int32_t len, bufFlag; } tests[] = {
        {{0}, {0}, 0, 0},
        {{0}, {0}, 0, 2},
        {{0}, {0}, 0, 3},
        {{1}, {1}, 1, 0},
        {{1}, {1}, 1, 1},
        {{1}, {1}, 1, 2},
        {{1}, {1}, 1, 3},
        {{2, 1}, {2, 0xff}, 2, 0},
        {{1, 1, 2, 3, 5, 8}, {1, 0, 1, 1, 2, 3}, 6, 0},
        {{1, 1, 2, 3, 5, 8}, {1, 0, 1, 1, 2, 3}, 6, 1},
        {{1, 1, 2, 3, 5, 8}, {1, 0, 1, 1, 2, 3}, 6, 2},
        {{1, 1, 2, 3, 5, 8}, {1, 0, 1, 1, 2, 3}, 6, 3},
    };

    for (int32_t i = 0; i < LEN(tests); i++) {
        /* Encoding */

        U8Seq xSeq = U8Seq_FromArray(tests[i].x, tests[i].len);
        U8Seq buf;
        switch (tests[i].bufFlag) {
        case 0:
            buf = U8Seq_Empty();
            break;
        case 1:
            buf = U8Seq_New(xSeq.Len - 1);
            break;
        case 2:
            buf = U8Seq_New(xSeq.Len + 1);
            break;
        case 3:
            buf = xSeq;
        }

        U8Seq deltaSeq = U8Seq_FromArray(tests[i].delta, tests[i].len);
        U8Seq deltaRes = util_U8DeltaEncode(xSeq, buf);
        if (!U8SeqEqual(deltaSeq, deltaRes)) {
            fprintf(stderr, "In test %d of testU8DeltaEncode(", i);
            U8SeqPrint(xSeq);
            fprintf(stderr, "), expected util_U8DeltaEncode to return ");
            U8SeqPrint(deltaSeq);
            fprintf(stderr, ", but got ");
            U8SeqPrint(deltaRes);
            fprintf(stderr, "\n");
            res = false;
        }

        U8Seq_Free(deltaRes);
        if (tests[i].bufFlag != 3) {
           U8Seq_Free(xSeq);
        }

        /* Decoding */

        switch (tests[i].bufFlag) {
        case 0:
            buf = U8Seq_Empty();
            break;
        case 1:
            buf = U8Seq_New(xSeq.Len - 1);
            break;
        case 2:
            buf = U8Seq_New(xSeq.Len + 1);
            break;
        case 3:
            buf = deltaSeq;
        }

        xSeq = U8Seq_FromArray(tests[i].x, tests[i].len);
        U8Seq xRes = util_U8UndoDeltaEncode(deltaSeq, buf);
        if (!U8SeqEqual(xRes, xSeq)) {
            fprintf(stderr, "In test %d of testU8DeltaEncode(", i);
            U8SeqPrint(deltaSeq);
            fprintf(stderr, "), expected util_U8UndoDeltaEncode to return ");
            U8SeqPrint(deltaSeq);
            fprintf(stderr, ", but got ");
            U8SeqPrint(deltaRes);
            fprintf(stderr, "\n");
            res = false;
        }

        U8Seq_Free(xRes);
        U8Seq_Free(xSeq);
        if (tests[i].bufFlag != 3) {
           U8Seq_Free(deltaSeq);
        }
    }

    return res;
}

bool testBinIndex() {
    bool res = true;

    struct {
        float x[8];
        uint8_t level[8];
        uint32_t idx[8]; 
        int32_t len;
        float x0, dx;
    } tests[] = {
        {{0}, {0}, {0}, 0, 0, 1},
        {{0}, {0}, {0}, 1, 0, 1},
        {{0.25}, {1}, {0}, 1, 0, 1},
        {{0.75}, {1}, {1}, 1, 0, 1},
        {{0.5, 1.5, 3.5}, {2, 2, 2}, {0, 1, 3}, 3, 0, 4},
        {{3.5, 1.5, 0.5}, {2, 2, 2}, {3, 1, 0}, 3, 0, 4},
        {{3.5, 1.5, 0.5}, {2, 0, 2}, {3, 0, 0}, 3, 0, 4},
        {{3, 5, 7, 9}, {2, 2, 2, 2}, {0, 1, 2, 3}, 4, 2, 8},
        {{3, 5, 7, 9}, {3, 3, 3, 3}, {1, 3, 5, 7}, 4, 2, 8},
        {{-0.5, -1.5, -3.5}, {2, 2, 2}, {0, 1, 3}, 3, 0, -4}
    };

    for (int i = 0; i < LEN(tests); i++) {
        FSeq x = FSeq_FromArray(tests[i].x, tests[i].len);
        U8Seq level = U8Seq_FromArray(tests[i].level, tests[i].len);
        U32Seq idx = U32Seq_FromArray(tests[i].idx, tests[i].len);
        U32Seq buf = U32Seq_New(3);

        U32Seq out = util_BinIndex(x, level, tests[i].x0, tests[i].dx, buf);
        if (!U32SeqEqual(out, idx)) {
            fprintf(stderr, "In test %d of testBinIndex, expected "
                    "util_BinIndex to return ", i);
            U32SeqPrint(idx);
            fprintf(stderr, ", but got ");
            U32SeqPrint(out);
            fprintf(stderr, "\n");
            res = false;
        }

        FSeq_Free(x);
        U8Seq_Free(level);
        U32Seq_Free(idx);
        U32Seq_Free(buf);
    }

    return res;
}

bool testUndoBinIndex() {
    bool res = true;

    struct {
        uint32_t idx[8];
        uint8_t level[8];
        int32_t len;
        float x0, dx;
    } tests[] = {
        {{0}, {0}, 0, 0, 16},
        {{0}, {0}, 1, 0, 16},
        {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2, 3, 4, 5, 6, 7}, 8, -4, 8},
    };

    rand_State *state = rand_Seed(0, 1);

    for (int32_t i = 0; i < LEN(tests); i++) {
        U32Seq idx = U32Seq_FromArray(tests[i].idx, tests[i].len);
        U8Seq level = U8Seq_FromArray(tests[i].level, tests[i].len);
        FSeq buf = FSeq_New(3);

        FSeq x = util_UndoBinIndex(
            idx, level, tests[i].x0, tests[i].dx, state, buf
        );
        U32Seq idx2 = util_BinIndex(
            x, level, tests[i].x0, tests[i].dx, U32Seq_Empty()
        );

        if (!U32SeqEqual(idx, idx2)) {
            fprintf(stderr, "In test %d of testUndoBinIndex(), expected "
                    "output of util_BinIndex() to be ", i);
            U32SeqPrint(idx);
            fprintf(stderr, ", but got ");
            U32SeqPrint(idx2);
            fprintf(stderr, ".\n");
            res = false;            
        }

        FSeq_Free(x);
        U8Seq_Free(level);
        U32Seq_Free(idx2);
        U32Seq_Free(idx);
    }

    free(state);

    return res;
}

bool testUniformBinIndex() {
    bool res = true;

    struct {
        float x[8];
        uint8_t level;
        uint32_t idx[8]; 
        int32_t len;
        float x0, dx;
    } tests[] = {
        {{0}, 0, {0}, 0, 0, 1},
        {{0}, 0, {0}, 1, 0, 1},
        {{0.25}, 1, {0}, 1, 0, 1},
        {{0.75}, 1, {1}, 1, 0, 1},
        {{0.5, 1.5, 3.5}, 2, {0, 1, 3}, 3, 0, 4},
        {{3.5, 1.5, 0.5}, 2, {3, 1, 0}, 3, 0, 4},
        {{3, 5, 7, 9}, 2, {0, 1, 2, 3}, 4, 2, 8},
        {{3, 5, 7, 9}, 3, {1, 3, 5, 7}, 4, 2, 8},
        {{-0.5, -1.5, -3.5}, 2, {0, 1, 3}, 3, 0, -4}
    };

    for (int i = 0; i < LEN(tests); i++) {
        FSeq x = FSeq_FromArray(tests[i].x, tests[i].len);
        U32Seq idx = U32Seq_FromArray(tests[i].idx, tests[i].len);
        U32Seq buf = U32Seq_New(3);

        U32Seq out = util_UniformBinIndex(
            x, tests[i].level, tests[i].x0, tests[i].dx, buf
        );
        if (!U32SeqEqual(out, idx)) {
            fprintf(stderr, "In test %d of testBinIndex, expected "
                    "util_UniformBinIndex to return ", i);
            U32SeqPrint(idx);
            fprintf(stderr, ", but got ");
            U32SeqPrint(out);
            fprintf(stderr, "\n");
            res = false;
        }

        FSeq_Free(x);
        U32Seq_Free(idx);
        U32Seq_Free(buf);
    }

    return res;
}

bool testUndoUniformBinIndex() {
    return false;
}

/********************/
/* Helper Functions */
/********************/

bool U8SeqEqual(U8Seq s1, U8Seq s2) {
    if (s1.Len != s2.Len) {
        return false;
    }
    for (int32_t i = 0; i < s1.Len; i++) {
        if (s1.Data[i] != s2.Data[i]) {
            return false;
        }
    }
    return true;
}

bool U32SeqEqual(U32Seq s1, U32Seq s2) {
    if (s1.Len != s2.Len) {
        return false;
    }
    for (int32_t i = 0; i < s1.Len; i++) {
        if (s1.Data[i] != s2.Data[i]) {
            return false;
        }
    }
    return true;
}

void U8SeqPrint(U8Seq s) {
    if (s.Len == 0) {
        fprintf(stderr, "[]");
        return;
    }

    fprintf(stderr, "[");
    fprintf(stderr, "%"PRIx8, s.Data[0]);
    for (int32_t i = 1; i < s.Len; i++) {
        fprintf(stderr, ", %"PRIx8, s.Data[i]);
    }

    fprintf(stderr, "]");
}

void U32SeqPrint(U32Seq s) {
    if (s.Len == 0) {
        fprintf(stderr, "[]");
        return;
    }

    fprintf(stderr, "[");
    fprintf(stderr, "%"PRIx32, s.Data[0]);
    for (int32_t i = 1; i < s.Len; i++) {
        fprintf(stderr, ", %"PRIx32, s.Data[i]);
    }

    fprintf(stderr, "]");
}
