#if defined(__arm__)

#include <mach/asm.h>

extern void trapinit(void);

void
kmain(void) {

    while (1) {
        m_waitint();
    }

    /* NOTREACHED */
}

#endif /* __arm__ */

