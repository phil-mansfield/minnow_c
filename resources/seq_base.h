#ifndef MNW_SEQ_BASE_H_
#define MNW_SEQ_BASE_H_

/* Author: Phil Mansfield (mansfield@uchicago.edu)
 . 
 . seq.h defines the interfaces for a host of sequence types (abbreviated
 . "seq"). A sequence is just another name for an array list/slice/dynamic
 . array/etc., and I'm only using it because these types will be ubiquitous
 . thorughout the project and I'm trying to keep keystrokes down.
 .
 . Most of the types in this file are autogenerated by a a macro. To help the
 . reader, there is a dummy example type, `ExSeq` which is concretely
 . implemented. This autogeneration is the reason multiline comments in this
 . file look wonky.
 .
 . If you are unfamiliar with how array lists work, I wrote a blog post on it
 . a while ago which you can read here:
 . https://floatingpointastronomy.wordpress.com/2016/10/20/high-performance-lists-iii-array-lists/
 . */

#include <stdint.h>
#include <stdlib.h>

/* Example is a placeholder type which is used as a concrete example of the
 . types generated by the GENERATE_SEQ_INTERFACE macro in this header file. */
typedef double Example;

/* ExSeq is a sequence (a dynamic array) which contains elements of type
 . Example. */
typedef struct ExSeq {
    Example *Data;
    int32_t Len, Cap;
} ExSeq;

ExSeq ExSeq_Empty();

/* `ExSeq` creates a new `Example` sequence with length `len`. */
ExSeq ExSeq_New(int32_t len);

/* `ExSeq_FromArray` creates an `Example` sequeence from an existing array. */
ExSeq ExSeq_FromArray(Example *data, int32_t len);

/* ExSeq_WrapArray returns an `Example` sequence that points to an existing
 . array without performing additional allocations. Appending and Joining can
 . only be done on the new sequence if the input array directly corresponds to
 . a block requested by malloc and isn't a sub array of any type. */
ExSeq ExSeq_WrapArray(Example *data, int32_t len);

/* `ExSeq_NewWithCap` creates a new `Example` seqeunce with length `len` and
 . a capsize of `cap`. */
ExSeq ExSeq_NewWithCap(int32_t len, int32_t cap);

/* `ExSeq_Free` frees an `Example` seqeunce regardless of how many living
 . references it has. */
void ExSeq_Free(ExSeq s);

/* `ExSeq_Append` appends an element of type `Example` to the tail of a
 . sequence. All pointers attached to data in the sequence and all slices of
 . sequence cannot be assumed to be valid after an append is performed. */
ExSeq ExSeq_Append(ExSeq s, Example tail);

/* `ExSeq_Join` joins two `Example` sequences together. All pointers attached
 . to data in s1 and all subslices into s1 cannot be  assumed to be valid after
 . a join is performed. */
ExSeq ExSeq_Join(ExSeq s1, ExSeq s2);

/* `ExSeq_Sub` returns a subsquence of an `Example` seqeunce. This subseqeunce
 . is only valid as long as no joins or appends are performed on any
 . subsequences or on the original sequence. */
ExSeq ExSeq_Sub(ExSeq s, int32_t start, int32_t end);

/* `ExSeq_Extend` increases the cap size of an `Example` sequence to at least
 .  `n`. */
ExSeq ExSeq_Extend(ExSeq s, int32_t n);

typedef struct ExBigSeq {
    Example *Data;
    int64_t Len, Cap;
} ExBigSeq;

ExBigSeq ExBigSeq_Empty();

/* `ExBigSeq` creates a new `Example` sequence with length `len`. */
ExBigSeq ExBigSeq_New(int64_t len);

/* `ExBigSeq_FromArray` creates an example array from an existing array. */
ExBigSeq ExBigSeq_FromArray(Example *data, int64_t len);

/* ExBigSeq_WrapArray returns an `Example` sequence that points to an existing
 . array without performing additional allocations. Appending and Joining can
 . only be done on the new sequence if the input array directly corresponds to
 . a block requested by malloc and isn't a sub array of any type. */
ExBigSeq ExBigSeq_WrapArray(Example *data, int64_t len);

/* `ExBigSeq_NewWithCap` creates a new `Example` seqeunce with length `len` and
 . a capsize of `cap`. */
ExBigSeq ExBigSeq_NewWithCap(int64_t len, int64_t cap);

/* `ExBigSeq_Free` frees an `Example` seqeunce regardless of how many living
 . references it has. */
void ExBigSeq_Free(ExBigSeq s);

/* `ExBigSeq_Append` appends an element of type `Example` to the tail of a
 . sequence. All pointers attached to data in the sequence and all slices of
 . sequence cannot be assumed to be valid after an append is performed. */
ExBigSeq ExBigSeq_Append(ExBigSeq s, Example tail);

/* `ExBigSeq_Join` joins two `Example` sequences together. All pointers attached
 . to data in s1 and all subslices into s1 cannot be  assumed to be valid after
 . a join is performed. */
ExBigSeq ExBigSeq_Join(ExBigSeq s1, ExBigSeq s2);

/* `ExBigSeq_Sub` returns a subsquence of an `Example` seqeunce. This subseqeunce
 . is only valid as long as no joins or appends are performed on any
 . subsequences or on the original sequence. */
ExBigSeq ExBigSeq_Sub(ExBigSeq s, int64_t start, int64_t end);

/* `ExBigSeq_Extend` increases the cap size of an `Example` sequence to at least
 .  `n`. */
ExBigSeq ExBigSeq_Extend(ExBigSeq s, int64_t n);

/* Autogenerated code below this point (including this comment). */

#endif /* MNW_SEQ_BASE_H_ */
    
