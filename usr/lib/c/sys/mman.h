#ifndef __SYS_MMAN_H__
#define __SYS_MMAN_H__

#include <features.h>
#include <stddef.h>
#include <sys/types.h>
#if (_ZERO_SOURCE)
#include <kern/syscall.h>
#endif

#define MAP_FAILED ((void *)-1)
#define PROT_EXEC  0x01
#define PROT_WRITE 0x02
#define PROT_READ  0x04

#if (_GNU_SOURCE)
typedef int error_t;
#endif

int   mlock(const void *adr, size_t len);
int   munlock(const void *adr, size_t len);
int   mlockall(int flg);
int   munlockall(void);
int   mprotect(void *adr, size_t len, int prot);
void *mmap(void *adr, size_t len, int prot, int flg, int fd, off_t ofs);
void  munmap(void *adr, size_t len);
int   msync(void *adr, size_t len, int flg);
#if (_BSD_SOURCE)
int   madvise(void *adr, size_t len, int advice);
#endif
/* FIXME: what version of XOPEN to check for? */
#if (_XOPEN_SOURCE)
int   posix_madvise(void *adr, size_t len, int advice);
#endif
/* FIXME: what systems is mincore() around on? */
int   mincore(void *adr, size_t len, unsigned char *_vec);
#if (_GNU_SOURCE)
void *mremap(void *adr, size_t len, size_t newlen, int flg);
int   remap_file_pages(void *adr, size_t len,
                       int prot, size_t ofs, int flg);
#endif
int shm_open(const char *name, int flg, mode_t mode);
int shm_unlink(const char *name);

#endif /* __SYS_MMAN_H__ */

