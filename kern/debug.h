#ifndef __KERN_DEBUG_H__
#define __KERN_DEBUG_H__

#define M_TRAPFRAMESTK 0x00000001

#define kdprint(fmt, ...) kprintf(fmt, __VA_ARGS__)

#endif /* __KERN_DEBUG_H__ */

