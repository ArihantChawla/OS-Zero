#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zero/trix.h>
#if (PRIMEPROF)
#include <zero/prof.h>
#endif

#define PRIMEN 1048576

/* Sieve of Erastosthenes with the optimisation to start from l^2 */
void *
sieve(size_t lim)
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
        for (m = l * l ; m < PRIMEN ; m += l) {
            clrbit(tab, m);
        }
        l++;
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

int
main(int argc, char *argv[])
{
    void *tab1;
    void *tab2;
    int   i;
    PROFDECLCLK(clk);

    sleep(2);
    profstartclk(clk);
    tab1 = sieve(PRIMEN);
    profstopclk(clk);
    printf("%ld\n", profclkdiff(clk));

    sleep(2);
    profstartclk(clk);
    tab2 = sieve2(PRIMEN);
    profstopclk(clk);
    printf("%ld\n", profclkdiff(clk));
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

