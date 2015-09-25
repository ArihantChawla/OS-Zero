#include <stdio.h>
#include <stdlib.h>

/** This function adds the two arguments using bitwise operators. Your      
* implementation should not use arithmetic operators except for loop
* control. Integers are 32 bits long.  This function prints a message
* saying "Overflow occurred\n" if a two's complement overflow occurs
* during the addition process. The sum is returned as the value of
* the function.
*/

long badd(long x,long y)
{
    
    long i;
    
    char sum;
    char car_in=0;
    char car_out;
    char a,b;
    
    unsigned long mask=0x00000001;
    long result=0;

    for(i=0;i<32;i++){
        
        a=(x&mask)!=0;
        b=(y&mask)!=0;
        car_out=car_in & (a|b) |a&b;
        sum=a^b^car_in;

        if(sum) {
            result|=mask;
        }

        if(i!=31) {
            car_in=car_out;
        } else {
            if(car_in!=car_out) {
                printf("Overflow occurred\n");
            }
        }

        mask<<=1;
    }

    return result;
}

// subracts two integers by finding the compliemnt
// of "y", adding 1, and using the badd() function
// to add "-y" and "x"
long bsub(long x, long y){

    return badd(x, badd(~y, 1));
}

#if 0
//add x to total for however many y
long bmult(long x,long y){

    long total;
    long i;
    for(i=0; i < = y; i++)
        {
            total = badd(total,x)
                }
    return total;
}

// comment me
unsigned long bdiv(unsigned long dividend, unsigned long divisor){

// write me
    return 0;
}
#endif

unsigned long bmult(unsigned long x, unsigned long y)
{
    long total = 0;
    long i;

    /* if the i-th bit is non-zero, add 'x' to total */
    /* Multiple total by 2 each step */
    for(i = 32 ; i >= 0 ; i--)
        {
            total <<= 1;
            if( (y & (1 << i)) >> i )
                {
                    total = badd(total, x);
                }
        }

    return total;
}

unsigned long bdiv(unsigned long dividend, unsigned long divisor)
{
    long i, quotient = 0, remainder = 0;

    if(divisor == 0) { printf("div by zero\n"); return 0; }

    for(i = 31 ; i >= 0 ; i--)
        {
            quotient <<= 1;
            remainder <<= 1;
            remainder |= (dividend & (1 << i)) >> i;
            fprintf(stderr, "%ld (%lx): q: %lx, r: %lx\n",
                    i, dividend & (1 << i), quotient, remainder);

            if(remainder >= divisor)
                {
                    remainder = bsub(remainder, divisor);
                    quotient |= 1;
                }
        }

    return quotient;
}

int
main(int argc, char *argv[])
{
    unsigned long val = 1UL << 48;
    unsigned long div = 1000000000;

    fprintf(stderr, "DIVISOR: %lx (%lx)\n", div, val / div);
    bdiv(val, div);

    exit(0);
}

