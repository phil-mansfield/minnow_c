#ifndef _MIN_IO_H
#define _MIN_IO_H

#include <stdio.h>
#include <stdint.h>
#include "algo.h"

/* File format:
 *   +----------------------+
 *   | 0xbadf00d : uint32_t |  XXXX
 *   +----------------------+
 *   | ByteNum : int64_t    |  XXXXXXXX
 *   +----------------------+
 *   | Data : []uint8_t     |  X ... X
 *   +----------------------+
 */

int io_Write(FILE *file, U8BigSeq bytes);
int64_t io_Size(FILE *file);
int io_Read(FILE *file, U8BigSeq bytes);

algo_Particles io_ReadGadget2(FILE *file, algo_Particles buf);

#endif /* _MIN_IO_H */
