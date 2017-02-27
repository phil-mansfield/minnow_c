#include <stdint.h>
#include <stdio.h>

#include "tmp.h"

int main() {
    float in[4] = {1.0, 1.5, 2.5, 100.5};
    //int64_t out[4] = {0, 0, 0, 0};

    mnw_QuantizeFloat(in, 0, 4);
}
