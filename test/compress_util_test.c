#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "util.h"
#include "seq.h"
#include "rand.h"

#define LEN(x) (int) (sizeof(x) / sizeof(x[0]))
#define MIN(x, y) ((x) < (y)? (x): (y))

bool testU32TransposeBytes();
bool testU8DeltaEncode();
bool testBinIndex();
bool testUndoBinIndex();
bool testUniformBinIndex();
bool testUndoUniformBinIndex();
bool testU32UniformPack();
bool testU32UndoPeriodic();
bool testEntropyEncode();
bool testFastUniformCompress();
bool testLittleEndian();

bool U8SeqEqual(U8Seq s1, U8Seq s2);
bool U32SeqEqual(U32Seq s1, U32Seq s2);
bool U64SeqEqual(U64Seq s1, U64Seq s2);
bool FSeqAlmostEqual(FSeq x1, FSeq x2, float eps, float L);

void U8SeqPrint(U8Seq s);
void U32SeqPrint(U32Seq s);
void U64SeqPrint(U64Seq s);
void FSeqPrint(FSeq s);

void FShuffle(FSeq x, rand_State *state);

int main() {
    bool res = true;

    res = res && testU32TransposeBytes();
    res = res && testU8DeltaEncode();
    res = res && testBinIndex();
    res = res && testUndoBinIndex();
    res = res && testUniformBinIndex();
    res = res && testUndoUniformBinIndex();
    res = res && testU32UniformPack();
    res = res && testU32UndoPeriodic();
    res = res && testEntropyEncode();
    res = res && testFastUniformCompress();
    res = res && testLittleEndian();

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
        {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2, 3, 4, 4, 4, 4}, 8, -4, 8},
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
        U32Seq_Free(idx2);
        U32Seq_Free(idx);
        U8Seq_Free(level);
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
    bool res = true;

    struct {
        uint32_t idx[8];
        uint8_t level;
        int32_t len;
        float x0, dx;
    } tests[] = {
        {{0}, 0, 0, 0, 16},
        {{0}, 0, 1, 0, 16},
        {{0, 1, 2, 3, 4, 5, 6, 7}, 4, 8, -4, 8},
    };

    rand_State *state = rand_Seed(0, 1);

    for (int32_t i = 0; i < LEN(tests); i++) {
        U32Seq idx = U32Seq_FromArray(tests[i].idx, tests[i].len);
        FSeq buf = FSeq_New(3);

        FSeq x = util_UndoUniformBinIndex(
            idx, tests[i].level, tests[i].x0, tests[i].dx, state, buf
        );
        U32Seq idx2 = util_UniformBinIndex(
            x, tests[i].level, tests[i].x0, tests[i].dx, U32Seq_Empty()
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
        U32Seq_Free(idx2);
        U32Seq_Free(idx);
    }

    free(state);

    return res;
}

bool testU32UniformPack() {
    bool res = true;

    struct {
        uint32_t unpacked[4];
        int32_t ulen;
        uint32_t packed[4];
        int32_t plen;
        uint8_t width;
    } tests[] = {
        {{0}, 0, {0}, 0, 32},
        {{0x12345678}, 1, {0}, 0, 0},
        {{0x12345678}, 1, {0x8}, 1, 4},
        {{0x12345678}, 1, {0x78}, 1, 8},
        {{0x12345678}, 1, {0x678}, 1, 12},
        {{0x12345678}, 1, {0x12345678}, 1, 32},
        {{0xaa, 0xbb}, 2, {0xbbaa}, 1, 8},
        {{0xaa, 0xbb}, 2, {0xbb00aa}, 1, 16},
        {{0xaa, 0xbb}, 2, {0xbb0aa}, 1, 12},
        {{3, 2, 3}, 3, {5}, 1, 1},
        {{0xaaaaaaaa, 0xbbbbbbbb}, 2, {0xaaaaaaaa, 0xbbbbbbbb}, 2, 32},
        {{0x87654321, 0x87654321}, 2, {0x17654321, 0x765432}, 2, 28},
        {{0xaaaaaa, 0xbbbbbb, 0xcccccc, 0xdddddd}, 4,
         {0xbbaaaaaa, 0xccccbbbb, 0xddddddcc}, 3, 24}
    };

    for (int i = 0; i < LEN(tests); i++) {
        U32Seq unpacked = U32Seq_FromArray(tests[i].unpacked, tests[i].ulen);
        U32Seq packed = U32Seq_FromArray(tests[i].packed, tests[i].plen);
        int32_t len = tests[i].ulen;
        uint8_t width = tests[i].width;

        U32Seq packedBuf = U32Seq_New(2);
        U32Seq unpackedBuf = U32Seq_New(2);

        U32Seq outPacked = util_U32UniformPack(unpacked, width, packedBuf);
        U32Seq outUnpacked = util_U32UndoUniformPack(
            packed, width, len, unpackedBuf
        );

        if(!U32SeqEqual(outPacked, packed)) {
            fprintf(stderr, "Expected util_U32UniformPack(");
            U32SeqPrint(unpacked);
            fprintf(stderr, ", %"PRIu8") to return ", width);
            U32SeqPrint(packed);
            fprintf(stderr, ", but got ");
            U32SeqPrint(outPacked);
            fprintf(stderr, "\n");
            res = false;
        }

        uint32_t widthFlag = 0xffffffff;
        if (width != 32) {
            widthFlag = ~( widthFlag << width);
        }
        for (int32_t j = 0; j < unpacked.Len; j++) {
            unpacked.Data[j] &= widthFlag;
        }

        if(!U32SeqEqual(outUnpacked, unpacked)) {
            fprintf(stderr, "Expected util_U32UndoUniformPack(");
            U32SeqPrint(packed);
            fprintf(stderr, ", %"PRIu8", %"PRIu32") to return ", width, len);
            U32SeqPrint(unpacked);
            fprintf(stderr, ", but got ");
            U32SeqPrint(outUnpacked);
            fprintf(stderr, "\n");
            res = false;
        }

        U32Seq_Free(packed);
        U32Seq_Free(outPacked);
        U32Seq_Free(unpacked);
        U32Seq_Free(outUnpacked);
    }

    if(!res) {
        return res;
    }

    rand_State *state = rand_Seed(0, 1);

    /* Large randomized tests. */
    for (uint8_t width = 0; width <= 32; width++) {
        for (int32_t len = 0; len < 2; len++) {
            U32Seq unpacked = U32Seq_New(len);
            for (int32_t j = 0; j < unpacked.Len; j++) {
                unpacked.Data[j] = (uint32_t) rand_Uint64(state);
            }
            U32Seq packed = util_U32UniformPack(unpacked, width, U32Seq_Empty());
            U32Seq outUnpacked = util_U32UndoUniformPack(
                packed, width, len, U32Seq_Empty()
            );

            uint32_t widthFlag = 0xffffffff;
            if (width != 32) {
                widthFlag = ~( widthFlag << width);
            }
            for (int32_t j = 0; j < unpacked.Len; j++) {
                unpacked.Data[j] &= widthFlag;
            }

            if (!U32SeqEqual(unpacked, outUnpacked)) {
                fprintf(stderr, "width = %"PRIu8", len = %"PRId32" randomized"
                        "test failed.\n", width, len);

                res = false;
            }

            U32Seq_Free(unpacked);
            U32Seq_Free(packed);
            U32Seq_Free(outUnpacked);
        }
    }

    free(state);

    return res;
}

bool testU32UndoPeriodic() {
    bool res = true;

    struct { uint32_t L, x[8], out[8]; int32_t len; } tests[] = {
        { 10, {0}, {0}, 0 },
        { 10, {1}, {1}, 1 },
        { 10, {9}, {9}, 1 },
        { 10, {1, 2}, {1, 2}, 2 },
        { 10, {8, 9}, {8, 9}, 2 },
        { 10, {1, 9}, {11, 9}, 2},
        { 10, {9, 1}, {9, 11}, 2},
        { 10, {0, 1, 2, 3, 4, 5, 6, 7}, {10, 11, 12, 13, 14, 5, 6, 7}, 8}
    };

    for (int i = 0; i < LEN(tests); i++) {
        U32Seq x = U32Seq_FromArray(tests[i].x, tests[i].len);
        U32Seq out = U32Seq_FromArray(tests[i].out, tests[i].len);
        util_U32UndoPeriodic(x, tests[i].L);
        
        if (!U32SeqEqual(x, out)) {
            fprintf(stderr, "In test %d of testU32UndoPeriodic, got ", i);
            U32SeqPrint(x);
            fprintf(stderr, ", but expected ");
            U32SeqPrint(out);
            fprintf(stderr, ".\n");
        }
    }

    return res;
}

bool testEntropyEncode() {
    char *source = "The Hitchhiker's Guide to the Galaxy has a few things to say on the subject of towels. A towel, it says, is about the most massively useful thing an interstellar hitch hiker can have.";
    U8Seq sourceSeq = U8Seq_FromArray(
        (uint8_t*) source, (int32_t)strlen(source)
    );
    U8Seq compressed = util_EntropyEncode(sourceSeq, U8Seq_Empty());
    U8Seq decompressed = util_UndoEntropyEncode(
        compressed, sourceSeq.Len, U8Seq_Empty()
    );

    if (!U8SeqEqual(sourceSeq, decompressed)) {
        fprintf(stderr, "entropy encoding garbled input string.\n");
        return false;
    }

    U8Seq_Free(sourceSeq);
    U8Seq_Free(compressed);
    U8Seq_Free(decompressed);

    return true;
}

bool testFastUniformCompress() {
    FSeq x = FSeq_New((int32_t) 1e6);
    FSeq y = FSeq_Empty();
    U32Seq buf = U32Seq_Empty();
    U32Seq buf2 = U32Seq_Empty();
    U32Seq out = U32Seq_Empty();

    uint8_t level = 15;
    
    float L = 10;
    float delta = L / (float)(1 << level);

    rand_State *state = rand_Seed(0, 1);

    FShuffle(x, state);
    for (int32_t j = 0; j < x.Len; j++)
        x.Data[j] += 1.5;
    util_Periodic(x, L);
    
    util_UndoPeriodic(x, L);
    float min, max;
    util_MinMax(x, &min, &max);
    buf = util_UniformBinIndex(x, level, min, max - min, buf);
    out = util_U32UniformPack(buf, level, out);

    buf2 = util_U32UndoUniformPack(out, level, x.Len, buf2);

    y = util_UndoUniformBinIndex(buf2, level, min, max - min, state, y);
    util_Periodic(y, L);

    free(state);

    util_Periodic(x, L);


    FSeq deltas = FSeq_New(x.Len);
    float deltaSum = 0;
    for (int32_t i = 0; i < x.Len; i++) {
        deltas.Data[i] = x.Data[i] - y.Data[i];

        deltaSum += MIN(fabs(deltas.Data[i]), L - fabs(deltas.Data[i]));
    }

    if (!FSeqAlmostEqual(x, y, delta, 2)) {
        fprintf(stderr, "in testFastUniformCompress, decompressed values are "
                "different from original values.\n");
    }

    return true;
}

bool testLittleEndian() {
    bool res = true;

    struct { uint32_t in[2], out[2]; } tests32[] = {
#if defined(DEBUG_MOCK_BIG_ENDIAN)
        {{0x00112233, 0x00abcdef}, {0x33221100, 0xefcdab00}}
#elif defined(DEBUG_MOCK_LITTLE_ENDIAN)
        {{0x00112233, 0x00abcdef}, {0x00112233, 0x00abcdef}}
#else
        {{0, 0}, {0, 0}}
#endif
    };

    for (int i = 0; i < LEN(tests32); i++) {
        U32Seq in = U32Seq_FromArray(tests32[i].in, 2);
        U32Seq out = U32Seq_FromArray(tests32[i].out, 2);
        for (int32_t j = 0; j < in.Len; j++) {
            out.Data[j] = util_U32LittleEndian(in.Data[j]);
        }

        if (!U32SeqEqual(in, out)) {
            fprintf(stderr, "For test %d of 32 bit testLittleEndian, got ", i);
            U32SeqPrint(in);
            fprintf(stderr, ", but expected ");
            U32SeqPrint(out);
            fprintf(stderr, ".\n");
            
            res = false;
        }

        U32Seq_Free(in);
        U32Seq_Free(out);
    }

    struct { uint64_t in[2], out[2]; } tests64[] = {
#if defined(DEBUG_MOCK_BIG_ENDIAN)
        {{0x0011223344556677, 0x00abcdef00abcdef},
         {0x7766554433221100, 0xefcdab00efcdab00}}
#elif defined(DEBUG_MOCK_LITTLE_ENDIAN)
        {{0x0011223344556677, 0x00abcdef00abcdef},
         {0x0011223344556677, 0x00abcdef00abcdef}}
#else
        {{0, 0}, {0, 0}}
#endif
    };

    for (int i = 0; i < LEN(tests64); i++) {
        U64Seq in = U64Seq_FromArray(tests64[i].in, 2);
        U64Seq out = U64Seq_FromArray(tests64[i].out, 2);
        for (int32_t j = 0; j < in.Len; j++) {
            out.Data[j] = util_U64LittleEndian(in.Data[j]);
        }

        if (!U64SeqEqual(in, out)) {
            fprintf(stderr, "For test %d of 64 bit testLittleEndian, got ", i);
            U64SeqPrint(in);
            fprintf(stderr, ", but expected ");
            U64SeqPrint(out);
            fprintf(stderr, ".\n");
            
            res = false;
        }

        U64Seq_Free(in);
        U64Seq_Free(out);
    }

    return res;
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

bool U64SeqEqual(U64Seq s1, U64Seq s2) {
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

bool FSeqAlmostEqual(FSeq s1, FSeq s2, float eps, float L) {
    if (s1.Len != s2.Len) {
        return false;
    }

    if (L == 0) {
        for (int32_t i = 0; i < s1.Len; i++) {
            if (s1.Data[i] + eps < s2.Data[i] ||
                s1.Data[i] - eps > s2.Data[i]) {
                return false;
            }
        }
    } else {
        for (int32_t i = 0; i < s1.Len; i++) {
            if ((s1.Data[i] + eps < s2.Data[i] ||
                 s1.Data[i] - eps > s2.Data[i]) &&
                (s1.Data[i] + L + eps < s2.Data[i] ||
                 s1.Data[i] + L - eps > s2.Data[i]) &&
                (s1.Data[i] - L + eps < s2.Data[i] ||
                 s1.Data[i] - L - eps > s2.Data[i])) {
                return false;
            }
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

void U64SeqPrint(U64Seq s) {
    if (s.Len == 0) {
        fprintf(stderr, "[]");
        return;
    }

    fprintf(stderr, "[");
    fprintf(stderr, "%"PRIx64, s.Data[0]);
    for (int32_t i = 1; i < s.Len; i++) {
        fprintf(stderr, ", %"PRIx64, s.Data[i]);
    }

    fprintf(stderr, "]");
}

void FSeqPrint(FSeq s) {
    if (s.Len == 0) {
        fprintf(stderr, "[]");
        return;
    }

    fprintf(stderr, "[");
    fprintf(stderr, "%.5g", s.Data[0]);
    for (int32_t i = 1; i < s.Len; i++) {
        fprintf(stderr, ", %.5g", s.Data[i]);
    }

    fprintf(stderr, "]");
}

void FShuffle(FSeq x, rand_State *s) {
    for (int32_t i = 0; i < x.Len; i++) {
        x.Data[i] = rand_Float(s);
    }
}

