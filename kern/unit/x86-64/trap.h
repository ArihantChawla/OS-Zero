#ifndef __UNIT_X86_64_TRAP_H__
#define __UNIT_X86_64_TRAP_H__

#include <kern/unit/x86/trap.h>

#define trapsetintgate(p, func, ring)                                   \
    do {                                                                \
        uint64_t lo;                                                    \
        uint64_t hi;                                                    \
                                                                        \
        p[0] = trapmkdesc(TEXTSEL, (func) & 0xffffffff, ring);          \
        p[1] = (func) & 0xffffffff00000000;
    } while (0)

#endif /* __UNIT_X86_64_TRAP_H__ */

