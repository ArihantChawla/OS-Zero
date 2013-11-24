#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zero/trix.h>
#if (PRIMEPROF)
#include <zero/prof.h>
#endif

#define PRIMEN 1048576

/*
 * Thanks to Warren Driscoll for spotting some redundant code and optimisations.
 */

/* Sieve of Erastosthenes with the optimisation to start from l^2 */
void *
sieve1(size_t lim)
{
    void *tab;
    long  l;
    long  m;
    
    tab = malloc(PRIMEN >> 3 * sizeof(char));
    if (tab) {
        memset(tab, 0xff, (PRIMEN >> 3) * sizeof(char));
        l = 2;
        while (1) {
            for (m = l * l ; m < PRIMEN ; m += l) {
                clrbit(tab, m);
            }
            for ( ++l ; l < PRIMEN ; l++) {
                if (bitset(tab, l)) {
                    
                    break;
                }
            }
            if (l == PRIMEN) {
                
                break;
            }
        }
    }
    
    return tab;
}

void *
sieve2(size_t lim)
{
    void *tab;
    long  l;
    long  m;
    
    tab = malloc(PRIMEN >> 3 * sizeof(char));
    if (tab) {
        memset(tab, 0xff, (PRIMEN >> 3) * sizeof(char));
        l = 2;
        while (1) {
            for (m = l * l ; m < PRIMEN ; m += l) {
                clrbit(tab, m);
            }
            l++;
            if (!(l & 0x01)) {
                l++;
            }
            while (l < PRIMEN) {
                if (bitset(tab, l)) {
                    
                    break;
                }
                l += 2;
            }
            if (l >= PRIMEN) {
                
                break;
            }
        }
    }
    
    return tab;
}

void *
sieve3(size_t lim)
{
    void *tab;
    long  l;
    long  m;
    
    tab = malloc(PRIMEN >> 3 * sizeof(char));
    if (tab) {
        memset(tab, 0xff, (PRIMEN >> 3) * sizeof(char));
        l = 2;
        while (1) {
            for (m = l * l ; m < PRIMEN ; m += l) {
                clrbit(tab, m);
            }
            l++;
            l += !(l & 0x01);
            while (l < PRIMEN) {
                if (bitset(tab, l)) {
                    
                    break;
                }
                l += 2;
            }
            if (l >= PRIMEN) {
                
                break;
            }
        }
    }
    
    return tab;
}

void *
sieve4(size_t lim)
{
    void *tab;
    long  l;
    long  m;
    
    tab = malloc(PRIMEN >> 3 * sizeof(char));
    if (tab) {
        memset(tab, 0xff, (PRIMEN >> 3) * sizeof(char));
        l = 2;
        for (m = l * l ; m < PRIMEN ; m += l) {
            clrbit(tab, m);
        }
        l++;
        while (1) {
            for (m = l * l ; m < PRIMEN ; m += (l << 1)) {
                clrbit(tab, m);
            }
            l++;
            l += !(l & 0x01);
            while (l < PRIMEN) {
                if (bitset(tab, l)) {
                    
                    break;
                }
                l += 2;
            }
            if (l >= PRIMEN) {
                
                break;
            }
        }
    }
    
    return tab;
}

void *
sieve5(size_t lim)
{
    void *tab;
    long  l;
    long  m;

    tab = malloc(PRIMEN >> 3 * sizeof(char));
    if (tab) {
        memset(tab, 0xff, (PRIMEN >> 3) * sizeof(char));
        for (l = 1 ; l < PRIMEN ; l++) {
            
        }
    }
}

int
main(int argc, char *argv[])
{
    uint8_t *tab1;
    uint8_t *tab2;
    uint8_t *tab3;
    uint8_t *tab4;
    int      i;
    PROFDECLCLK(clk);

    sleep(1);
    profstartclk(clk);
    tab1 = sieve1(PRIMEN);
    profstopclk(clk);
    printf("%ld\n", profclkdiff(clk));

    sleep(1);
    profstartclk(clk);
    tab2 = sieve2(PRIMEN);
    profstopclk(clk);
    printf("%ld\n", profclkdiff(clk));

    for (i = 2 ; i < PRIMEN >> 3 ; i++) {
        if (tab2[i] != tab1[i]) {
            fprintf(stderr, "ERROR sieve2(%d)\n", i);

            exit(1);
        }
    }

    sleep(1);
    profstartclk(clk);
    tab3 = sieve3(PRIMEN);
    profstopclk(clk);
    printf("%ld\n", profclkdiff(clk));

    for (i = 0 ; i < PRIMEN >> 3 ; i++) {
        if (tab3[i] != tab1[i]) {
            fprintf(stderr, "ERROR sieve2(%d)\n", i);

            exit(1);
        }
    }

    sleep(1);
    profstartclk(clk);
    tab4 = sieve4(PRIMEN);
    profstopclk(clk);
    printf("%ld\n", profclkdiff(clk));

    for (i = 0 ; i < PRIMEN >> 3 ; i++) {
        if (tab4[i] != tab1[i]) {
            fprintf(stderr, "ERROR sieve2(%d)\n", i);

            exit(1);
        }
    }

#if 0
    for (i = 2 ; i < PRIMEN ; i++) {
        if (bitset(tab2, i)) {
            printf("%d\n", i);
        }
//        printf("%d\t%lx\n", i, bitset(tab1, i));
    }
#endif

    exit(0);
}

