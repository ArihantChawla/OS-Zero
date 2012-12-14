#if defined(__arm__)

#include <zero/arm/asm.h>

void
kmain(void) {

    while (1) {
        m_waitint();
    }

    /* NOTREACHED */
}

#endif /* __arm__ */

