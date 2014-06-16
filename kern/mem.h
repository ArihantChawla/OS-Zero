#ifndef __KERN_MEM_H__
#define __KERN_MEM_H__

#include <kern/mem/mem.h>
#define  kwalloc(nb)  memalloc(nb, MEMWIRE)
#define  kmalloc(nb)  memalloc(nb, 0)
#define  kcalloc(nb)  memalloc(nb, MEMZERO)
#define  kcwalloc(nb) memalloc(nb, MEMZERO | MEMWIRE)
void    *memalloc(unsigned long nb, long flg);
void     kfree(void *ptr);

#endif /* __KERN_MEM_H__ */

