#define IP4NPKT    16348
#define IP4PKTSIZE 65536
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
//#include <zero/param.h>
#include <zero/trix.h>
#if (IP4TEST)
#include <stdio.h>
#include <string.h>
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

#if (LONGSIZE == 4) || (IP4TEST)

uint_fast16_t ip4chksum16(const uint8_t *buf, size_t size)
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

#endif /* LONGSIZE == 4 || IP4TEST */

#if (LONGSIZE == 8) || (IPTEST)

uint_fast16_t
ip4chksum64(const uint8_t *buf, size_t size)
{
    uint64_t        sum = 0;
    const uint64_t *ptr = (uint64_t *)buf;
    size_t          len = size >> 3;
    uint32_t        tmp1;
    uint32_t        tmp2;
    uint16_t        tmp3;
    uint16_t        tmp4;

    size -= len << 3;
    while (len--) {
        uint64_t u = *ptr;

        sum += u;
        ptr++;
        if (sum < u) {
            sum++;
        }
   }
    
    while (size >= sizeof(uint64_t)) {
        uint64_t u = *ptr;

        sum += u;
        size -= sizeof(uint64_t);
        ptr++;
        if (sum < u) {
            sum++;
        }
    }
    
    /* Handle tail less than 8-bytes long */
    buf = (const uint8_t *)ptr;
    if (size & 4) {
        uint32_t u = *(uint32_t *)buf;

        sum += u;
        buf += sizeof(uint32_t);
        if (sum < u) {
            sum++;
        }
    }
    
    if (size & 2) {
        uint16_t u = *(uint16_t *)buf;

        sum += u;
        buf += sizeof(uint16_t);
        if (sum < u) {
            sum++;
        }
    }
    
    if (size) {
        uint8_t u = *(uint8_t *)buf;

        sum += u;
        if (sum < u) sum++;
    }
    
    /* Fold down to 16 bits */
    tmp1 = sum;
    tmp2 = sum >> 32;
    tmp1 += tmp2;
    if (tmp1 < tmp2) tmp1++;
    tmp3 = tmp1;
    tmp4 = tmp1 >> 16;
    tmp3 += tmp4;
    if (tmp3 < tmp4) tmp3++;
    
    return ~tmp3;
}

#endif /* LONGSIZE == 8 || IP4TEST */

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
    unsigned long long nbyte;

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
    nbyte = 0;
    for (l = 0 ; l < IP4NPKT ; l++) {
        nbyte += lentab[l];
    }
    fprintf(stderr, "%llu\n", nbyte);
    profstartclk(clk);
    for (l = 0 ; l < IP4NPKT ; l++) {
        chk1tab[l] = ip4chksum64((const uint8_t *)pkttab[l], lentab[l]);
    }
    profstopclk(clk);
    fprintf(stderr, "%ld microseconds\n", profclkdiff(clk));
    fprintf(stderr, "%f KB/s\n", (1000000.0 * ((double)nbyte / (double)profclkdiff(clk))) / 1024.0);
#if 0
    profstartclk(clk);
    for (l = 0 ; l < IP4NPKT ; l++) {
        chk2tab[l] = ip4chksum64_2((const uint8_t *)pkttab[l], lentab[l]);
    }
    profstopclk(clk);
    fprintf(stderr, "%ld microseconds\n", profclkdiff(clk));
#endif
    memcpy(chk2tab, chk1tab, sizeof(uint16_t) * IP4NPKT);
    for (l = 0 ; l < IP4NPKT ; l++) {
        if (chk1tab[l] != chk2tab[l]) {
            fprintf(stderr, "%ld\n", l);

            exit(1);
        }
    }
    
    return 0;
}

#endif

