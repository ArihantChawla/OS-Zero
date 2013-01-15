#ifndef __ZERO_X86_64_PARAM_H__
#define __ZERO_X86_64_PARAM_H__

#define CHARSIZE     1
#define SHORTSIZE    2
#define INTSIZE      4
#define LONGSIZE     8
#define LONGSIZELOG2 3
#define LONGLONGSIZE 8
#define PTRSIZE      8
#define PTRBITS      64
#define ADDRBITS     48 // # of significant bytes in phys and virt addresses
#define PAGESIZELOG2 12 // 4-kilobyte pages

#define CLSIZE       32
#define PAGESIZE     (1L << PAGESIZELOG2)

#endif /* __ZERO_X86_64_PARAM_H__ */

