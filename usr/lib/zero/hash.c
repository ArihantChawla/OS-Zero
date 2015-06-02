/* libzero hash routines */

/*
 * This stuff is based on Chris Anderson's C-conversion of the code described
 * at http://locklessinc.com/articles/fast_hash/
 * The full original source is at http://www.eetbeetee.org/h.c
 */

#if (__KERNEL__)
#undef HASHTEST
#define HASHTEST 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#if defined(__amd64__) || defined(__x86_64__)
#define bswapq128(r)                                                   \
    __asm__ __volatile__ ("bswapq %0"                                  \
                          : "+r" (r)                                   \
                          :)
#define mulq128(a, d, r)                                                \
    __asm__ __volatile__ ("mulq %2"                                     \
                          : "+a" (a), "=d" (d)                          \
                          : "r" (r))
#else
#include <byteswap.h>
#define bswapq128(r)     ((r) = bswap_64(r))
#define mulq128(a, d, r) ((d) = (a) * (r))
#endif

uint64_t
hashq128(const void *ptr,
         size_t len,
         size_t nkeybit)
{
    register uint64_t r8 = 0x1591aefa5e7e5a17ULL;
    register uint64_t r9 = 0x2bb6863566c4e761ULL;
    register uint64_t rax = len ^ r8;
    register uint64_t rcx = r9;
    register uint64_t rdx;
    
    while (len >= 16) {
        rax = (rax ^ ((uint64_t *)ptr)[0]) * r8;
        rcx = (rcx ^ ((uint64_t *)ptr)[1]) * r9;
        bswapq128(rax);
        bswapq128(rcx);
        ptr = &((uint64_t *)ptr)[2];
        len -= 16;
    }
    if (len) {
        if (len & 8) {
            rdx = 0;
            rax ^= ((uint64_t *)ptr)[0];
            ptr = &((uint64_t *)ptr)[1];
        }
        if (len & 4) {
            rdx = ((uint32_t *)ptr)[0];
            ptr = &((uint32_t *)ptr)[1];
        }
        if (len & 2) {
            rdx = (rdx << 16) | ((uint16_t *)ptr)[0];
            ptr = &((uint16_t *)ptr)[1];
        }
        if (len & 1) {
            rdx = (rdx << 8) | ((uint8_t *)ptr)[0];
        }
        rcx ^= rdx;
    }
    mulq128(rax, rdx, r8);
    rcx = (rcx * r9) + rdx;
    rax ^= rcx;
    mulq128(rax, rdx, r8);
    rcx = (rcx * r9) + rdx;
    rax ^= rcx;
    
    if (nkeybit < 32) {
        rcx = rax >> 32;
        rax ^= rcx;
    }
    rax &= (1UL << nkeybit) - 1;
    
    return rax;
}

#if (HASHTEST)

int
main(int argc, char *argv[])
{
    unsigned long long  key;
    void               *ptr;
    long                l = argc;

    for (l = 1 ; l < argc ; l++) {
        key = hashq128(argv[l], strlen(argv[l]), 64);
        fprintf(stderr, "%s: 0x%llx\n", argv[l], key & 0xff);
    }

    exit(0);
}

#endif

