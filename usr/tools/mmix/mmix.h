#ifndef __MMIX_MMIX_H__
#define __MMIX_MMIX_H__

#include <stdint.h>

#define MMIXNREG  256   // # of general-purpose register
#define MMIXNXREG 32    // # of special-purpose registers
#define MMIXNOP   256

struct mmixop {
    uint8_t code;       // instruction type
    uint8_t dest;       // destination
    uint8_t src;        // source
    uint8_t args;       // operands for the instruction
};

struct mmixunit {
    uint64_t regs[MMIXNREG];
    uint64_t xregs[MMIXNXREG];
};

#endif /* __MMIX_MMIX_H__ */

