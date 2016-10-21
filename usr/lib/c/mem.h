#ifndef __MEM_H__
#define __MEM_H__

#if !defined(NULL)
#include <share/null.h>
#endif
#if !defined(__ptrdiff_t_defined)
#include <share/ptrdiff_t.h>
#endif

void * memccpy(void *__dest, const void *__src,
               int __c, size_t __n);
void * memchr(const void *__s, int __c, size_t __n);
int    memcmp(const void *__s1, const void *__s2,
              size_t __n);
void * memcpy(void *__dest, const void *__src,
              size_t __n);
int    memicmp(const void *__s1, const void *__s2,
               size_t __n);
void * memmove(void *__dest, const void *__src,
               size_t __n);
void * memset(void *__s, int __c, size_t __n);
void   movedata(unsigned __srcseg, unsigned __srcoff,
                unsigned __dstseg,unsigned __dstoff, size_t __n);
void   movmem(const void *__src, void *__dest,
              unsigned __length);
void   setmem(void *__dest,unsigned __length, char __value);

#if defined(__STDC__)
void * _fmemccpy(void *__dest, const void *__src,
                 int c, size_t __n);
void * _fmemchr(const void *__s, int c, size_t __n);
int    _fmemcmp(const void *__s1, const void *__s2,
                size_t __n);
void * _fmemcpy(void *__dest, const void *__src,
                size_t __n);
int    _fmemicmp(const void *__s1, const void *__s2,
                 size_t __n);
void * _fmemmove(void *__dest, const void *__src,
                 size_t __n);
void * _fmemset(void *__s, int c, size_t __n);
void   _fmovmem(const void *__src, void *__dest,
                unsigned __length);
void   _fsetmem(void *__dest,unsigned __length, 
                char __value);
#endif

/* Intrinsic functions */

void * __memchr__ (const void *__s, int __c, size_t __n);
int    __memcmp__(const void *__s1,
                  const void *__s2, size_t __n);
void * __memcpy__(void *__dest, const void *__src,
                  size_t __n);
void * __memset__(void *__s, int __c, size_t __n);

#endif /* __MEM_H__ */

