#ifndef __STDLIB_H__
#define __STDLIB_H__

/* FIXME: use feature test macros more */

#include <features.h>
#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <sys/wait.h>
#include <zero/trix.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

typedef struct { int quot; int rem; }             div_t;
typedef struct { long quot; long rem; }           ldiv_t;
typedef struct { long long quot; long long rem; } lldiv_t;

/* pseudo-random numbers */
#if (_ZERO_SOURCE)
/* FIXME: should this be UINT32_MAX? use a PRNG from libzero? :) */
#define RAND_MAX INT32_MAX
#endif

#if !defined(__KERNEL__)

/* process termination */
void   abort(void);
void   exit(int status);
int    atexit(void (*func)(void));
/* ISO C99 */
void   _Exit(int status);
#if defined(_ISO_C11_SOURCE)
/* TODO: quick_exit(), at_quick_exit() */
#endif

/* dynamic memory */
void * malloc(size_t size);
void * calloc(size_t n, size_t size);
void * realloc(void *ptr, size_t size);
void   free(void *ptr);
#if defined(_ISOC11_SOURCE)
void * aligned_alloc(size_t align, size_t size);
#endif
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
int    posix_memalign(void **ret, size_t align, size_t size);
#endif /* POSIX || XOPEN */
#if ((_BSD_SOURCE)                                                      \
    || (_XOPEN_SOURCE >= 500 || ((_XOPEN_SOURCE) && (_XOPEN_SOURCE_EXTENDED))) \
    && !(_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600))
void * valloc(size_t size);
#endif /* BSD || XOPEN || POSIX */
#if (_BSD_SOURCE)
void * reallocf(void *ptr, size_t size);
#endif /* BSD */

/* sorting and searching */
void * bsearch(const void *key, const void *base,
               size_t nitems, size_t itemsize,
               int (*cmp)(const void *, const void *));
void   qsort(void *const base, size_t nitems, size_t size,
             int (*cmp)(const void *, const void *));

int                  rand(void);
int                  rand_r(unsigned *seedptr);
void                 srand(unsigned seed);
double               drand48(void);
double               erand48(unsigned short tab[3]);
long                 jrand48(unsigned short tab[3]);
long                 lrand48(void);
long                 mrand48(void);
long                 nrand48(unsigned short tab[3]);
char               * initstate(unsigned seed, char *state, size_t n);
char               * setstate(char *state);
long                 random(void);
void                 srandom(unsigned seed);
unsigned short       seed48(unsigned short tab[3]);
void                 srand48(long seed);

/* FIXME: should this be dependent on the locale? */
//#define MB_CUR_MAX 4

/* miscellaneous/uncategorized routines */
long                 a64l(const char *str);
int                  abs(int val);
div_t                div(int numer, int denom);
char               * ecvt(double num, int ndigit, int *decptr, int *sign);
char               * fcvt(double num, int ndigit, int *decptr, int *sign);
/* FIXME: on linux, ndigit is size_t...? :) */
char               * gcvt(double num, int ndigit, char *buf);
char               * getenv(const char *name);
int                  getsubopt(char *opt, char *const *tok, char **val);
char               * l64a(long val); 
long                 labs(long val);
void                 lcong48(unsigned short tab[7]);
ldiv_t               ldiv(long numer, long denom);
long long            llabs(long long val);
lldiv_t              lldiv(long long numer, long long denom);

/* multibyte and wide character routines */
//int                  mbsinit(const mbstate_t *state);
int                  mblen(const char *str, size_t n);
size_t               mbstowcs(wchar_t *dest, const char *src, size_t n);
int                  mbtowc(wchar_t *dest, const char *str, size_t n);
size_t               wcstombs(char *dest, const wchar_t *src, size_t n);
int                  wctomb(char *src, wchar_t wc);

/* temporary files */
char               * mktemp(char *template);
int                  mkstemp(char *template);
int                  mkostemp(char *template, int flg);
int                  mkstemps(char *template, int suflen);
int                  mkostemps(char *template, int suflen, int flg);

/* pseudo-terminal interface */
int                  posix_openpt(int flg);
char                 ptsname(int fd);
int                  grantpt(int fd);
int                  unlockpt(int fd);

/* environment */
int                  putenv(char *str);
int                  setenv(const char *name, const char *value, int replace);
int                  unsetenv(const char *name);

/* canonical absolute pathname */
char               * realpath(const char *path, char *buf);

/* cryptography */
void                 setkey(const char *key);
void                 encrypt(char blk[64], int edflg);

/* textual to numerical conversions */
/* old interface */
double               atof(const char *str);
int                  atoi(const char *str);
long                 atol(const char *str);
long long            atoll(const char *str);
/* integrals */
long                 strtol(const char *nptr, char **endptr, int base);
long long            strtoll(const char *nptr, char **endptr, int base);
unsigned long        strtoul(const char *nptr, char **endptr, int base);
unsigned long long   strtoull(const char *nptr, char **endptr, int base);
/* floating-point */
double               strtod(const char *nptr, char **endptr);
float                strtof(const char *nptr, char **endptr);
long double          strtold(const char *nptr, char **endptr);

int                  system(const char *cmd);

#endif /* !defined(__KERNEL__) */

#endif /* __STDLIB_H__ */

