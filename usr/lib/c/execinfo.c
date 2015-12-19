#include <features.h>

#if defined(_GNU_SOURCE)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dlfcn.h>
#include <zero/asm.h>

uintptr_t
_backtrace(void **buf, int size, long syms, int fd)
{
    Dl_info        info;
    FILE          *fp = (fd >= 0) ? fdopen(fd, "a") : NULL;
    void         **pptr = buf;
    void          *ptr;
    void          *fptr = NULL;
    unsigned int   ndx;
    unsigned int   lim = size;
    int            ret = 0;

    if ((syms) && !fp) {
        buf = malloc(size * sizeof(void *));
        pptr = buf;
    }
    if (size) {
        if ((syms) && (fp)) {
            for (ndx = 0 ; ndx < lim ; ndx++) {
                ptr = fptr;
                m_getfrmadr2(__builtin_frame_address(0), &fptr);
//                pptr[ndx] = ptr;
                if (fptr) {
                    fprintf(fp, "%p: %p\n", ptr, fptr);
                } else {
                    ret = ndx;
                    
                    break;
                }
            }
        } else if (syms) {
            for (ndx = 0 ; ndx < lim ; ndx++) {
                ptr = fptr;
                m_getfrmadr2(__builtin_frame_address(0), &fptr);
                pptr[ndx] = ptr;
                if (fptr) {
                    printf("%p: %p\n", ptr, fptr);
                } else {
                    ret = ndx;
                    
                    break;
                }
            }
        } else {
            for (ndx = 0 ; ndx < lim ; ndx++) {
                ptr = fptr;
                m_getfrmadr2(__builtin_frame_address(0), &fptr);
                pptr[ndx] = ptr;
                if (fptr) {
                    printf("%p: %p\n", ptr, fptr);
                } else {
                    ret = ndx;
                    
                    break;
                }
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

