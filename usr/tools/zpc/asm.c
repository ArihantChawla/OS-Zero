#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wpm/asm.h>
#include <zpc/asm.h>

char *opnametab[ZPCNASMOP]
= {
    "ILL",
    "not",
    "shr",
    "shra",
    "shl",
    "xor",
    "or",
    "and",
    "ror",
    "rol",
    "inc",
    "dec",
    "add",
    "sub",
    "mul",
    "div",
    "mod",
    "bz",
    "bnz",
    "blt",
    "ble",
    "bgt",
    "bge",
    "mov",
    "call",
    "ret",
    "trap",
    "iret"
};
uint64_t zpcintregs[ZPCNREG];

asmuword_t
zpcgetreg(uint8_t *str, uint8_t **retptr)
{
    asmuword_t reg = 0;
    
#if (ASMDEBUG)
    fprintf(stderr, "getreg: %s\n", str);
#endif
    if (*str == 'r') {
        str++;
        while ((*str) && isdigit(*str)) {
            reg *= 10;
            reg += *str - '0';
            str++;
        }
        while (*str == ')' || *str == ',') {
            str++;
        }
        *retptr = str;
    } else if (*str == 's' && str[1] == 't') {
        str += 2;
        while ((*str) && isdigit(*str)) {
            reg *= 10;
            reg += *str - '0';
            str++;
        }
        while (*str == ')' || *str == ',') {
            str++;
        }
        reg |= ZPCREGSTKBIT;
        *retptr = str;
    } else {
        fprintf(stderr, "invalid register name %s\n", str);
        
        exit(1);
    }
    
    return reg;
}

