#include "debug.h"
#include "stream.h"
#include "util.h"
#include "string.h"

stream_Writer stream_NewWriter() {
    return U8BigSeq_New(0);
}

stream_Reader stream_NewReader(U8BigSeq bytes) {
    stream_Reader reader = {
        .Bytes = bytes,
        .offset = 0,
    };

    return reader;
}

void stream_Read(
    stream_Reader reader, void *ptr, size_t bytes, size_t elemSize
) {
    DebugAssert(bytes % elemSize) {
        Panic("elemSize %zu does not evenly divide byte number %zu.",
              elemSize, bytes);
    }

    memcpy(ptr, reader.Bytes.Data + reader.offset, bytes);
    reader.offset += bytes;

    size_t dataLen = bytes / elemSize;

    switch (bytes) {
    case 1:
        break;
    case 4:
        ;
        uint32_t *data32 = (uint32_t*)ptr;
        for (int64_t i = 0; i < (int64_t)dataLen; i++) {
            util_U32LittleEndian(data32[i]);
        }
        break;
    case 8:
        ;
        uint64_t *data64 = (uint64_t*)ptr;
        for (int64_t i = 0; i < (int64_t)dataLen; i++) {
            util_U64LittleEndian(data64[i]);
        }
        break;
    default:
        Panic("Unsupported elemSize, %zu.", elemSize);
    }
}

void stream_Write(
    stream_Writer writer, void *ptr, size_t bytes, size_t elemSize
) {
    DebugAssert(bytes % elemSize) {
        Panic("elemSize %zu does not evenly divide byte number %zu.",
              elemSize, bytes);
    }

    size_t dataLen = bytes / elemSize;

    /* We can't do this within the stream itself because the current point
     * in the stream might not be aligned correctly. */
    switch (bytes) {
    case 1:
        break;
    case 4:
        ;
        uint32_t *data32 = (uint32_t*)ptr;
        for (int64_t i = 0; i < (int64_t)dataLen; i++) {
            util_U32LittleEndian(data32[i]);
        }
        break;
    case 8:
        ;
        uint64_t *data64 = (uint64_t*)ptr;
        for (int64_t i = 0; i < (int64_t)dataLen; i++) {
            util_U64LittleEndian(data64[i]);
        }
        break;
    default:
        Panic("Unsupported elemSize, %zu.", elemSize);
    }

    writer = U8BigSeq_Join(writer, U8BigSeq_WrapArray(ptr, (int64_t)bytes));

    switch (bytes) {
    case 1:
        break;
    case 4:
        ;
        uint32_t *data32 = (uint32_t*)ptr;
        for (int64_t i = 0; i < (int64_t)dataLen; i++) {
            util_U32UndoLittleEndian(data32[i]);
        }
        break;
    case 8:
        ;
        uint64_t *data64 = (uint64_t*)ptr;
        for (int64_t i = 0; i < (int64_t)dataLen; i++) {
            util_U64UndoLittleEndian(data64[i]);
        }
        break;
    default:
        Panic("Unsupported elemSize, %zu.", elemSize);
    }
    return;
}
