/*
 * NOTE: allow including multiple times; the effect depends on definition of
 * NDEBUG.
 */

#undef assert
#ifdef NDEBUG

#define assert(cond) ((void)0)

#else

#include <zero/cdecl.h>

#define assert(cond)                                                    \
    ((cond)                                                             \
     ? ((void)0)                                                        \
     : (fprintf(stderr,                                                 \
                "Assertion failed: %s, file %s, function %s, line %d\n", \
                #cond,                                                  \
                __FILE__,                                               \
                __func__,                                               \
                __LINE__),                                              \
        abort()))

#endif

