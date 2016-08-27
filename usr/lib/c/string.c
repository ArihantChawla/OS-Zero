#include <features.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#if defined(_GNU_SOURCE)
#include <alloca.h>
#endif
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/trix.h>

#if defined(__GLIBC__)

#undef strdupa
#undef strndupa
#undef mempcpy
#undef __mempcpy

#else

extern const uint8_t stringcolltab_c_en_US[256];

static THREADLOCAL struct _string  _string;
static const  uint8_t             *colltab = stringcolltab_c_en_US;
static const  uint8_t             *localecolltab = stringcolltab_c_en_US;
const unsigned char               *collnametab[STRINGNLANG]
= {
    (unsigned char *)"en_US"
};

#endif /* __GLIBC__ */

/* TESTED OK */
void *
memcpy(void *dest,
       const void *src,
       size_t n)
{
    unsigned long *ulptr1;
    unsigned long *ulptr2;
    uint8_t       *u8ptr1;
    uint8_t       *u8ptr2;
    size_t         cnt;
    size_t         nleft;
    size_t         val;

    if (!n) {

        return dest;
    }

    nleft = n;
    u8ptr1 = (void *)src;
    u8ptr2 = dest;
    val = sizeof(unsigned long);
    if (nleft > (val << 3)
        && (((uintptr_t)u8ptr1 & (val - 1))
            == ((uintptr_t)u8ptr2 & (val - 1)))) {
        cnt = (uintptr_t)u8ptr1 & (val - 1);
        if (cnt) {
            cnt = val - cnt;
            nleft -= cnt;
            while (cnt--) {
                *u8ptr2++ = *u8ptr1++;
            }
        }
        val = LONGSIZELOG2 + 3;
        ulptr2 = (unsigned long *)u8ptr2;
        cnt = nleft >> val;
        ulptr1 = (unsigned long *)u8ptr1;
        nleft -= cnt << val;
        val = 8;
        while (cnt--) {
            ulptr2[0] = ulptr1[0];
            ulptr2[1] = ulptr1[1];
            ulptr2[2] = ulptr1[2];
            ulptr2[3] = ulptr1[3];
            ulptr2[4] = ulptr1[4];
            ulptr2[5] = ulptr1[5];
            ulptr2[6] = ulptr1[6];
            ulptr2[7] = ulptr1[7];
            ulptr2 += val;
            ulptr1 += val;
        }
        u8ptr2 = (uint8_t *)ulptr2;
        u8ptr1 = (uint8_t *)ulptr1;
    }
    while (nleft--) {
        *u8ptr2++ = *u8ptr1++;
    }

    return dest;
}

/* TESTED OK */
static void *
_memcpybk(void *dest,
          void *src,
          size_t n)
{
    unsigned long *ulptr1;
    unsigned long *ulptr2;
    uint8_t       *u8ptr1;
    uint8_t       *u8ptr2;
    size_t         cnt;
    size_t         nleft;
    size_t         val;

    if (!n) {

        return dest;
    }

    nleft = n;
    u8ptr1 = src;
    u8ptr2 = dest;
    u8ptr1 += n;
    u8ptr2 += n;
    val = sizeof(unsigned long);
    if (labs(u8ptr1 - u8ptr2) >= (long)(val << 3)
        && nleft >= (val << 3)
        && (((uintptr_t)u8ptr1 & (val - 1))
            == ((uintptr_t)u8ptr2 & (val - 1)))) {
        cnt = (uintptr_t)u8ptr1 & (val - 1);
        if (cnt) {
//            cnt = val - cnt;
            nleft -= cnt;
            while (cnt--) {
                *--u8ptr2 = *--u8ptr1;
            }
        }
        val = LONGSIZELOG2 + 3;
        ulptr2 = (unsigned long *)u8ptr2;
        cnt = nleft >> val;
        ulptr1 = (unsigned long *)u8ptr1;
        nleft -= cnt << val;
        val = 8;
        while (cnt--) {
            ulptr2 -= val;
            ulptr1 -= val;
            ulptr2[0] = ulptr1[0];
            ulptr2[1] = ulptr1[1];
            ulptr2[2] = ulptr1[2];
            ulptr2[3] = ulptr1[3];
            ulptr2[4] = ulptr1[4];
            ulptr2[5] = ulptr1[5];
            ulptr2[6] = ulptr1[6];
            ulptr2[7] = ulptr1[7];
        }
        u8ptr2 = (uint8_t *)ulptr2;
        u8ptr1 = (uint8_t *)ulptr1;
    }
    while (nleft--) {
        *--u8ptr2 = *--u8ptr1;
    }

    return dest;
}

/* TESTED OK */
void *
memmove(void *dest,
        const void *src,
        size_t n)
{
    if (!n) {

        return dest;
    }

    if ((uintptr_t)dest < (uintptr_t)src
        || (uintptr_t)src + n < (uintptr_t)dest) {
        memcpy(dest, src, n);
    } else {
        _memcpybk(dest, src, n);
    }

    return dest;
}

/* TESTED OK */
char *
strcpy(char *dest,
       const char *src)
{
    char *cptr = dest;

    while (*src) {
        *cptr++ = *src++;
    }
    *cptr = *src;

    return dest;
}

/* TESTED OK */
char *
strncpy(char *dest,
        const char *src,
        size_t n)
{
    char *cptr = dest;

    if (n) {
        while ((*src) && (n--)) {
            *cptr++ = *src++;
        }
        if (n) {
            *cptr = *src;
        }
    }

    return dest;
}

/* TESTED OK */
char *
strcat(char *dest,
       const char *src)
{
    char *cptr = dest;

    while (*cptr) {
        cptr++;
    }
    while (*src) {
        *cptr++ = *src++;
    }
    *cptr = *src;

    return dest;
}

/* TESTED OK */
char *
strncat(char *dest,
        const char *src,
        size_t n)
{
    char *cptr = dest;

    if (n) {
        while (*cptr) {
            cptr++;
        }
        while ((*src) && (n--)) {
            *cptr++ = *src++;
        }
        if (n) {
            *cptr = *src;
        }
    }

    return dest;
}

/* TESTED OK */
int
memcmp(const void *ptr1,
       const void *ptr2,
       size_t n)
{
    unsigned char *ucptr1 = (unsigned char *)ptr1;
    unsigned char *ucptr2 = (unsigned char *)ptr2;
    int            retval = 0;

    if (n) {
        while ((*ucptr1 == *ucptr2) && (n--)) {
            ucptr1++;
            ucptr2++;
        }
        if (n) {
            retval = (int)*ucptr1 - (int)*ucptr2;
        }
    }

    return retval;
}

#if !defined(__GLIBC__)

/* TESTED OK */
int
strcmp(const char *str1,
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

/* TESTED OK */
int
strncmp(const char *str1,
        const char *str2,
        size_t n)
{
    unsigned char *ucptr1 = (unsigned char *)str1;
    unsigned char *ucptr2 = (unsigned char *)str2;
    int            retval = 0;

    if (n) {
        while ((*ucptr1) && (*ucptr1 == *ucptr2) && (n--)) {
            ucptr1++;
            ucptr2++;
        }
        if (n) {
            retval = (int)*ucptr1 - (int)*ucptr2;
        }
    }

    return retval;
}

#endif /* !__GLIBC__ */

int
strcoll(const char *str1,
        const char *str2)
{
    exit(1);
}

size_t
strxfrm(char *RESTRICT str1,
        const char *RESTRICT str2,
        size_t n)
{
//    fprintf(stderr, "TODO: strxfrm() not implemented\n");

    exit(1);
}

/* TESTED OK */
void *
memchr(const void *ptr,
       int ch,
       size_t n)
{
    unsigned char *ucptr = (unsigned char *)ptr;
    unsigned char  uc = (unsigned char)ch;
    void          *retval = NULL;

    if (n) {
        while ((*ucptr != uc) && (n--)) {
            ucptr++;
        }
        if (*ucptr == uc && (n)) {
            retval = ucptr;
        }
    }

    return retval;
}

#if !defined(__GLIBC__)

/* TESTED OK */
void *
strchr(const char *str,
       int ch)
{
    char *cptr = (char *)str;
    char  c = (char)ch;
    void *retval = NULL;
    
    while ((*cptr) && *cptr != c) {
        cptr++;
    }
    if (*cptr == c) {
        retval = cptr;
    }

    return retval;
}

#endif /* !__GLIBC__ */

/* TESTED OK */
size_t
strcspn(const char *str1,
        const char *str2)
{
    unsigned char *ucptr1 = (unsigned char *)str1;
    unsigned char *ucptr2 = (unsigned char *)str2;
    size_t         len = 0;

    while (*ucptr1) {
        while ((*ucptr2) && *ucptr1 != *ucptr2) {
            ucptr2++;
        }
        if (*ucptr2) {
            
            break;
        }
        ucptr1++;
        ucptr2 = (unsigned char *)str2;
        len++;
    }

    return len;
}

char *
strpbrk(const char *str1,
        const char *str2)
{
    unsigned char *ucptr1 = (unsigned char *)str1;
    unsigned char *ucptr2;
    char          *retptr = NULL;

    if (*str1) {
        while (!retptr && (*ucptr1)) {
            ucptr2 = (unsigned char *)str2;
            while (!retptr && (*ucptr2)) {
                if (*ucptr1 == *ucptr2) {
                    retptr = (char *)ucptr1;
                }
                ucptr2++;
            }
            ucptr1++;
        }
    }
    
    return retptr;
}

/* TESTED OK */
char *
strrchr(const char *str,
        int ch)
{
    char *cptr = (char *)str;
    char  c = (char)ch;
    
    while (*cptr) {
        cptr++;
    }
    while (cptr >= str && *cptr != c) {
            cptr--;
    }
    if (cptr < str) {
        cptr = NULL;
    }

    return cptr;
}

/* TESTED OK */
size_t
strspn(const char *str1,
       const char *str2)
{
    unsigned char *ucptr1 = (unsigned char *)str1;
    unsigned char *ucptr2;
    size_t         len = 0;

    while (*ucptr1) {
        ucptr2 = (unsigned char *)str2;
        while ((*ucptr2) && *ucptr1 != *ucptr2) {
            ucptr2++;
        }
        if (!*ucptr2) {
            
            break;
        }
        ucptr1++;
        len++;
    }

    return len;
}

char *
strstr(const char *str1,
       const char *str2)
{
    char *cptr1 = (char *)str1;
    char *cptr2;
    char *cptr3;
    char *retptr = NULL;
    
    while (!retptr && (*cptr1)) {
        cptr2 = cptr1;
        cptr3 = (char *)str2;
        while (*cptr2 == *cptr3) {
            cptr2++;
            cptr3++;
        }
        if (!*cptr2) {
            retptr = cptr1;
        }
        cptr1++;
    }

    return retptr;
}

#if !defined(__GLIBC__)

char *
strtok(char *str1,
       const char *str2)
{
    char   *cptr = _string.curtok;
    char   *retptr = NULL;
    size_t  nspn;
    
    if ((str1) && (*str1)) {
        nspn = strspn(str1, str2);
        str1 += nspn;
        cptr = str1;
    }
    if (cptr) {
        retptr = cptr;
        nspn = strcspn(cptr, str2);
        cptr += nspn;
        *cptr = '\0';
        _string.curtok = ++cptr;
        if (!*cptr) {
            _string.curtok = NULL;
        }
        if (!retptr) {
            retptr = cptr;
            nspn = strcspn(cptr, str2);
            cptr += nspn;
            if (*cptr) {
                *cptr = '\0';
            } else {
                _string.curtok = NULL;
            }
        }
    }
    
    return retptr;
}

#endif /* !__GLIBC__ */

#if defined(MEMSETDUALBANK) && (MEMSETDUALBANK)

void *
memset(void *ptr,
       int ch,
       size_t nb)
{
    unsigned long *ulptr1;
    unsigned long *ulptr2;
    uint8_t       *u8ptr;
    unsigned long  ul;
    long           val;
    size_t         nleft;
    size_t         n;
    size_t         cnt;
    size_t         tmp;
    uint8_t        u8;

    if (!nb) {

        return ptr;
    }
    nleft = nb;
    u8ptr = ptr;
    tmp = sizeof(unsigned long) - 1;
    val = sizeof(unsigned long);
    u8 = (uint8_t)ch;
    if (nleft > (val << 3)) {
        n = (uintptr_t)u8ptr & tmp;
        if (n) {
            n = val - n;
            nleft -= n;
            while (n--) {
                *u8ptr++ = u8;
            }
        }
        ul = ch;
        tmp = CLSIZE - 1;
        ul |= (ch << 8);
        ulptr1 = (unsigned long *)u8ptr;
        ul |= (ul << 16);
        val = CLSIZE;
#if (LONGSIZE == 8)
        ul |= (ul << 32);
#endif
        n = (uintptr_t)ulptr1 & tmp;
        if (n) {
            n = val - n;
            n >>= LONGSIZELOG2;
            n = min(n, nleft >> LONGSIZELOG2);
            if (n) {
                nleft -= n << LONGSIZELOG2;;
                while (n--) {
                    *ulptr1++ = ul;
                }
            }
        }
        val = 8;
        n = nleft >> PAGESIZELOG2;
        nleft -= n << val;
        while (n--) {
            cnt = PAGESIZE >> (LONGSIZELOG2 + 1);
            ulptr2 = ulptr1 + cnt;
            while (cnt--) {
                ulptr1[0] = ul;
                ulptr2[0] = ul;
                ulptr1[1] = ul;
                ulptr2[1] = ul;
                ulptr1[2] = ul;
                ulptr2[2] = ul;
                ulptr1[3] = ul;
                ulptr2[3] = ul;
                ulptr1[4] = ul;
                ulptr2[4] = ul;
                ulptr1[5] = ul;
                ulptr2[5] = ul;
                ulptr1[6] = ul;
                ulptr2[6] = ul;
                ulptr1[7] = ul;
                ulptr2[7] = ul;
                ulptr1 += val;
                ulptr2 += val;
                ulptr1 = ulptr2;
            }
        }
        val = LONGSIZELOG2;
        n = nleft >> val;
        nleft -= n << val;
        while (cnt--) {
            *ulptr1++ = ul;
        }
        u8ptr = (uint8_t *)ulptr1;
    }
    while (nleft--) {
        *u8ptr++ = u8;
    }
    
    return ptr;
}

#else /* !MEMSETDUALBANK */

/* TESTED OK */
void *
memset(void *ptr,
       int ch,
       size_t n)
{
    unsigned long *ulptr;
    uint8_t       *u8ptr;
    unsigned long  ul;
    size_t         cnt;
    size_t         nleft;
    size_t         val;
    uint8_t        u8;

    if (!n) {

        return ptr;
    }
    nleft = n;
    u8ptr = ptr;
    val = sizeof(unsigned long);
    u8 = (uint8_t)ch;
    if (nleft > (val << 3)) {
        cnt = (uintptr_t)u8ptr & (val - 1);
        if (cnt) {
            cnt = val - cnt;
            nleft -= cnt;
            while (cnt--) {
                *u8ptr++ = u8;
            }
        }
        u8 = (uint8_t)ch;
        ul = u8;
        ulptr = (unsigned long *)u8ptr;
        ul |= (ul << 8);
        val = LONGSIZELOG2 + 3;
        ul |= (ul << 16);
        cnt = nleft >> val;
#if (LONGSIZE == 8)
        ul |= (ul << 32);
#endif
        if (cnt) {
            nleft -= cnt << val;
            val = 8;
            while (cnt--) {
                ulptr[0] = ul;
                ulptr[1] = ul;
                ulptr[2] = ul;
                ulptr[3] = ul;
                ulptr[4] = ul;
                ulptr[5] = ul;
                ulptr[6] = ul;
                ulptr[7] = ul;
                ulptr += val;
            }
        }
        u8ptr = (uint8_t *)ulptr;
    }
    while (nleft--) {
        *u8ptr++ = u8;
    }
    
    return ptr;
}

#endif /* MEMSETDUALBANK */

char *
strerror(int errnum)
{
//    fprintf(stderr, "TODO: strerror() not implemented\n");
    
    return NULL;
}

size_t
strlen(const char *str)
{
    size_t len = 0;
    
    while (*str++) {
        len++;
    }
    
    return len;
}

#if ((defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 700))                 \
     || (defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200809L)))

size_t
strnlen(const char *str, size_t maxlen)
{
    size_t len = 0;

    while (str[0] && (maxlen--)) {
        len++;
    }

    return len;
}

#endif

#if (defined(_SVID_SOURCE) || defined(_BSD_SOURCE)                      \
     || (defined(_XOPEN_SOURCE)                                         \
         && (_XOPEN_SOURCE >= 500 || defined(_XOPEN_SOURCE_EXTENDED))))
char *
strdup(const char *str)
{
    size_t  len = strlen(str);
    char   *buf = (len) ? malloc(len + 1) : NULL;

    if (buf) {
        memcpy(buf, str, len);
        buf[len] = '\0';
    } else {
#if defined(ENOMEM)
        errno = ENOMEM;
#endif
    }

    return buf;
}
#endif

#if !defined(__GLIBC__)

#if ((defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L)           \
     || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 700))
char *
strndup(const char *str, size_t maxlen)
{
    size_t  len = strlen(str);
    size_t  sz = min(len, maxlen);
    char   *buf = (sz) ? malloc(sz + 1) : NULL;

    if (buf) {
        memcpy(buf, str, sz);
        buf[sz] = '\0';
    } else {
#if defined(ENOMEM)
        errno = ENOMEM;
#endif
    }

    return buf;
}
#endif

#endif /* !__GLIBC__ */

#if defined(_GNU_SOURCE)

char *
strdupa(const char *str)
{
    size_t  len = strlen(str);
    char   *buf = (len) ? alloca(len + 1) : NULL;

    if (buf) {
        memcpy(buf, str, len);
        buf[len] = '\0';
    } else {
#if defined(ENOMEM)
        errno = ENOMEM;
#endif
    }

    return buf;
}

char *
strndupa(const char *str, size_t maxlen)
{
    size_t  len = strlen(str);
    size_t  sz = min(len, maxlen);
    char   *buf = (sz) ? alloca(sz + 1) : NULL;

    if (buf) {
        memcpy(buf, str, sz);
        buf[sz] = '\0';
    } else {
#if defined(ENOMEM)
        errno = ENOMEM;
#endif
    }

    return buf;
}

#endif

