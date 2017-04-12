#ifndef ALGO_H_
#define ALGO_H_

/* Author: Phil Mansfield (mansfield@uchicago.edu)
 *
 * algo.h contains common strucutures used by all of Minnow's algorithms. The
 * general structure is this:
 *
 * Compression:
 * User fills algo_Particles
 * algo_Quantize(algo_Particles) -> algo_QuantizedParticles
 * algo*_Compress(algo_QuantizedParticles) -> algo_CompressedParticles
 * The user can then store algo_CompressedParticles to disk.
 *
 * Decompression:
 * User reads algo_CompressedParticles from disk
 * algo*_UndoCompress(algo_CompressedParticles) -> algo_QuantizedParticles
 * algo_UndoQuantize(algo_QuantizedParticles) -> algo_Particles
 * The user can then do whatever they want with algo_Particles.
 *
 * A user external to Minnow (as opposed to a user which is another Minnow file)
 * should probably not be using any of these directly, but if they are, they
 * should only use algo_Particles. */

#include <stdbool.h>
#include <stdint.h>
#include "seq.h"

/*******************/
/* Data Structures */
/*******************/

/* algo_VarAccuracy specifies how accurately a float variable needs to
 * be stored. */
typedef struct algo_Accuracy {
    /* Delta and Deltas represent the minimum accuracy that values need to
     * be set to. A positive value for Delta indicates uniform accuracy and
     * non-empty Deltas indicates non-uniform accuracy. */
    float Delta;
    FSeq Deltas;

    /* Log indicates that Delta is a logrithmic (in base 10) accuracy. */
    bool Log;
} algo_Accuracy;

/* algo_Particles represents the particles in some (ideally spatially coherent)
 * sub-volume of a simulation. V, ID, FVars, and U64Vars are optional and may
 * be empty. */
typedef struct algo_Particles {
    FSeq X[3], V[3];
    algo_Accuracy XAcc, VAcc;
    /* XWidth is the width of the simulation box. */
    float XWidth;

    U64Seq ID;
    /* IDWidth is the width of the simulation box in ID space. If IDs do not
     * correspond to grid coordinates, they will need to be stored as a generic
     * U64Vars. */
    uint32_t IDWidth;

    /* FVars and U64Vars are miscellaneous variables associated with each
     * particle. FVars.Data[j].Data[i] corresponds to the jth variable of the
     * ith particle. */
    FSeqSeq FVars;
    algo_Accuracy *FVarsAcc;
    U64SeqSeq U64Vars;
} algo_Particles;

static const algo_Particles algo_EmptyParticles;

/* algo_QuantizedRange specifies the range of values that a quantized value
 * can take on. These ranges must always be powers of two and are represented
 * by "depths," which are the base two exponents. */
typedef struct algo_QuantizedRange {
    uint8_t Depth;
    /* A non-empty Depths indicates the non-uniform accuracies have been
     * specified and that the value of Depth should be ignored. */
    U8Seq Depths;
    float X0, X1;
    bool Log;
} algo_QuantizedRange;

static const algo_QuantizedRange algo_EmptyQuantizedRange;

/* algo_QuantizedVectorRange specifies the range of values that a quantized
 * vector can take on. These ranges must always be powers of two and are
 * represented by "depths," which are the base two exponents. */
typedef struct algo_QuantizedVectorRange {
    uint8_t Depth;
    /* A non-empty Depths indicates the non-uniform accuracies have been
     * specified and that the value of Depth should be ignored. */
    U8Seq Depths;
    float X0[3], X1[3];
} algo_QuantizedVectorRange;

static const algo_QuantizedVectorRange algo_EmptyQuantizedVectorRange;

/* algo_QuantizedIDRange specifies the range of values that particle IDs can
 * take on. The IDs are "vectorized," which means that they've been converted
 * to an (ID_x, ID_y, ID_z) format.*/
typedef struct algo_QuantizedIDRange {
    uint32_t X0[3], X1[3];
} algo_QuantizedIDRange;

static const algo_QuantizedVectorRange algo_EmptyQuantizedIDRange;

/* algo_QuantizedParticles represents the same information as algo_Particles,
 * but quantized to a grid. It also contains information on the quantization
 * depths and ranges. */
typedef struct algo_QuantizedParticles {
    U32Seq X[3], V[3];
    algo_QuantizedVectorRange XRange, VRange;
    float XWidth;

    /* Here, ID is represented as a 3-vector, which is generally what IDs
     * correspond to in cosmological simulations. If this isn't true, the IDs
     * will have been stored as one of the generic U64Vars elements. */
    U32Seq ID[3];
    algo_QuantizedIDRange IDRange;
    uint32_t IDWidth;

    U32SeqSeq FVars;
    algo_QuantizedRange *FVarsRange;
    U64SeqSeq U64Vars;

} algo_QuantizedParticles;

static const algo_QuantizedParticles algo_EmptyQuantizedParticles;

/* algo_CompressedParticles contains the same information as algo_Particles,
 * but in a compressed format. All particle data not contained within these
 * fields is stored within the Blocks elements. */
typedef struct algo_CompressedParticles {
    U8SeqSeq Blocks;
    int32_t ParticleNum, FVarsLen, U64VarsLen;
    bool HasV, HasID;
} algo_CompressedParticles;

static const algo_CompressedParticles algo_EmptyCompressedParticles;

/****************/
/* Quantization */
/****************/

/* algo_Quantize quantizes an block of particles. An optional buffer can be
 * passed to this function, although it cannot be assumed to point to valid
 * memory after this function returns. buf must be zeroed or must be previous
 * return value of algo_Quantize. */
algo_QuantizedParticles algo_Quantize(
    algo_Particles p, algo_QuantizedParticles buf
);

/* algo_Quantize reverses quantization of a block of particles. An optional
 * buffer can be passed to this function, although it cannot be assumed to
 * point to valid memory after this function returns. buf must be zeroed or
 * must be previous return value of algo_UndoQuantize. */
algo_Particles algo_UndoQuantize(
    algo_QuantizedParticles p, algo_Particles buf
);

/********************/
/* Struct Functions */
/********************/

/* Particles_Free frees all heap-allocated memory associated with p. This
 * includes arrays referenced to by seqeunces within p. */
void Particles_Free(algo_Particles p);

/* QuantizedParticles_Free frees all heap-allocated memory associated with p.
 * This includes arrays referenced to by seqeunces within p. */
void QuantizedParticles_Free(algo_QuantizedParticles p);

/* CompressedParticles_Free frees all heap-allocated memory associated with p.
 * This includes arrays referenced to by seqeunces within p. */
void CompressedParticles_Free(algo_CompressedParticles p);

/* The format used by CompressedParticles_ToBytes and
 * CompressedParticles_FromBytes is the following:
 *
 * | hd: SegmentHeader |
 * | Len_0: int32_t | ... | Len_n: int32_t |
 * | Data_0: []uint8_t | ... | Len_n: int32_t |
 *
 * - hd: SegmentHeader - A header of type
 *   struct SegmentHeader {
 *       BlockNum, ParticleNum int32_t;
 *       HasV, HasID, FVarsLen, U64VarsLen int32_t;
 *   };
 * - Len_i: int32_t - The length of the ith block.
 * - Data_i: []uint8_t - The data of the ith block.
 */

/* CompressedParticles_ToBytes converts an algo_CompressedParticles struct to
 * a raw byte sequence. */
U8BigSeq CompressedParticles_ToBytes(
    algo_CompressedParticles p, U8BigSeq buf
);

/* CompressedParticles_FromBytes converts a raw byte sequence into an
 * algo_CompressedParticles struct. */
algo_CompressedParticles CompressedParticles_FromBytes(
    U8BigSeq bytes, algo_CompressedParticles buf
);

/*********************/
/* Utility Functions */
/*********************/

/* QuantizedParticles_Check checks that every field in an
 * algo_QuantizedParticles struct is valid. It Panics otherwise. */
void QuantizedParticles_Check(algo_QuantizedParticles p);

/* Particles_check checks that every field in an algo_Particles struct is
 * valid. It Panics otherwise. */
void Particles_Check(algo_Particles p);

#endif /* ALGO_H_ */
