#ifndef __KERN_TYPES_H__
#define __KERN_TYPES_H__

#include <stdint.h>
#include <time.h>
#include <mach/atomic.h>
/* byte-pointer (for arithmetics */
#define m_castadr(adr, type) ((type)(adr))

/* signed types to facilitate negative return values such as -1 */
typedef int32_t desc_t;

struct stkhdr {
    int8_t *top;
    int8_t *sp;
};

#endif /* __KERN_TYPES_H__ */

