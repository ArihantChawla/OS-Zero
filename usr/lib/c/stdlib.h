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

void   abort(void);
void   exit(int status);
void * malloc(size_t size);
void * calloc(size_t n, size_t size);
void * realloc(void *ptr, size_t size);
void   free(void *ptr);
#if (_ISOC11_SOURCE)
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
//void * memalign(size_t align, size_t size)

#if (_BSD_SOURCE)
void * reallocf(void *ptr, size_t size);
#endif /* BSD */

void * bsearch(const void *key, const void *base,
               size_t nitems, size_t itemsize,
               int (*cmp)(const void *, const void *));

void   qsort(void *const base, size_t nitems, size_t size,
             int (*cmp)(const void *, const void *));
			 
#if (_ZERO_SOURCE)
/* FIXME: should this be UINT32_MAX? */
#define RAND_MAX INT32_MAX
#endif

/* FIXME: should this be dependent on the locale? */
//#define MB_CUR_MAX 4

void                 _Exit(int status);
long                 a64l(const char *str);
void                 abort(void);
int                  abs(int val);
int                  atexit(void (*func)(void));
double               atof(const char *str);
int                  atoi(const char *str);
long                 atol(const char *str);
long long            atoll(const char *str);
div_t                div(int numer, int denom);
double               drand48(void);
char               * ecvt(double num, int ndigit, int *decptr, int *sign);
double               erand48(unsigned short tab[3]);
void                 exit(int status);
char               * fcvt(double num, int ndigit, int *decptr, int *sign);
/* FIXME: on linux, ndigit is size_t...? :) */
char               * gcvt(double num, int ndigit, char *buf);
char               * getenv(const char *name);
int                  getsubopt(char *opt, char *const *tok, char **val);
int                  grantpt(int fd);
char               * initstate(unsigned seed, char *state, size_t n);
long                 jrand48(unsigned short tab[3]);
char               * l64a(long val); 
long                 labs(long val);
void                 lcong48(unsigned short tab[7]);
ldiv_t               ldiv(long numer, long denom);
long long            llabs(long long val);
lldiv_t              lldiv(long long numer, long long denom);
long                 lrand48(void);
int                  mblen(const char *str, size_t n);
size_t               mbstowcs(wchar_t *dest, const char *src, size_t n);
int                  mbtowc(wchar_t *dest, const char *str, size_t n);
char               * mktemp(char *template);
int                  mkstemp(char *template);
int                  mkostemp(char *template, int flg);
int                  mkstemps(char *template, int suflen);
int                  mkostemps(char *template, int suflen, int flg);
long                 mrand48(void);
long                 nrand48(unsigned short tab[3]);
int                  posix_openpt(int flg);
char                 ptsname(int fd);
int                  putenv(char *str);
int                  rand(void);
int                  rand_r(unsigned *seedptr);
void                 srand(unsigned seed);
long                 random(void);
char               * realpath(const char *path, char *buf);
unsigned short       seed48(unsigned short tab[3]);
int                  setenv(const char *name, const char *value, int replace);
void                 setkey(const char *key);
char               * setstate(char *state);
void                 srand(unsigned seed);
void                 srand48(long seed);
void                 srandom(unsigned seed);
double               strtod(const char *nptr, char **endptr);
float                strtof(const char *nptr, char **endptr);
long double          strtold(const char *nptr, char **endptr);
long                 strtol(const char *nptr, char **endptr, int base);
long long            strtoll(const char *nptr, char **endptr, int base);
unsigned long        strtoul(const char *nptr, char **endptr, int base);
unsigned long long   strtoull(const char *nptr, char **endptr, int base);
int                  system(const char *cmd);
int                  unlockpt(int fd);
int                  unsetenv(const char *name);
size_t               wcstombs(char *dest, const wchar_t *src, size_t n);
int                  wctomb(char *src, wchar_t wc);
#if (_ISO_C11_SOURCE)
/* TODO: quick_exit(), at_quick_exit() */
#endif

#endif /* __STDLIB_H__ */

