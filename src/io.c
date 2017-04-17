#include "io.h"

int io_Write(FILE *file, U8BigSeq bytes) {
    (void) file;
    (void) bytes;
    return 0;
}

int64_t io_Size(FILE *file) {
    (void) file;
    return -1;
}

int io_Read(FILE *file, U8BigSeq bytes) {
    (void) file;
    (void) bytes;
    return 0;
}

algo_Particles io_ReadGadget2(FILE *file, algo_Particles buf) {
    (void) file;
    return buf;
}
