#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>
#include <string.h>
#include <stdint.h>

#define field_Posn 0x506f736e 
#define field_Velc 0x56656c63
#define field_Ptid 0x50746964
#define field_Unsf 0x556e7366
#define field_Unsi 0x556e7369

#define algo_Test 0x5472696d
#define algo_Trim 0x5472696d
#define algo_Diff 0x44696666
#define algo_Coil 0x436f696c
#define algo_Octo 0x4f63746f
#define algo_Sort 0x536f7274
#define alog_Cart 0x43617274

/* YOLO strats: redo everything. */

/* The Accuracy type is how the user specifies how accurately Shellfish needs
 * to store a field. The structure that this pointer points to depends on 
 * the type of the field. */

typedef void *Accuracy;

typedef struct FloatAccuracy {
    float *Deltas; /* NULL, if Len = 0. */
    float Delta;
    float SymLog10Threshold; /* Only meaningful is Log10Scaled = 2 */
    int32_t Len;
    int32_t Log10Scaled; /* 0 = not log scaled
                          1 = log10 scaled
                          2 = symlog10 scaled */
} FloatAccuracy;

typedef uint8_t IntAccuracy; /* Does nothing. */

typedef struct PositionAccuracy {
    float *Deltas; /* NULL, if Len = 0. */
    float Delta, BoxWidth;
    int32_t Len;
} PositionAccuracy;

typedef struct VelocityAccuracy {
    float *Deltas; /* NULL, if Len = 0. */
    float Delta;
    int32_t Len;
    int32_t SymLog10Scaled; /* Velocities can only be symlog10 scaled
                             since they're signed. */
    float SymLog10Threshold; /* Only meaningful if SymLogScaled != 0 */
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
    uint8_t *Depths;
    uint64_t NaNFlag;
    int32_t Len, Log10Scaled;
    float SymLog10Threshold, X0, X1;
    uint8_t Depth;
} FloatQuantization;

typedef struct IntQuantization {
    uint64_t NaNFlag, X0, X1;
} IntQuantization;

typedef struct PositionQuantization {
    uint8_t *Depths;
    uint64_t NaNFlag;
    int32_t Len;
    float X0[3], X1[3];
    uint8_t Depth;
} PositionQuantization;

typedef struct VelocityQuantization {
    uint8_t *Depths;
    uint64_t NaNFlag;
    int32_t Len, SymLog10Scaled;
    float X0[3], X1[3];
    float SymLog10Threshold;
    uint8_t Depth;
} VelocityQuantization;

typedef struct IDQuantization {
    uint64_t NaNFlag, Width;
    uint64_t X0[3], X1[3];
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
    uint8_t *Data; /* Quantization is also stored here. */
    int64_t DataLen;
    uint32_t Checksum;
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
