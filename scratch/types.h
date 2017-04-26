#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>
#include <string.h>
#include <stdint.h>

/* YOLO strats: redo everything. */

/* The Accuracy type is how the user specifies how accurately Shellfish needs
 * to store a field. The structure that this pointer points to depends on 
 * the type of the field. */

typedef void *Accuracy;

typedef struct FloatAccuracy {
    float *Deltas; /* NULL, if Len = 0. */
    float Delta;
    float SymLogThreshold; /* Only meaningful is LogScaled = 2 */
    int32_t Len;
    int32_t LogScaled; /* 0 = not log scaled
                          1 = log10 scaled
                          2 = symlog10 scaled */
} FloatAccuracy;

typedef struct PositionAccuracy {
    float *Deltas; /* NULL, if Len = 0. */
    float Delta, BoxWidth;
    int32_t Len;
} PositionAccuracy;

typedef struct VelocityAccuracy {
    float *Deltas; /* NULL, if Len = 0. */
    float Delta;
    int32_t Len;
    int32_t SymLogScaled; /* Velocities can only be symlog10 scaled
                             since they're signed. */
    float SymLogThreshold; /* Only meaningful if SymLogScaled != 0 */
} VelocityAccuracy;

typedef struct IDAccuracy {
    uint64_t Width;
} IDAccuracy;

/* The Quantization type is how minnow remembers how accurately it stored
 * a field. More specifically, it contains enough information to unambiguously
 * decode the quantization process. As with Accuracy, the exact structure that
 * this pointer points to depends on the field type. */

typedef void *Quantization;

/* struct field meanings are the same as in FloatAccuracy. However, minnow
 * has the freedom to change Acc.Delta and Acc.Deltas to smaller values (e.g.
 * to help with alignment). */
typedef struct FloatQuantization {
    FloatAccuracy Acc;
    float Offset;
} FloatQuantization;

typedef struct PositionQuantization {
    PositionAccuracy Acc;
    float Offset[3];
} PositionQuantization;

typedef struct VelocityQuantization {
    VelocityAccuracy Acc;
    float Offset[3];
} VelocityQuantization;

typedef struct IDQuantization {
    IDAccuracy Acc;
    uint64_t Offset[3];
} IDQuantization;

/* Fields */

typedef struct FieldHeader {
    uint32_t FieldCode;
    uint32_t AlgoCode;
    uint32_t AlgoVersion;
    int32_t ParticleLen;
} FieldHeader;

typedef struct Field {
    FieldHeader Hd;
    void *Data;
    Accuracy Acc;
} Field;

typedef struct QField {
    FieldHeader Hd;
    uint64_t *Data;
    Quantization Quant;
} QField;

typedef struct CField {
    FieldHeader Hd;
    uint32_t Checksum;
    int32_t DataLen;
    uint8_t *Data;
} CField;

/* Compressors and Decompressors */

/* Second argument is an algorithm-dependent buffer. */
typedef QField (*DFunc)(CField, void*);
typedef CField (*CFunc)(QField, void*);

typedef struct Decompressor {
    void *Buffer;
    DFunc DFunc;
    uint64_t NaNFlag;
} Decompressor;

typedef struct Compressor {
    void *Buffer;
    CFunc CFunc;
} Compressor;

/* Segments */

typedef struct Seg {
    Field *Fields;
    int32_t FieldLen;
} Seg;

typedef struct QSeg {
    QField *Fields;
    Compressor *Compressors;
    int32_t FieldLen;
} QSeg;

typedef struct CSeg {
    CField *Fields;
    Decompressor *Decompressors;
    int32_t FieldLen;
} CSeg;

#endif
