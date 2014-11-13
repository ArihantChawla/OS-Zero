#define IP4NPKT    16348
#define IP4PKTSIZE 65536
#include <stddef.h>
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

uint16_t ip4chksum16(const uint8_t *buf, size_t size)
{
    uint32_t sum = 0;
    int            i;
    
    /* Accumulate checksum */
    for (i = 0; i < size - 1; i += 2) {
        uint16_t word16 = *(uint16_t *)&buf[i];

        sum += word16;
    }
    
    /* Handle odd-sized case */
    if (size & 1) {
        uint16_t word16 = (uint8_t)buf[i];

        sum += word16;
    }
    
    /* Fold to get the ones-complement result */
    while (sum >> 16) sum = (sum & 0xFFFF)+(sum >> 16);

    /* Invert to get the negative in ones-complement arithmetic */
    return ~sum;
}

uint16_t ip4chksum64(const uint8_t *buf, size_t size)
{
    uint64_t        sum = 0;
    const uint64_t *b = (uint64_t *)buf;
    
    uint32_t t1, t2;
    uint16_t t3, t4;
    
    /* Main loop - 8 bytes at a time */
    while (size >= sizeof(uint64_t)) {
        uint64_t s = *b++;

        sum += s;
        if (sum < s) sum++;
        size -= 8;
    }
    
    /* Handle tail less than 8-bytes long */
    buf = (const uint8_t *) b;
    if (size & 4) {
        uint32_t s = *(uint32_t *)buf;

        sum += s;
        if (sum < s) sum++;
        buf += 4;
    }
    
    if (size & 2) {
        uint16_t s = *(uint16_t *)buf;

        sum += s;
        if (sum < s) sum++;
        buf += 2;
    }
    
    if (size) {
        uint8_t s = *(uint8_t *)buf;

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
        chk1tab[l] = ip4chksum16((const uint8_t *)pkttab[l], lentab[l]);
    }
    profstopclk(clk);
    fprintf(stderr, "%ld microseconds\n", profclkdiff(clk));
    /* make the compiler not optimise the loop above out :) */
    profstartclk(clk);
    for (l = 0 ; l < IP4NPKT ; l++) {
        chk2tab[l] = ip4chksum64((const uint8_t *)pkttab[l], lentab[l]);
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

