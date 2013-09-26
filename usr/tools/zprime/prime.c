#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zero/trix.h>

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

int
main(int argc, char *argv[])
{
    void *tab1;
    int   i;

    tab1 = sieve(PRIMEN);
    for (i = 2 ; i < PRIMEN ; i++) {
        if (bitset(tab1, i)) {
            printf("%d\n", i);
        }
//        printf("%d\t%lx\n", i, bitset(tab1, i));
    }

    exit(0);
}

