#ifndef __SYS_BITS_STAT_H__
#define __SYS_BITS_STAT_H__

#include <kern/perm.h>
#include <kern/sys/stat.h>

#if !defined(S_ISUID)
#include <share/perm.h>
#endif

#define __S_IFMT   NODE_MASK
#define __S_IFIFO  NODE_FIFO
#define __S_IFCHR  NODE_CHR
#define __S_IFDIR  NODE_DIR
#define __S_IFBLK  NODE_BLK
#define __S_IFREG  NODE_REG
#define __S_IFLNK  NODE_LNK
#if (_BSD_SOURCE) || (USEUNIX98)
#define __S_IFSOCK NODE_SOCK
#endif
#if (_POSIX_SOURCE) && (_POSIX_C_SOURCE >= 199309L)
#define __S_IFMQ   NODE_MQ
#define __S_IFSEM  NODE_SEM
#define __S_IFSHM  NODE_SHM
#endif
#if !defined(_POSIX_SOURCE)
#define __S_IFWHT  0x16000000
#endif

#endif /* __SYS_BITS_STAT_H__ */

