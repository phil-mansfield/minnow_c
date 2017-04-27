#ifndef _MNW_STREAM_H
#define _MIN_STREAM_H

#include "seq.h"

typedef struct stream_Reader {
    U8BigSeq Bytes;
    size_t offset;
} stream_Reader;

typedef U8BigSeq stream_Writer;

stream_Writer stream_NewWriter();
stream_Reader stream_NewReader(U8BigSeq bytes);

void stream_Read(
    stream_Reader reader, void *ptr, size_t bytes, size_t elemSize
);

void stream_Write(
    stream_Writer writer, void *ptr, size_t bytes, size_t elemSize
);

#endif
