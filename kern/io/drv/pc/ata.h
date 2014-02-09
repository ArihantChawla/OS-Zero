#ifndef __KERN_IO_DRV_PC_ATAPI_H__
#define __KERN_IO_DRV_PC_ATAPI_H__

#define ATAIDENTIFY  0xec
#define ATAPRIMARY   0x1f0
#define ATASECONDARY 0x170
#define ATASELMASTER 0xa0
#define ATASELSLAVE  0xb0
#define ATAERR       0x01
#define ATADRQ       0x08
#define ATABSY       0x80
#define ATANERR      6

#endif /* __KERN_IO_DRV_PC_ATAPI_H__ */

