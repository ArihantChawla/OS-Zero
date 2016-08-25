#include <stdint.h>
#include <stdio.h>
#include <zero/param.h>

#define _min(a, b) ((b) ^ (((a) ^ (b)) & -((a) < (b))))

/* NOTE: THIS ONE IS BORKED :) */
unsigned int
_divu131071(unsigned int uval)
{
    unsigned int mul = 0x80004001;
    unsigned int res = uval;
    unsigned int cnt = 16;
    
    res *= mul;
    res >>= cnt;
    
    return res;
}
//#define _modu131071(u) ((u) - (_divu131071(u) * 131071))

/* hashpwj from the dragon book as supplied on the internet */
#define PRIME 131071    /* was 211 in the implementation I saw */

unsigned int
hashpjw(char *str)
{
    unsigned char *ucp; // changed from char
    unsigned int   u;
    unsigned int   g;
    unsigned int   h = 0;
    

    for (ucp = (unsigned char *)str ; (*ucp) ; ucp++) {
        u = *ucp;
        h <<= 4;
        h += u;
        g = h & 0xf0000000U;
        /* branch eliminated */
        h ^= (g >> 24);
        h ^= g;
    }

    return h % PRIME;
}

#define MULT 31
/* this one was on the net; said to be from the book programming pearls */
unsigned int
pphash(char *str)
{
    unsigned char *ucp = (unsigned char *)str;
    unsigned int   h = 0;
    int            k = 2;

    while (k--) {
        if (!*ucp) {

            break;
        }
        h = MULT * h + *ucp;
        ucp++;
    }

    return h % PRIME;
}

#define SEED 0xf0e1d2
/* let's try Mersenne primes */
#define SEED32 (UINT32_C(0x7fffffff))
#define SEED64 ((UINT64_C(2) << 61) - 1)
/* Ramakrishna & Zobel hash function, improvised for 32- and 64-bit keys*/
#define SHLCNT 7
#define SHRCNT 2
#if (LONGSIZE == 8) || (LONGLONGSIZE == 8)
uint64_t
#elif (LONGSIZE == 4)
uint32_t
#endif
razohash(void *ptr, size_t len, size_t nbit)
{
#if (LONGSIZE == 8) || (LONGLONGSIZE == 8)
    uint64_t hash = SEED64;
#elif (LONGSIZE == 4)
    uint32_t hash = (len <= 4) ? SEED32 : SEED;
#endif
    if (len == 8) {
        uint64_t *vp = (uint64_t *)&ptr;
        uint64_t  val = *vp;

        hash ^= (hash << SHLCNT) + (hash >> SHRCNT) + (val & 0xffU);
        hash ^= (hash << SHLCNT) + (hash >> SHRCNT) + ((val >> 8) & 0xffU);
        hash ^= (hash << SHLCNT) + (hash >> SHRCNT) + ((val >> 16) & 0xffU);
        hash ^= (hash << SHLCNT) + (hash >> SHRCNT) + ((val >> 24) & 0xffU);
        hash ^= (hash << SHLCNT) + (hash >> SHRCNT) + ((val >> 32) & 0xffU);
        hash ^= (hash << SHLCNT) + (hash >> SHRCNT) + ((val >> 40) & 0xffU);
        hash ^= (hash << SHLCNT) + (hash >> SHRCNT) + ((val >> 48) & 0xffU);
        hash ^= (hash << SHLCNT) + (hash >> SHRCNT) + ((val >> 56) & 0xffU);
    } else if (len == 4) {
        uint32_t *vp = ptr;
    } else {
        unsigned char *ucp = (unsigned char *)ptr;
        unsigned int   u;

        while (*ucp) {
            u = *ucp;
            hash ^= (hash << 7) + (hash >> 2) + u;
            ucp++;
        }
    }
    hash &= (1UL << nbit) - 1;

    return hash;
}

#if 0
int
main(int argc, char *argv[])
{
    hashpjw("vendu");

    return 0;
}
#endif

