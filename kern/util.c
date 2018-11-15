#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <stdarg.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <zero/trix.h>
#include <kern/printf.h>
#include <kern/io/drv/chr/cons.h>
#include <kern/io/drv/pc/vga.h>
#include <kern/unit/x86/asm.h>
#include <kern/unit/x86/trap.h>

#define CACHEPREWARM    0

/*
 * NOTES
 * -----
 * - do not initialize stack variables at top of functions; do it explicitly in
 *   code to avoid stack problems, at least for linker constants
 */

const char *trapnametab[TRAPSCPU] ALIGNED(PAGESIZE)
= {
    "DE",
    "DB",
    "NMI",
    "BP",
    "OF",
    "BR",
    "UD",
    "NM",
    "DF",
    NULL,
    "TS",
    "NP",
    "SS",
    "GP",
    "PF",
    NULL,
    "MF",
    "AC",
    "MC",
    "XF"
};

/* assumes longword-aligned blocks */
void
kbzero(void *adr, size_t len)
{
    long *next;
    long *ptr = adr;
    long  val = 0;
    long  incr = 8;
    long  nleft = 0;

    if (len > (1UL << (LONGSIZELOG2 + 3))) {
        /* zero non-cacheline-aligned head long-word by long-word */
        nleft = ((uintptr_t)adr) & ((1UL << (LONGSIZELOG2 + 3)) - 1);
        if (nleft) {
            nleft = (1UL << (LONGSIZELOG2 + 3)) - nleft;
            nleft >>= LONGSIZELOG2;
            len -= nleft;
            while (nleft--) {
                *ptr++ = val;
            }
        }
        nleft = len & ~((1UL << (LONGSIZELOG2 + 3)) - 1);
    }
    next = ptr;
    if (len >= (1UL << (LONGSIZELOG2 + 3))) {
        nleft = len & ((1UL << (LONGSIZELOG2 + 3)) - 1);
        len >>= LONGSIZELOG2 + 3;
        /* zero aligned cachelines */
        while (len) {
            len--;
            ptr[0] = val;
            ptr[1] = val;
            ptr[2] = val;
            ptr[3] = val;
            next += incr;
            ptr[4] = val;
            ptr[5] = val;
            ptr[6] = val;
            ptr[7] = val;
            ptr = next;
        }
    }
    while (nleft--) {
        /* zero tail long-words */
        *ptr++ = val;
    }

    return;
}

/* assumes longword-aligned blocks with sizes of long-word multiples */
void
kmemset(void *adr, int byte, size_t len)
{
    long *ptr = adr;
    long *next;
    long  val = 0;
    long  incr = 8;
    long  nleft = 0;

    val = byte;
    val |= (val << 8);
    val |= (val << 16);
#if (LONGSIZE == 8)
    val |= (val << 32);
#endif
    if (len > (1UL << (LONGSIZELOG2 + 3))) {
        /* zero non-cacheline-aligned head long-word by long-word */
        nleft = ((uintptr_t)adr) & ((1UL << (LONGSIZELOG2 + 3)) - 1);
        if (nleft) {
            nleft = (1UL << (LONGSIZELOG2 + 3)) - nleft;
            nleft >>= LONGSIZELOG2;
            len -= nleft;
            while (nleft--) {
                *ptr++ = val;
            }
        }
        nleft = len & ~((1UL << (LONGSIZELOG2 + 3)) - 1);
    }
    next = ptr;
    if (len >= (1UL << (LONGSIZELOG2 + 3))) {
        nleft = len & ((1UL << (LONGSIZELOG2 + 3)) - 1);
        len >>= LONGSIZELOG2 + 3;
        /* zero aligned cachelines */
        while (len) {
            len--;
            ptr[0] = val;
            ptr[1] = val;
            ptr[2] = val;
            ptr[3] = val;
            next += incr;
            ptr[4] = val;
            ptr[5] = val;
            ptr[6] = val;
            ptr[7] = val;
            ptr = next;
        }
    }
    while (nleft--) {
        /* zero tail long-words */
        *ptr++ = val;
    }

    return;
}

void
kmemcpy(void *dest, void *src, m_ureg_t len)
{
    m_ureg_t  nleft = len;
    long     *dptr = NULL;
    long     *sptr = NULL;
    long     *dnext;
    long     *snext;
    long      incr;
#if (CACHEPREWARM) && !defined(__GNUC__)
    long      tmp;
#endif

    dnext = dest;
    snext = src;
    incr = 8;
    /* set loop count */
    len >>= LONGSIZELOG2 + 3;
    nleft -= len << (LONGSIZELOG2 + 3);
    while (len) {
#if defined(__GNUC__)
        __builtin_prefetch(dnext);
        __builtin_prefetch(snext);
#elif (CACHEPREWARM)
        tmp = *dnext;   // cache prewarm; fetch first byte of cacheline
        tmp = *snext;   // cache prewarm; fetch first byte of cacheline
#endif
        dptr = dnext;   // set pointers
        sptr = snext;
        len--;          // adjust loop count
        dnext += incr;  // set next pointers
        snext += incr;
        /* copy memory */
        dptr[0] = sptr[0];
        dptr[1] = sptr[1];
        dptr[2] = sptr[2];
        dptr[3] = sptr[3];
        dptr[4] = sptr[4];
        dptr[5] = sptr[5];
        dptr[6] = sptr[6];
        dptr[7] = sptr[7];
    }
    while (nleft--) {
        *dptr++ = *sptr++;
    }

    return;
}

void
kbfill(void *adr, uint8_t byte, m_ureg_t len)
{
    m_ureg_t  nleft = len;
    long     *ptr = NULL;
    long     *next;
    long      val;
    long      incr;
#if (CACHEPREWARM)
    long      tmp;
#endif

    next = adr;
    val = byte;
    val |= val << 8;
    val |= val << 16;
    incr = 8;
    /* set loop count */
    len >>= LONGSIZELOG2 + 3;
#if (LONGSIZE == 8)
    val |= val << 32;
#endif
    nleft -= len << (LONGSIZELOG2 + 3);
    while (len) {
#if (CACHEPREWARM)
        tmp = *next;    // cache prewarm; fetch first byte of cacheline
#endif
        ptr = next;     // set pointer
        len--;          // adjust loop count
        next += incr;   // set next pointer
        /* fill memory */
        ptr[0] = val;
        ptr[1] = val;
        ptr[2] = val;
        ptr[3] = val;
        ptr[4] = val;
        ptr[5] = val;
        ptr[6] = val;
        ptr[7] = val;
    }
    while (nleft--) {
        *ptr++ = val;
    }

    return;
}

PURE int
kmemcmp(const void *ptr1,
        const void *ptr2,
        m_ureg_t nb)
{
    unsigned char *ucptr1 = (unsigned char *)ptr1;
    unsigned char *ucptr2 = (unsigned char *)ptr2;
    int            retval = 0;

    if (nb) {
        while ((*ucptr1 == *ucptr2) && (nb--)) {
            ucptr1++;
            ucptr2++;
        }
        if (nb) {
            retval = (int)*ucptr1 - (int)*ucptr2;
        }
    }

    return retval;
}

PURE int
kstrcmp(const char *str1,
        const char *str2)
{
    unsigned char *ucptr1 = (unsigned char *)str1;
    unsigned char *ucptr2 = (unsigned char *)str2;
    int            retval = 0;

    while ((*ucptr1) && *ucptr1 == *ucptr2) {
        ucptr1++;
        ucptr2++;
    }
    if (*ucptr1) {
        retval = (int)*ucptr1 - (int)*ucptr2;
    }

    return retval;
}

long
kstrncpy(char *dest, const char *src, long len)
{
    long nb = 0;

    while ((*src) && --len) {
        *dest++ = *src++;
        nb++;
    }
    *dest = '\0';

    return nb;
}

void *
kstrtok(void *ptr, int ch)
{
    uint8_t *u8ptr = ptr;

    while ((u8ptr) && (*u8ptr) && *u8ptr != ch) {
        u8ptr++;
    }
    if ((u8ptr) && (*u8ptr)) {
        *u8ptr = '\0';
    } else {
        u8ptr = NULL;
    }

    return u8ptr;
}

#if defined(__KERNEL__) && (__KERNEL__)

void
krewind2(void *frame, void *symmap)
{
    while (frame) {
        frame = m_getfrmadr1(frame);
        if (symmap) {
            ; /* TODO */
        }
        kprintf("%p\n", frame);
    }

    return;
}

void
kpanic(int32_t trap, long err, void *frame)
{
    const char *name;

    if (trap >= 0) {
        name = trapnametab[trap];
        if (name) {
            kprintf("CAUGHT TRAP %ld (%s): %lx\n",
                    (long)trap, name, err);
        } else {
            kprintf("CAUGHT RESERVED TRAP %ld (%s)\n",
                    (long)trap, name);
        }
        if (frame) {
            kprintf("STACK TRACE\n");
            kprintf("-----------\n");
            krewind2(frame, NULL);
        }
    }
    k_halt();
}

#endif /* defined(__KERNEL__) && (__KERNEL__) */

