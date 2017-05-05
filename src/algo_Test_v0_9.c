#include "algo_Test_v0_9.h"

CField TestCompress_v0_9(QField cf, void *buffer) {
    (void) cf;
    (void) buffer;

    CField dummy;
    return dummy;
}

QField TestDecompress_v0_9(CField cf, void *buffer) {
    (void) cf;
    (void) buffer;
    
    QField dummy;
    return dummy;
}

void *TestCAlloc_v0_9(void) {
    return NULL;
}

void TestCFree_v0_9(void *buffer) {
    (void) buffer;
}

void *TestDAlloc_v0_9(void) {
    return NULL;
}

void TestDFree_v0_9(void *buffer) {
    (void) buffer;
}
