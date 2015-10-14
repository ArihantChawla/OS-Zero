#ifndef __ZERO_IA32_PARAM_H__
#define __ZERO_IA32_PARAM_H__

/* # of I/O ports */
#define NIOPORT      65536

#define WORDSIZE     4
#define CHARSIZE     1
#define SHORTSIZE    2
#define INTSIZE      4
#define LONGSIZE     4
#define LONGSIZELOG2 2
#define LONGLONGSIZE 8
#define PTRSIZE      4
#define PTRBITS      32
#define ADRBITS      32
#define PAGESIZELOG2 12

#define CLSIZE       32
#define CLSIZELOG2   5
#define PAGESIZE     (1 << PAGESIZELOG2)

#endif /* __ZERO_IA32_PARAM_H__ */

