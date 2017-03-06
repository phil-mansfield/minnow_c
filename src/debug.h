#ifndef MNW_DEBUG_
#define MNW_DEBUG_

/* Author: Phil Mansfield (mansfield@uchicago.edu)
 *
 * `debug.h` contains many useful debugging macros.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(__GNUC__) || defined(__linux__)
#include <execinfo.h>
#endif /* __GNUC__ || __linux__ */

/* `Assert` is an assertion which allows you to print messages an clean up
 * prior to failure. An example usage would be the following:
 * 
 * Assert(myPointer) {
 *     fprintf(stderr, "myPointer is NULL.");
 * }
 *
 * If you want a stack trace (which you should), prefer to use Panic instead
 * of fprintf for reporting your error.
 */
#define Assert(x) for (; !(x); assert(x))
        

/* `AssertAlloc` is an assertion which specifically checks whether an allocation
 * error has occured. Unlike Panic, it does not assume that allocators still
 * work. All alloctation in the code should look pretty much exactly like this:
 * 
 * int64_t *ptr = calloc(num, size);
 * AssertAlloc(ptr);
 */
#define AssertAlloc(ptr) \
    do { \
        Assert(ptr) {                                            \
            fprintf(stderr, "%s: %s: L%d: Allocation failed\n.", \
                    __FILE__, __FUNCTION__, __LINE__);         \
            exit(1); \
        } \
    } while(0)

/* `UcheckedDebugAssert` and `DebugPrintf` are `Assert` and `printf` statements
 * (respectively), which only run if the `DEBUG` flag is set at compile time.
 * They are otherwise identical. `DebugAssert` acts like an `Assert` during
 * debug mode and like a standard assert otherwise. This should be used if
 * you're worried about the code contained within the assert block being large
 * enough to prevent a function from being inlined.
 */
#ifdef DEBUG
#define DebugAssert(x) Assert(x)
#if defined(__clang__)
#define DebugPrintf(fmt, ...)
#else
#define DebugPrintf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif
#define UncheckedDebugAssert(x) Assert(x)
#else /* !DEBUG */
#define DebugAssert(x) assert(x); if(0)
#define DebugPrintf(fmt, ...)
#define UncheckedDebugAssert(x)
#endif /* DEBUG */

/* Panic crashes the program and prints out descriptive information. On some
 * compilers/systems (read: GCC), this might include a stack trace. Panic
 * may require dynamic memory allocation, but also kills the program, so this
 * is only a problem if your allocator is currently compromised.
 */
#if defined(__clang__)
#define Panic(fmt, ...) \
    do { \
        fprintf(stderr, "Panic at file %s, function %s, line %d:\n", \
                __FILE__, __FUNCTION__, __LINE__);               \
        fprintf(stderr, fmt, __VA_ARGS__); \
        fprintf(stderr, "\n");                                   \
        fprintf(stderr, "Stack trace:\n"); \
        void **panicStackframes_ = calloc((size_t)100, sizeof(*panicStackframes_)); \
        size_t panicSize_ = backtrace(panicStackframes_, 100); \
        char **panicStrings_ = backtrace_symbols( \
            panicStackframes_, (int)panicSize_                          \
        );                                                              \
        for (size_t panicIdx_ = 0; panicIdx_ < panicSize_; panicIdx_++) \
            fprintf(stderr, "%s\n", panicStrings_[panicIdx_]);           \
        free(panicStrings_); \
        exit(1); \
    } while (0)
#else
//#if defined(__GNUC__) || defined(__linux__)
#define Panic(fmt, ...) \
    do { \
        /* This is a bit excessive, but I'd prefer to avoid linking. */ \
        fprintf(stderr, "Panic at file %s, function %s, line %d:\n", \
                __FILE__, __FUNCTION__, __LINE__);               \
        fprintf(stderr, fmt, ##__VA_ARGS__); \
        fprintf(stderr, "\n"); \
        fprintf(stderr, "Stack trace:\n"); \
        void **panicStackframes_ = calloc((size_t)100, sizeof(*panicStackframes_)); \
        size_t panicSize_ = backtrace(panicStackframes_, 100); \
        char **panicStrings_ = backtrace_symbols( \
            panicStackframes_, (int)panicSize_                          \
        );                                                              \
        for (size_t panicIdx_ = 0; panicIdx_ < panicSize_; panicIdx_++) \
            fprintf(stderr, "%s\n", panicStrings_[panicIdx_]);           \
        free(panicStrings_); \
        exit(1); \
    } while (0)
#endif
//#else
//#define Panicf(fmt, ...)                      \
//    do {                                                              \
//        fprintf(stderr, "%s:%s:L%d:\n", __FILE__, __FUNCTION__, __LINE__); \
//        fprintf(stderr, fmt, ##__VA_ARGS__);                                  \
//        fprintf(stderr, "\n");                                        \
//        exit(1);                                                      \
//    } while (0)
//#endif /* __GNUC__ || __linux__ */

#endif /* MNW_DEBUG_*/
