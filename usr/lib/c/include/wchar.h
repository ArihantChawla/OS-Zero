#ifndef __WCHAR_H__
#define __WCHAR_H__

#include <features.h>
//#include <stddef.h>
//#include <stdint.h>
#include <stdio.h>
#if !defined(__wchar_t_defined)
#include <share/wchar.h>
#endif
#include <share/wint.h>

#define WCHAR_MIN    0
#if (WCHARSIZE == 4)
#define WEOF         ((wint_t)0xfffffff) /* -1 */
#define WCHAR_MAX    0x0010ffff
#define WCHAR_MIN    (-0x7fffffff - 1)
#define WCHARNBIT    21
#elif (WCHARSIZE == 2)
#define WEOF         ((wint_t)0xffff) /* invalid character value in Unicode */
#define WCHAR_MAX    0xffff
#define WCHAR_MIN    0
#define WCHARNBIT    16
#elif (WCHARSIZE == 1)
#define WEOF         0x00
#define WCHAR_MAX    0xff
#define WCHAR_MIN    0
#define WCHARNBIT    8
#endif /* WCHARSIZE */
#define WCHARMASK    ((1 << WCHARNBIT) - 1)

struct               tm;

#if !defined(__KERNEL__)

#if (_GNU_SOURCE) || (_XOPEN_SOURCE >= 700 || _POSIX_C_SOURCE >= 200809L)
FILE *open_wmemstream(wchar_t **ptr, size_t *sizeptr);
#endif

/* wide-character I/O */
extern wint_t      btowc(int);
extern wint_t      fgetwc(FILE *);
extern wchar_t *   fgetws(wchar_t *__restrict, int, FILE *__restrict);
extern wint_t      fputwc(wchar_t, FILE *);
extern int         fputws(const wchar_t *__restrict, FILE *__restrict);
extern int         fwide(FILE *, int);
extern int         fwprintf(FILE *__restrict fp,
                            const wchar_t *__restrict fmt, ...);
extern int         fwscanf(FILE *__restrict fp,
                           const wchar_t *__restrict fmt, ...);
extern wint_t      getwc(FILE *);
extern wint_t      getwchar(void);
/* XSI extensions */
/* character classification */
extern int         iswalnum(wint_t wc);
extern int         iswalpha(wint_t wc);
extern int         iswcntrl(wint_t wc);
extern int         iswdigit(wint_t wc);
extern int         iswgraph(wint_t wc);
extern int         iswlower(wint_t wc);
extern int         iswprint(wint_t wc);
extern int         iswpunct(wint_t wc);
extern int         iswspace(wint_t wc);
extern int         iswupper(wint_t wc);
extern int         iswxdigit(wint_t wc);
extern int         iswctype(wint_t wc, wctype_t type);
/* iswblank() isn't in POSIX */
extern int         iswblank(wint_t wc);
extern size_t      mbrlen(const char *__restrict, size_t, mbstate_t *__restrict);
extern size_t      mbrtowc(wchar_t *__restrict, const char *__restrict, size_t,
                         mbstate_t *__restrict);
extern int         mbsinit(const mbstate_t *);
extern size_t      mbsrtowcs(wchar_t *__restrict, const char **__restrict, size_t,
                           mbstate_t *__restrict);
extern wint_t      putwc(wchar_t, FILE *);
extern wint_t      putwchar(wchar_t);
extern int         swprintf(wchar_t *__restrict, size_t,
                          const wchar_t *__restrict, ...);
extern int         swscanf(const wchar_t *__restrict,
                         const wchar_t *__restrict, ...);
extern wint_t      towlower(wint_t);
extern wint_t      towupper(wint_t);
extern wint_t      ungetwc(wint_t, FILE *);
extern int         vfwprintf(FILE *__restrict, const wchar_t *__restrict,
                             va_list);
extern int         vfwscanf(FILE *__restrict, const wchar_t *__restrict,
                            va_list);
extern int         vwprintf(const wchar_t *__restrict, va_list);
extern int         vswprintf(wchar_t *__restrict, size_t,
                             const wchar_t *__restrict, va_list);
extern int         vswscanf(const wchar_t *__restrict,
                            const wchar_t *__restrict,
                            va_list);
extern int         vwscanf(const wchar_t *__restrict, va_list);
extern size_t      wcrtomb(char *__restrict, wchar_t, mbstate_t *__restrict);
extern wchar_t   * wcscat(wchar_t *__restrict, const wchar_t *__restrict);
extern wchar_t   * wcschr(const wchar_t *, wchar_t);
extern int         wcscmp(const wchar_t *, const wchar_t *);
extern int         wcscoll(const wchar_t *, const wchar_t *);
extern wchar_t   * wcscpy(wchar_t *__restrict, const wchar_t *__restrict);
extern size_t      wcscspn(const wchar_t *, const wchar_t *);
extern size_t      wcsftime(wchar_t *__restrict, size_t,
                            const wchar_t *__restrict,
                            const struct tm *__restrict);
extern size_t      wcslen(const wchar_t *);
extern wchar_t   * wcsncat(wchar_t *__restrict, const wchar_t *__restrict,
                           size_t);
extern int         wcsncmp(const wchar_t *, const wchar_t *, size_t);
extern wchar_t   * wcsncpy(wchar_t *__restrict, const wchar_t *__restrict,
                           size_t);
extern wchar_t   * wcspbrk(const wchar_t *, const wchar_t *);
extern wchar_t   * wcsrchr(const wchar_t *, wchar_t);
extern size_t      wcsrtombs(char *__restrict, const wchar_t **__restrict,
                             size_t, mbstate_t *__restrict);
extern size_t      wcsspn(const wchar_t *, const wchar_t *);
extern wchar_t   * wcsstr(const wchar_t *__restrict, const wchar_t *__restrict);
extern double      wcstod(const wchar_t *__restrict, wchar_t **__restrict);
extern float       wcstof(const wchar_t *__restrict, wchar_t **__restrict);
extern wchar_t   * wcstok(wchar_t *__restrict, const wchar_t *__restrict,
                          wchar_t **__restrict);
extern long        wcstol(const wchar_t *__restrict, wchar_t **__restrict, int);
extern long double wcstold(const wchar_t *__restrict, wchar_t **__restrict);
extern long long   wcstoll(const wchar_t *__restrict, wchar_t **__restrict,
                           int);
extern unsigned long wcstoul(const wchar_t *__restrict, wchar_t **__restrict,
                             int);
extern unsigned long long wcstoull(const wchar_t *__restrict,
                                   wchar_t **__restrict,
                                   int);
extern wchar_t   * wcswcs(const wchar_t *, const wchar_t *);
extern int         wcswidth(const wchar_t *, size_t);
extern size_t      wcsxfrm(wchar_t *__restrict, const wchar_t *__restrict,
                           size_t);
extern int         wctob(wint_t);
extern wctype_t    wctype(const char *);
extern int         wcwidth(wchar_t);
extern wchar_t   * wmemchr(const wchar_t *, wchar_t, size_t);
extern int         wmemcmp(const wchar_t *, const wchar_t *, size_t);
extern wchar_t   * wmemcpy(wchar_t *__restrict, const wchar_t *__restrict,
                           size_t);
extern wchar_t   * wmemmove(wchar_t *, const wchar_t *, size_t);
extern wchar_t   * wmemset(wchar_t *, wchar_t, size_t);
extern int         wprintf(const wchar_t *__restrict, ...);
extern int         wscanf(const wchar_t *__restrict, ...);

#endif /* !defined(__KERNEL__) */

#endif /* __WCHAR_H__ */

