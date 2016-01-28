#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <stdarg.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <kern/io/drv/chr/cons.h>
#include <kern/io/drv/pc/vga.h>
#include <kern/unit/x86/asm.h>
#include <kern/unit/x86/trap.h>

#define MAXPRINTFSTR 2048

#define isprintascii(c) ((c) >= 0x20 && (c) < 0x7f)

#define CACHEPREWARM    0
#if (LONGSIZE == 4)
#define LONGBUFSIZE     16
#else
#define LONGBUFSIZE     32
#endif
#define LONGLONGBUFSIZE 32

/*
 * NOTES
 * -----
 * - do not initialize stack variables at top of functions; do it explicitly in
 *   code to avoid stack problems, at least for linker constants
 */

const char *trapnametab[TRAPNCPU] ALIGNED(PAGESIZE)
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

const char _ltoxtab[]
= {
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'a',
    'b',
    'c',
    'd',
    'e',
    'f',
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
    long *next;
    long *ptr;
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
kmemcpy(void *dest, void *src, unsigned long len)
{
    unsigned long  nleft = len;
    long          *dptr = NULL;
    long          *sptr = NULL;
    long          *dnext;
    long          *snext;
    long           incr;
#if (CACHEPREWARM) && !defined(__GNUC__)
    long    tmp;
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
kbfill(void *adr, uint8_t byte, unsigned long len)
{
    unsigned long  nleft = len;
    long          *ptr = NULL;
    long          *next;
    long           val;
    long           incr;
#if (CACHEPREWARM)
    long           tmp;
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

int
kmemcmp(const void *ptr1,
        const void *ptr2,
        unsigned long nb)
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

int
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

    while ((*src) && len--) {
        *dest++ = *src++;
        nb++;
    }
    *dest = '\0';

    return nb;
}

static unsigned long
_ltoxn(long val, char *buf, uintptr_t len)
{
    unsigned long uval = zeroabs(val);
    unsigned long byte;
    unsigned long l = len - 1;
    unsigned long incr = 4;
    unsigned long sign = 0;

    if (val < 0) {
        sign = 1;
    }
    buf[l] = '\0';
    do {
        byte = uval & 0xf;
        l--;
        uval >>= 4;
        buf[l] = _ltoxtab[byte];
    } while (uval);
    if (sign) {
        l--;
        buf[l] = '-';
    }

    return l;
}

static unsigned long
_ltodn(long val, char *buf, uintptr_t len)
{
    unsigned long uval = zeroabs(val);
    unsigned long tmp;
    unsigned long byte;
    unsigned long l = len - 1;
    unsigned long n;
    unsigned long sign = 0;

    if (val < 0) {
        sign = 1;
    }
    buf[l] = '\0';
    do {
        tmp = divu10(uval);
        l--;
        byte = uval - 10 * tmp;
        buf[l] = _ltoxtab[byte];
        uval = tmp;
    } while (uval);
    if (sign) {
        l--;
        buf[l] = '-';
    }

    return l;
}

static unsigned long
_ultoxn(unsigned long uval, char *buf, uintptr_t len)
{
    unsigned long byte;
    unsigned long l = len - 1;

    buf[l] = '\0';
    do {
        byte = uval & 0xf;
        l--;
        uval >>= 4;
        buf[l] = _ltoxtab[byte];
    } while (uval);

    return l;
}

static unsigned long
_ultodn(unsigned long uval, char *buf, uintptr_t len)
{
    unsigned long byte;
    unsigned long tmp;
    unsigned long l = len - 1;
    unsigned long n;

    buf[l] = '\0';
    do {
        tmp = divu10(uval);
        l--;
        byte = uval - 10 * tmp;
        buf[l] = _ltoxtab[byte];
        uval = tmp;
    } while (uval);

    return l;
}

static void *
_strtok(void *ptr, int ch)
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

/*
 * %x, %c, %h, %d, %ld, %uc, %uh, %ud, %ul, %lx, %x, %p
 */
void
kprintf(const char *fmt, ...)
{
//    char    *str = fmt;
    struct cons   *cons;
    char          *arg;
    char          *sptr;
    char          *cptr;
    long           val;
    unsigned long  uval;
    long           isuns;
    long           isch;
    long           isdec;
    long           ishex;
    long           l;
    long           len;
    va_list        al;
    char           buf[LONGLONGBUFSIZE];
    char           str[MAXPRINTFSTR];

    cons = &constab[conscur];
    if (cons->puts) {
        va_start(al, fmt);
        len = MAXPRINTFSTR - 1;
        len = kstrncpy(str, fmt, len);
        sptr = str;
        while (*sptr) {
            isch = 0;
            isdec = 0;
            ishex = 0;
            val = 0;
            uval = 0;
            isuns = 0;
            arg = _strtok(sptr, '%');
            if (arg) {
                cons->puts(sptr);
                arg++;
                if (*arg) {
                    switch (*arg) {
                        case 's':
                            cptr = va_arg(al, char *);
                            arg++;
                            if (cptr) {
                                cons->puts(cptr);
                            }

                            break;
                        case 'c':
                            isch = 1;
                            val = (char)va_arg(al, int);
                            arg++;
                            
                            break;
                        case 'h':
                            isdec = 1;
                            val = (short)va_arg(al, int);
                            arg++;
                            
                            break;
                        case 'd':
                            isdec = 1;
                            val = va_arg(al, int);
                            arg++;
                            
                            break;
                        case 'p':
                        case 'l':
                            isdec = 1;
                            if (*arg == 'p') {
                                ishex = 1;
                                isuns = 1;
                                uval = (unsigned long)va_arg(al, void *);
                            } else if (arg[1] == 'x' || arg[1] == 'u') {
                                if (arg[1] == 'x') {
                                    ishex = 1;
                                } else {
                                    isdec = 1;
                                }
                                isuns = 1;
                                uval = va_arg(al, unsigned long);
                                arg++;
                            } else {
                                val = va_arg(al, long);
                            }
                            arg++;
                            if (*arg) {
                                if (*arg == 'd') {
                                    arg++;
#if 0
                                } else if (*arg == 'x') {
                                    arg++;
                                    ishex = 1;
                                    isuns++;
#endif
                                }
                            }
                            
                            break;
                        case 'x':
//                            val = va_arg(al, int);
                            ishex = 1;
                            isuns = 1;
                            uval = va_arg(al, unsigned int);
                            arg++;
                            
                            break;
                        case 'u':
                            isuns = 1;
                            arg++;
                            if (*arg) {
                                switch (*arg) {
                                    case 'c':
                                        isch = 1;
                                        uval = (char)va_arg(al, unsigned int);
                                        
                                        break;
                                    case 'h':
                                        isdec = 1;
                                        uval = (short)va_arg(al, unsigned int);
                                        
                                        break;
                                    case 'd':
                                        isdec = 1;
                                        uval = va_arg(al, unsigned int);

                                        break;
                                    case 'l':
                                        isdec = 1;
                                        uval = va_arg(al, unsigned long);
                                        
                                        break;
                                    default:
                                        
                                        break;
                                }
                                arg++;
                            } else {
                                va_end(al);
                                
                                return;
                            }
                            
                            break;
                        default:
                            
                            break;
                    }
                    if (isuns) {
                        if (ishex) {
                            l = _ultoxn(uval, buf, LONGLONGBUFSIZE);
                            cons->puts(&buf[l]);
                        } else if (isdec) {
                            l = _ultodn(uval, buf, LONGLONGBUFSIZE);
                            cons->puts(&buf[l]);
                        } else if ((isch) && isprintascii(val)) {
                            cons->putchar((int)uval);
                        } else {
                            cons->putchar(' ');
                        }
                    } else {
                        if (ishex) {
                            l = _ltoxn(val, buf, LONGLONGBUFSIZE);
                            cons->puts(&buf[l]);
                        } else if (isdec) {
                            l = _ltodn(val, buf, LONGLONGBUFSIZE);
                            cons->puts(&buf[l]);
                        } else if ((isch) && isprintascii(val)) {
                            cons->putchar((int)val);
                        } else {
                            cons->putchar(' ');
                        }
                    }
                } else {
                    va_end(al);
                    
                    return;
                }
            } else {
                if (*sptr) {
                    cons->puts(sptr);
                }
                va_end(al);
                
                return;
            }
            sptr = arg;
        }
        va_end(al);
    }
    
    return;
}

#endif /* defined(__KERNEL__) && (__KERNEL__) */

/*
 * scan bitmap for first zero-bit past ofs
 * return -1 if not found, offset otherwise
 */
long
bfindzerol(long *bmap, long ofs, long nbit)
{
    long *ptr;
    long  cnt = ofs & (((uintptr_t)1 << (LONGSIZELOG2 + 3)) - 1);
    long  ndx = ofs >> (LONGSIZELOG2 + 3);
    long  val;
    long  ones = ~0L;
    long  bit = 1;

    ptr = bmap + ndx;
    nbit -= ofs;
    if (nbit > 0) {
        if (cnt) {
            val = *ptr;
            val >>= cnt;
            ptr++;
            if (val != ones) {
                while (val & bit) {
                    val >>= 1;
                    ofs++;
                }
                if (ofs < nbit) {
                    
                    return ofs;
                } else {
                    
                    return -1;
                }
            } else {
                ofs += CHAR_BIT * sizeof(long) - cnt;
            }
        }
        while (ofs < nbit) {
            val = *ptr;
            if (!val) {
                
                return ofs;
            } else if (val != ones) {
                while (val & bit) {
                    val >>= 1;
                    ofs++;
                }
                if (ofs < nbit) {
                    
                    return ofs;
                } else {
                    
                    return -1;
                }
            } else {
                ofs += CHAR_BIT * sizeof(long);
                ptr++;
            }
        }
    }
    
    return -1;
}

#if defined(__KERNEL__) && (__KERNEL__)

void
panic(unsigned long pid, int32_t trap, long err)
{
    const char *name;

    if (trap >= 0) {
        name = trapnametab[trap];
        if (name) {
            kprintf("PROC %lu CAUGHT TRAP %ld (%s): %lx\n",
                    pid, trap, name, err);
        } else {
            kprintf("PROC %lu CAUGHT RESERVED TRAP %ld (%s)\n",
                    pid, trap, name);
        }
    }
    k_halt();
}

#endif /* defined(__KERNEL__) && (__KERNEL__) */

