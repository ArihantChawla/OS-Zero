#include <stdint.h>
#include <stdio.h>

#define _min(a, b) ((b) ^ (((a) ^ (b)) & -((a) < (b))))

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
#define _modu131071(u) ((u) - (_divu131071(u) * 131071))

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
    u = _modu131071(h);
    fprintf(stderr, "%u (%u)\n", h % PRIME, u >> 16);

    return _modu131071(h);
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
/* Ramakrishna & Zobel hash function */
unsigned long
razohash(const char *str)
{
    unsigned char *ucp = (unsigned char *)str;
    unsigned long  hash = SEED;
    unsigned int   u;

    while (*ucp) {
        u = *ucp;
        hash ^= (hash << 7) + (hash >> 2) + u;
        ucp++;
    }

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

