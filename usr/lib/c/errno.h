#ifndef __ERRNO_H__
#define __ERRNO_H__

#include <bits/errno.h>

#if !defined(__ASSEMBLER__)

#if !defined(__KERNEL__)

#if defined(__GLIBC__)

extern int errno;

#else

#if defined(__GNUC__)
__attribute__ ((const))
#endif /* __GNUC__ */

extern int * __errnoloc(void);
#define errno (*__errnoloc())

#endif /* defined(__GLIBC__) */

#endif /* !defined(__KERNEL__) */

#endif /* !defined(__ASSEMBLER__) */

#endif /* __ERRNO_H__ */

