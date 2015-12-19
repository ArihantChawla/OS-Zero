#include <features.h>

#if defined(_GNU_SOURCE)

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dlfcn.h>
#include <zero/asm.h>

#define _BTPRINTFMT  "%a <%n%D> at %f\n"
#define _BTOFSSTRLEN 32
#define _BTSTRLEN    128

uintptr_t
_btprint(void **buf, int size, const char *fmt, int fd)
{
    unsigned int   lim = size;
    char         **tab;
    unsigned int   ndx;

    if (!fmt) {
        fmt = _BTPRINTFMT;
    }
    for (ndx = 0 ; ndx < size ; ndx++) {
    }

    return (uintptr_t)tab;
}

uintptr_t
_backtrace(void **buf, int size, long syms, int fd)
{
    Dl_info        info;
    FILE          *fp = (fd >= 0) ? fdopen(fd, "a") : NULL;
    void          *ptr = NULL;
    void          *oldptr;
    void          *fptr = NULL;
    unsigned int   lim = size - 1;
    int            ret = 0;
    void          *adr;
    void         **pptr;
    uintptr_t      delta;
    unsigned int   ndx;

    if ((syms) && !fp) {
        buf = malloc(size * sizeof(void *));
    }
    pptr = buf;
    if (size) {
        m_getretfrmadr(&fptr);
        m_loadretadr(fptr, &ptr);
        if (syms) {
            for (ndx = 0 ; ndx < lim ; ndx++) {
                if ((ptr) && ptr != oldptr) {
                    if (!fp) {
                        pptr[ndx] = ptr;
                    } else {
                        dladdr(ptr, &info);
                        adr = info.dli_saddr;
                        if (ptr != info.dli_saddr) {
                            delta = (uintptr_t)ptr - (uintptr_t)adr;
                            fprintf(fp, "0x%p <%s+0x%llx> at %s\n",
                                    ptr, info.dli_sname,
                                    (unsigned long long)delta, info.dli_fname);
                        } else {
                            fprintf(fp, "0x%p <%s> at %s\n",
                                    ptr, info.dli_sname, info.dli_fname);
                        }
                        fflush(fp);
                    }
                } else {
                    ret = ndx;
                    
                    break;
                }
                oldptr = ptr;
                m_getretfrmadr(&fptr);
                m_loadretadr(fptr, &ptr);
            }
        } else {
            for (ndx = 0 ; ndx < lim ; ndx++) {
                if ((ptr) && ptr != oldptr) {
                    pptr[ndx] = ptr;
                } else {
                    ret = ndx;
                    
                    break;
                }
                oldptr = ptr;
                m_getretfrmadr(&fptr);
                m_loadretadr(fptr, &ptr);
            }
        }
    }
    if (fp) {
        fclose(fp);

        return 0;
    } else if (syms) {

        return (uintptr_t)buf;
    }

    return (uintptr_t)ret;
}

#endif /* defined(_GNU_SOURCE) */

