#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>
#include <string.h>
#include <stdint.h>

#define FIELD_POSN 0x506f736e 
#define FIELD_VELC 0x56656c63
#define FIELD_PTID 0x50746964
#define FIELD_UNSF 0x556e7366
#define FIELD_UNSI 0x556e7369

#define ALGO_TRIM 0x5472696d
#define ALGO_DIFF 0x44696666
#define ALGO_COIL 0x436f696c
#define ALGO_OCTO 0x4f63746f
#define ALGO_SORT 0x536f7274
#define ALGO_CART 0x43617274

/* YOLO strats: redo everything. */

/* The Accuracy type is how the user specifies how accurately Shellfish needs
 * to store a field. The structure that this pointer points to depends on 
 * the type of the field. */

typedef void *Accuracy;

typedef struct FloatAccuracy {
    uint32_t FieldCode; /* Must be first. = FIELD_UNSF */
    float *Deltas; /* NULL, if Len = 0. */
    float Delta;
    float SymLogThreshold; /* Only meaningful is LogScaled = 2 */
    int32_t Len;
    int32_t LogScaled; /* 0 = not log scaled
                          1 = log10 scaled
                          2 = symlog10 scaled */
} FloatAccuracy;

typedef struct IntAccuracy {
    uint32_t FieldCode;
} IntAccuracy;

typedef struct PositionAccuracy {
    uint32_t FieldCode; /* Must be first. */
    float *Deltas; /* NULL, if Len = 0. */
    float Delta, BoxWidth;
    int32_t Len;
} PositionAccuracy;

typedef struct VelocityAccuracy {
    uint32_t FieldCode; /* Must be first. */
    float *Deltas; /* NULL, if Len = 0. */
    float Delta;
    int32_t Len;
    int32_t SymLogScaled; /* Velocities can only be symlog10 scaled
                             since they're signed. */
    float SymLogThreshold; /* Only meaningful if SymLogScaled != 0 */
} VelocityAccuracy;

typedef struct IDAccuracy {
    uint32_t FieldCode; /* Must be first. */
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
    uint64_t NaNFlag;
    float Offset;
} FloatQuantization;

typedef struct IntQuantization {
    IntAccuracy Acc;
    uint64_t Offset;
} IntQuantization;

typedef struct PositionQuantization {
    PositionAccuracy Acc;
    uint64_t NaNFlag;
    float Offset[3];
} PositionQuantization;

typedef struct VelocityQuantization {
    VelocityAccuracy Acc;
    uint64_t NaNFlag;
    float Offset[3];
} VelocityQuantization;

typedef struct IDQuantization {
    IDAccuracy Acc;
    uint64_t NaNFlag;
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
    uint8_t *Data; /* Quantization is also stored here. */
} CField;

/* Compressors and Decompressors */

/* Second argument is an algorithm-dependent buffer. */
typedef QField (*DFunc)(CField, void*);
/* CFunc will not compute the checksum. */
typedef CField (*CFunc)(QField, void*);

typedef struct Decompressor {
    void *Buffer;
    DFunc DFunc;
    uint64_t NaNFlag; /* If NaNFlag == 0, it's assumed that the decompressor
                       * cannot paritally recover and all values will just be
                       * set to NaN on checksum failure. */
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
    int32_t FieldLen;
} QSeg;

typedef struct CSeg {
    CField *Fields;
    int32_t FieldLen;
} CSeg;

#endif
