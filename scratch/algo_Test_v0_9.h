#ifndef MNW_TEST_V0_9_H_
#define MNW_TEST_V0_9_H_

#include "types.h"

CField TestCompress_v0_9(QField cf, void *buffer);
QField TestDecompress_v0_9(CField cf, void *buffer);
void *TestCAlloc_v0_9(void);
void TestCFree_v0_9(void *buffer);
void *TestDAlloc_v0_9(void);
void TestDFree_v0_9(void *buffer);

#endif
