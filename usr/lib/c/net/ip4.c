#define IP4NPKT    16348
#define IP4PKTSIZE 65536
#include <stdlib.h>
#include <stdint.h>
#include <zero/trix.h>
#if (IP4TEST)
#include <stdio.h>
#include <zero/prof.h>
#include <zero/randmt32.h>
#endif


/*
 * This code is based on what I found at
 * http://locklessinc.com/articles/tcp_checksum/
 */

#if (LONGSIZE == 4)
#define ip4chksum(buf, sz) ip4chksum16(buf, sz)
#elif (LONGSIZE == 8)
#define ip4chksum(buf, sz) ip4chksum64(buf, sz)
#endif

unsigned short ip4chksum16(const char *buf, unsigned size)
{
    unsigned sum = 0;
    int i;
    
    /* Accumulate checksum */
    for (i = 0; i < size - 1; i += 2) {
        unsigned short word16 = *(unsigned short *)&buf[i];
        sum += word16;
    }
    
    /* Handle odd-sized case */
    if (size & 1) {
        unsigned short word16 = (unsigned char)buf[i];
        sum += word16;
    }
    
    /* Fold to get the ones-complement result */
    while (sum >> 16) sum = (sum & 0xFFFF)+(sum >> 16);

    /* Invert to get the negative in ones-complement arithmetic */
    return ~sum;
}

unsigned short ip4chksum64(const char *buf, unsigned size)
{
    unsigned long long sum = 0;
    const unsigned long long *b = (unsigned long long *)buf;
    
    unsigned t1, t2;
    unsigned short t3, t4;
    
    /* Main loop - 8 bytes at a time */
    while (size >= sizeof(unsigned long long)) {
        unsigned long long s = *b++;
        sum += s;
        if (sum < s) sum++;
        size -= 8;
    }
    
    /* Handle tail less than 8-bytes long */
    buf = (const char *) b;
    if (size & 4) {
        unsigned s = *(unsigned *)buf;
        sum += s;
        if (sum < s) sum++;
        buf += 4;
    }
    
    if (size & 2) {
        unsigned short s = *(unsigned short *)buf;
        sum += s;
        if (sum < s) sum++;
        buf += 2;
    }
    
    if (size) {
        unsigned char s = *(unsigned char *)buf;
        sum += s;
        if (sum < s) sum++;
    }
    
    /* Fold down to 16 bits */
    t1 = sum;
    t2 = sum >> 32;
    t1 += t2;
    if (t1 < t2) t1++;
    t3 = t1;
    t4 = t1 >> 16;
    t3 += t4;
    if (t3 < t4) t3++;
    
    return ~t3;
}

#if (IP4TEST)

int
main(int argc, char *argv[])
{
    uint64_t *pkttab[IP4NPKT];
    size_t    lentab[IP4NPKT];
    uint16_t  chk1tab[IP4NPKT];
    uint16_t  chk2tab[IP4NPKT];
    PROFDECLCLK(clk);
    uint8_t  *ptr;
    long      l;
    long      n;
    uint16_t  len;

    srandmt32(666);
    for (l = 0 ; l < IP4NPKT ; l++) {
        len = randmt32() & (IP4PKTSIZE - 1);
        lentab[l] = len;
        pkttab[l] = calloc(rounduppow2(len, sizeof(uint64_t)) / sizeof(uint64_t), sizeof(uint64_t));
        ptr = (uint8_t *)pkttab[l];
        for (n = 0 ; n < len ; n++) {
            ptr[n] = randmt32() % 0xff;
        }
    }
    profstartclk(clk);
    for (l = 0 ; l < IP4NPKT ; l++) {
        chk1tab[l] = ip4chksum16((const char *)pkttab[l], lentab[l]);
    }
    profstopclk(clk);
    fprintf(stderr, "%ld microseconds\n", profclkdiff(clk));
    /* make the compiler not optimise the loop above out :) */
    profstartclk(clk);
    for (l = 0 ; l < IP4NPKT ; l++) {
        chk2tab[l] = ip4chksum64((const char *)pkttab[l], lentab[l]);
    }
    profstopclk(clk);
    fprintf(stderr, "%ld microseconds\n", profclkdiff(clk));
    for (l = 0 ; l < IP4NPKT ; l++) {
        if (chk1tab[l] != chk2tab[l]) {
            fprintf(stderr, "%ld\n", l);

            exit(1);
        }
    }
    
    return 0;
}

#endif

