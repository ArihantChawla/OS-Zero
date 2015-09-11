#ifndef __KERN_SPARSE_H__
#define __KERN_SPARSE_H__

#if defined(__CHECKER__)

#define KERNELMEM __attribute__ ((address_space(0)))
#define USERMEM   __attribute__ ((noderef, address_space(1)))
#define IOMEM     __attribute__ ((address_space(2)))
#define PERCPUMEM __attribute__ ((address_space(3)))
#define PHYSMEM   __attribute__ ((address_space(5)))

#else

#define KERNELMEM
#define USERMEM
#define IOMEM
#define PERCPUMEM
#define PHYSMEM

#endif

#endif /* __KERN_SPARSE_H__ */

