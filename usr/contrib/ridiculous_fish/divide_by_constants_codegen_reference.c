#include <stdio.h>
#include <stdlib.h>

/*
  Reference implementations of computing and using the "magic number" approach to dividing
  by constants, including codegen instructions. The unsigned division incorporates the
  "round down" optimization per ridiculous_fish.

  This is free and unencumbered software. Any copyright is dedicated to the Public Domain.
*/

#include <stdint.h> // for uint32_t etc. types -vendu
#include <limits.h> //for CHAR_BIT
#include <assert.h>

/* Types used in the computations below. These can be redefined to the types appropriate
   for the desired division type (i.e. uval can be defined as unsigned long long).
   
   Note that the uval type is used in compute_signed_magic_info, so the uval type must
   not be smaller than the sval type.
   
*/
#define REG_BITS 32
typedef unsigned int uval;
typedef signed int sval;

#if 0
typedef uint32_t uval;
typedef int32_t  sval;
#endif


/* Computes "magic info" for performing signed division by a fixed integer D.
   The type 'sval' is assumed to be defined as a signed integer type large enough
   to hold both the dividend and the divisor.
   Here >> is arithmetic (signed) shift, and >>> is logical shift.
 
   To emit code for n/d, rounding towards zero, use the following sequence:
 
     m = compute_signed_magic_info(D)
     emit("result = (m.multiplier * n) >> SVAL_BITS");
     if d > 0 and m.multiplier < 0: emit("result += n")
     if d < 0 and m.multiplier > 0: emit("result -= n")
     if m.post_shift > 0: emit("result >>= m.shift")
     emit("result += (result < 0)")
 
  The shifts by SVAL_BITS may be "free" if the high half of the full multiply
  is put in a separate register.
 
  The final add can of course be implemented via the sign bit, e.g.
      result += (result >>> (SVAL_BITS - 1))
   or
      result -= (result >> (SVAL_BITS - 1))
      
   This code is heavily indebted to Hacker's Delight by Henry Warren.
   See http://www.hackersdelight.org/HDcode/magic.c.txt
   Used with permission from http://www.hackersdelight.org/permissions.htm
 */

struct magics_info {
    sval multiplier; // the "magic number" multiplier
    unsigned shift; // shift for the dividend after multiplying
};
struct magics_info compute_signed_magic_info(sval D);


/* Computes "magic info" for performing unsigned division by a fixed positive integer D.
   The type 'uval' is assumed to be defined as an unsigned integer type large enough
   to hold both the dividend and the divisor. num_bits can be set appropriately if n is
   known to be smaller than the largest uval; if this is not known then pass
   (sizeof(uval) * CHAR_BIT) for num_bits.
 
   Assume we have a hardware register of width UVAL_BITS, a known constant D which is
   not zero and not a power of 2, and a variable n of width num_bits (which may be
   up to UVAL_BITS). To emit code for n/d, use one of the two following sequences
   (here >>> refers to a logical bitshift):
 
     m = compute_unsigned_magic_info(D, num_bits)
     if m.pre_shift > 0: emit("n >>>= m.pre_shift")
     if m.increment: emit("n = saturated_increment(n)")
     emit("result = (m.multiplier * n) >>> UVAL_BITS")
     if m.post_shift > 0: emit("result >>>= m.post_shift")

   or
 
     m = compute_unsigned_magic_info(D, num_bits)
     if m.pre_shift > 0: emit("n >>>= m.pre_shift")
     emit("result = m.multiplier * n")
     if m.increment: emit("result = result + m.multiplier")
     emit("result >>>= UVAL_BITS")
     if m.post_shift > 0: emit("result >>>= m.post_shift")
 
  The shifts by UVAL_BITS may be "free" if the high half of the full multiply
  is put in a separate register.
 
  saturated_increment(n) means "increment n unless it would wrap to 0," i.e.
    if n == (1 << UVAL_BITS)-1: result = n
    else: result = n+1
  A common way to implement this is with the carry bit. For example, on x86:
     add 1
     sbb 0
 
  Some invariants:
   1: At least one of pre_shift and increment is zero
   2: multiplier is never zero
   
   This code incorporates the "round down" optimization per ridiculous_fish.
 */

struct magicu_info {
    uval multiplier; // the "magic number" multiplier
    unsigned pre_shift; // shift for the dividend before multiplying
    unsigned post_shift; //shift for the dividend after multiplying
    int increment; // 0 or 1; if set then increment the numerator, using one of the two strategies
};
struct magicu_info compute_unsigned_magic_info(uval D, unsigned num_bits);


/* Implementations follow */

struct magicu_info compute_unsigned_magic_info(uval D, unsigned num_bits) {
    
    //The numerator must fit in a uval
    assert(num_bits > 0 && num_bits <= sizeof(uval) * CHAR_BIT);
    
    // D must be larger than zero and not a power of 2
    assert(D & (D-1));
    
    // The eventual result
    struct magicu_info result;
    
    // Bits in a uval
//    const unsigned UVAL_BITS = sizeof(uval) * CHAR_BIT;
    const unsigned UVAL_BITS = num_bits;    
    
    // The extra shift implicit in the difference between UVAL_BITS and num_bits
    const unsigned extra_shift = UVAL_BITS - num_bits;
    
    // The initial power of 2 is one less than the first one that can possibly work
    const uval initial_power_of_2 = (uval)1 << (UVAL_BITS-1);
    
    // The remainder and quotient of our power of 2 divided by d
    uval quotient = initial_power_of_2 / D, remainder = initial_power_of_2 % D;
    
    // ceil(log_2 D)
    unsigned ceil_log_2_D;
    
    // The magic info for the variant "round down" algorithm
    uval down_multiplier = 0;
    unsigned down_exponent = 0;
    int has_magic_down = 0;
    
    // Compute ceil(log_2 D)
    ceil_log_2_D = 0;
    uval tmp;
    for (tmp = D; tmp > 0; tmp >>= 1)
        ceil_log_2_D += 1;
    
    
    // Begin a loop that increments the exponent, until we find a power of 2 that works.
    unsigned exponent;
    for (exponent = 0; ; exponent++) {
        // Quotient and remainder is from previous exponent; compute it for this exponent.
        if (remainder >= D - remainder) {
            // Doubling remainder will wrap around D
            quotient = quotient * 2 + 1;
            remainder = remainder * 2 - D;
        } else {
            // Remainder will not wrap
            quotient = quotient * 2;
            remainder = remainder * 2;
        }
        
        // We're done if this exponent works for the round_up algorithm.
        // Note that exponent may be larger than the maximum shift supported,
        // so the check for >= ceil_log_2_D is critical.
        if ((exponent + extra_shift >= ceil_log_2_D) || (D - remainder) <= ((uval)1 << (exponent + extra_shift)))
            break;
            
        // Set magic_down if we have not set it yet and this exponent works for the round_down algorithm
        if (! has_magic_down && remainder <= ((uval)1 << (exponent + extra_shift))) {
            has_magic_down = 1;
            down_multiplier = quotient;
            down_exponent = exponent;
        }
    }
        
    if (exponent < ceil_log_2_D) {
        // magic_up is efficient
        result.multiplier = quotient + 1;
        result.pre_shift = 0;
        result.post_shift = exponent;
        result.increment = 0;
    } else if (D & 1) {
        // Odd divisor, so use magic_down, which must have been set
        assert(has_magic_down);
        result.multiplier = down_multiplier;
        result.pre_shift = 0;
        result.post_shift = down_exponent;
        result.increment = 1;
    } else {
        // Even divisor, so use a prefix-shifted dividend
        unsigned pre_shift = 0;
        uval shifted_D = D;
        while ((shifted_D & 1) == 0) {
            shifted_D >>= 1;
            pre_shift += 1;
        }
        result = compute_unsigned_magic_info(shifted_D, num_bits - pre_shift);
        assert(result.increment == 0 && result.pre_shift == 0); //expect no increment or pre_shift in this path
        result.pre_shift = pre_shift;
    }
    return result;
}

struct magics_info compute_signed_magic_info(sval D) {
    // D must not be zero and must not be a power of 2 (or its negative)
    assert(D != 0 && (D & -D) != D && (D & -D) != -D);

    // Our result
    struct magics_info result;
    
    // Bits in an sval
    const unsigned SVAL_BITS = sizeof(sval) * CHAR_BIT;
    
    // Absolute value of D (we know D is not the most negative value since that's a power of 2)
    const uval abs_d = (D < 0 ? -D : D);
    
    // The initial power of 2 is one less than the first one that can possibly work
    // "two31" in Warren
    unsigned exponent = SVAL_BITS - 1;
    const uval initial_power_of_2 = (uval)1 << exponent;
    
    // Compute the absolute value of our "test numerator,"
    // which is the largest dividend whose remainder with d is d-1.
    // This is called anc in Warren.
    const uval tmp = initial_power_of_2 + (D < 0);
    const uval abs_test_numer = tmp - 1 - tmp % abs_d;

    // Initialize our quotients and remainders (q1, r1, q2, r2 in Warren)
    uval quotient1 = initial_power_of_2 / abs_test_numer, remainder1 = initial_power_of_2 % abs_test_numer;
    uval quotient2 = initial_power_of_2 / abs_d, remainder2 = initial_power_of_2 % abs_d;
    uval delta;
    
    // Begin our loop
    do {
        // Update the exponent
        exponent++;
        
        // Update quotient1 and remainder1
        quotient1 *= 2;
        remainder1 *= 2;
        if (remainder1 >= abs_test_numer) {
            quotient1 += 1;
            remainder1 -= abs_test_numer;
        }
        
        // Update quotient2 and remainder2
        quotient2 *= 2;
        remainder2 *= 2;
        if (remainder2 >= abs_d) {
            quotient2 += 1;
            remainder2 -= abs_d;
        }
        
        // Keep going as long as (2**exponent) / abs_d <= delta
        delta = abs_d - remainder2;
    } while (quotient1 < delta || (quotient1 == delta && remainder1 == 0));
    
    result.multiplier = quotient2 + 1;
    if (D < 0) result.multiplier = -result.multiplier;
    result.shift = exponent - SVAL_BITS;
    return result;
}

// added by vendu

/*
 *   if n == (1 << UVAL_BITS)-1: result = n
 *   else: result = n+1
*/

int
main(int argc, char *argv[])
{
    struct magicu_info info;
    unsigned int       uval;
    unsigned int       cnt;
    unsigned int       nbit = sizeof(unsigned long);

    if (argc > 1) {
        uval = strtoul(argv[1], NULL, 10);
        if (argc == 3) {
            nbit = strtoul(argv[2], NULL, 10);
        }
        info = compute_unsigned_magic_info(uval, nbit);
    } else {
        uval = 1193182;
        info = compute_unsigned_magic_info(1193182, nbit);
    }
    cnt = info.post_shift;
#if 0
    if (cnt) {
        cnt += REG_BITS;
    }
#endif
    fprintf(stderr, "MUL: %lu, pre: %ud, post: %lu, incr %d\n",
            (unsigned long)info.multiplier,
            info.pre_shift,
            cnt,
            info.increment);
    if (nbit == 32) {
        printf("unsigned int\n");
        printf("divu%ld(unsigned int uval)\n", uval);
    } else {
        printf("unsigned long\n");
        printf("divu%ld(unsigned long uval)\n", uval);
    }
    printf("{\n");
    if (nbit == 32) {
        printf("\tuint32_t mul = UINT32_C(%ld);\n",
               (unsigned long)info.multiplier);
        printf("\tuint32_t res = uval;\n");
        printf("\tuint32_t cnt = %lu;\n\n", cnt);
    } else {
        printf("\tunsigned long long mul = UINT64_C(0x%lx);\n",
               (unsigned long)info.multiplier);
        printf("\tunsigned long long res = uval;\n");
        printf("\tunsigned long      cnt = %lu;\n\n", cnt);
    }
    if (info.pre_shift) {
        printf("\tres >>= %u;\n", info.pre_shift);
    }
    if (info.increment) {
        printf("\tres = max(res + 1, ~0UL);\n");
    }
    printf("\tres *= mul;\n");
    printf("\tres >>= cnt;\n");
    printf("\tuval = (unsigned long)res;\n");
    printf("\n\treturn uval;\n");
    printf("}\n");

    exit(0);
}

