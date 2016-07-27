#ifndef __ZERO_TAGPTR_H__
#define __ZERO_TAGPTR_H__

/* tagged pointers for libzero */

#if defined(__x86_64__) || defined(__amd64__)

struct tagptr {
    union {
        __m128             m128;        // force dualword-alignment
        struct {
            void          *adr;
            unsigned long  tag;
        }
    }
};

#endif

#endif /* __ZERO_TAGPTR_H__ */

