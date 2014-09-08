#ifndef __ZERO_X86_64_PARAM_H__
#define __ZERO_X86_64_PARAM_H__

/* FIXME: Windows uses 32-bit long on x86-64 */
#define CHARSIZE     1
#define SHORTSIZE    2
#define INTSIZE      4
#if defined(_WIN32) || defined(__WIN32) || defined(__WIN32__) \
    || defined(__CYGWIN__) || defined(__CYGWIN32__) \
    || defined(WIN64) || defined(_WIN64) \
	|| defined(__WIN64) || defined(__WIN64__) \
	|| defined(__MINGW32__) || defined(__MINGW64__)
#define LONGSIZE     4
#define LONGSIZELOG2 2
#else
#define LONGSIZE     8
#define LONGSIZELOG2 3
#endif
#define LONGLONGSIZE 8
#define PTRSIZE      8
#define PTRBITS      64
#define ADRBITS      48 // # of significant bytes in phys and virt addresses
#define PAGESIZELOG2 12 // 4-kilobyte pages

#define CLSIZE       64
#define PAGESIZE     (1L << PAGESIZELOG2)

#endif /* __ZERO_X86_64_PARAM_H__ */

