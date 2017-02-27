#include <stdio.h>

#include "tmp.h"
#include "debug.h"

void mnw_QuantizeFloat(float *in, int64_t *out, int64_t len) {
    Assert(in && out) {
        Panic("mnw_QuantizeFloat passed in = %p, out = %p\n", in, out);
    }

    for (int64_t i = 0; i < len; i++) {
        out[i] = (int64_t) in[i];
    }
}
