#include <stdint.h>
//#include <stddef.h>
#include <limits.h>
#include <stdarg.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <kern/io/drv/chr/cons.h>
#include <kern/io/drv/pc/vga.h>
#include <kern/unit/x86/asm.h>

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

/* assumes longword-aligned blocks with sizes of long-word multiples */
void
kbzero(void *adr, unsigned long len)
{
    long *next;
    long *ptr = adr;
    char *cptr;
    long  val = 0;
    long  incr = 8;
    long  nleft = len;

    if (len > (1UL << (LONGSIZELOG2 + 3))) {
        /* zero non-cacheline-aligned head long-word by long-word */
        nleft = ((uintptr_t)adr) & ((1UL << LONGSIZELOG2) - 1);
        if (nleft) {
            nleft = (1UL << (LONGSIZELOG2 + 3)) - nleft;
            nleft >>= LONGSIZELOG2;
            len -= nleft;
            while (nleft--) {
                *ptr++ = val;
            }
        }
        nleft = len & ~((1UL << LONGSIZELOG2) - 1);
    }
    next = ptr;
    if (len >= (1UL << (LONGSIZELOG2 + 3))) {
        nleft = len & ((1UL << (LONGSIZELOG2 + 3)) - 1);
        len >>= LONGSIZELOG2 + 3;
        /* zero aligned cachelines */
        while (len--) {
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
    if (nleft > 0) {
        len = nleft >> LONGSIZELOG2;
        nleft -= len << LONGSIZELOG2;
        while (len--) {
            /* zero tail long-words */
            *ptr++ = val;
        }
        cptr = (char *)ptr;
        while (nleft--) {
            *cptr++ = 0;
        }
    }

    return;
}

/* assumes longword-aligned blocks with sizes of long-word multiples */
void
kmemset(void *adr, int byte, unsigned long len)
{
    long *next;
    long *ptr = adr;
    char *cptr;
    long  val = 0;
    long  incr = 8;
    long  nleft = len;
    
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
    if (nleft > 0) {
        len = nleft >> LONGSIZELOG2;
        nleft -= len << LONGSIZELOG2;
        while (len--) {
            /* zero tail long-words */
            *ptr++ = val;
        }
        cptr = (char *)ptr;
        while (nleft--) {
            *cptr++ = 0;
        }
    }
    
    return;
}

void
kmemcpy(void *dest, const void *src, unsigned long len)
{
    unsigned long  nleft = len;
    long          *dptr = dest;
    const long    *sptr = src;
    char          *dcptr;
    long          *dnext;
    const char    *scptr;
    const long    *snext;
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
    if (nleft > 0) {
        len = nleft >> LONGSIZELOG2;
        nleft -= len << LONGSIZELOG2;
        while (len--) {
            *dptr++ = *sptr++;
        }
        dcptr = (char *)dptr;
        scptr = (char *)sptr;
        while (nleft--) {
            *dcptr++ = *scptr++;
        }
    }

    return;
}

void
kbfill(void *adr, uint8_t byte, unsigned long len)
{
    unsigned long  nleft = len;
    long          *ptr = NULL;
    char          *cptr;
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
    if (nleft > 0) {
        len = nleft >> LONGSIZELOG2;
        nleft -= len << LONGSIZELOG2;
        while (len--) {
            *ptr++ = val;
        }
        cptr = (char *)ptr;
        while (nleft--) {
            *cptr++ = byte;
        }
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

unsigned long
kstrncpy(char *dest, char *src, unsigned long len)
{
    unsigned long nb = 0;

    while ((*src) && len--) {
        *dest++ = *src++;
        nb++;
    }
    *dest = '\0';

    return nb;
}

static unsigned long
_ltoxn(long val, char *buf, unsigned long len)
{
    uint8_t       u8;
    unsigned long l;
    unsigned long m;
    unsigned long n;
    long          incr = 4;
    long          sign = 0;

    if (val < 0) {
        sign = 1;
    }
    buf[len - 1] = '\0';
    m = len - 2;
    buf[m] = 0;
    n = 0;
    for (l = 0 ; l < 8 * LONGSIZE ; l += incr) {
        u8 = (val >> l) & 0xf;
        buf[m] = _ltoxtab[u8];
        if (++n == len) {

            break;
        }
        m--;
    }
    m++;
    m = min(m, len - 2);
    while (buf[m] == '0' && m < len - 2) {
        m++;
    }
    if (sign) {
        m--;
        buf[m] = '-';
    }

    return m;
}

static unsigned long
_ltodn(long val, char *buf, unsigned long len)
{
    uint8_t       u8;
    unsigned long l;
    unsigned long n;
    long          sign = 0;

    if (val < 0) {
        sign = 1;
    }
    buf[len - 1] = '\0';
    l = len - 2;
    n = 0;
    do {
        u8 = val % 10;
        val /= 10;
        buf[l] = _ltoxtab[u8];
        if (++n == len) {
            
            break;
        }
        l--;
    } while (val);
    l++;
    l = min(l, len - 2);
    while (buf[l] == '0' && l < len - 2) {
        l++;
    }
    if (sign) {
        l--;
        buf[l] = '-';
    }

    return l;
}

static unsigned long
_ultoxn(unsigned long uval, char *buf, unsigned long len)
{
    uint8_t       u8;
    unsigned long l;
    unsigned long m;
    unsigned long n;
    long          incr = 4;

    buf[len - 1] = '\0';
    m = len - 2;
    buf[m] = 0;
    n = 0;
    for (l = 0 ; l < 8 * LONGSIZE ; l += incr) {
        u8 = (uval >> l) & 0xf;
        buf[m] = _ltoxtab[u8];
        if (++n == len) {

            break;
        }
        m--;
    }
    m++;
    m = min(m, len - 2);
    while (buf[m] == '0' && m < len - 2) {
        m++;
    }

    return m;
}

static unsigned long
_ultodn(unsigned long uval, char *buf, unsigned long len)
{
    uint8_t       u8;
    unsigned long l;
    unsigned long n;

    buf[len - 1] = '\0';
    l = len - 2;
    n = 0;
    do {
        u8 = uval % 10;
        uval /= 10;
        buf[l] = _ltoxtab[u8];
        if (++n == len) {
            
            break;
        }
        l--;
    } while (uval);
    l++;
    l = min(l, len - 2);
    while (buf[l] == '0' && l < len - 2) {
        l++;
    }

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

/*
 * %x, %c, %h, %d, %ld, %uc, %uh, %ud, %ul, %lx, %x, %p
 */
void
kprintf(char *fmt, ...)
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
                            cptr = (char *)va_arg(al, char *);
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
                                ishex = 1;
                                isuns = 1;
                                uval = (unsigned long)va_arg(al, unsigned long);
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
                            uval = (unsigned int)va_arg(al, unsigned int);
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

#if 0
/*
 * scan bitmap for first zero-bit past ofs
 * return -1 if not found, offset otherwise
 */
long
kbfindzerol(long *bmap, long ofs, long nbit)
{
    long *ptr;
    long  cnt = ofs & ((1UL << (LONGSIZELOG2 + 3)) - 1);
    long  ndx = ofs >> (LONGSIZELOG2 + 3);
    long  val;
    long  ones = ~0L;
    long  bit = 1;

    ptr = bmap + (ofs / CHAR_BIT / sizeof(long));
    nbit -= ofs;
    if (nbit > 0) {
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
                ofs += sizeof(long) * CHAR_BIT;
                ptr++;
            }
        }
    }
    
    return -1;
}
#endif

void
panic(long trap)
{
    if (trap >= 0) {
        kprintf("KERNEL CAUGHT TRAP %ld\n", trap);
    }
    k_halt();
}

