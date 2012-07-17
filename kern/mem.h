#ifndef __KERN_MEM_H__
#define __KERN_MEM_H__

#define  kwalloc(nb) memalloc(nb, PAGEWIRED)
#define  kmalloc(nb) memalloc(nb, 0)
void    *memalloc(unsigned long nb, long flg);
void     kfree(void *ptr);

#endif /* __KERN_MEM_H__ */

