#ifndef __SYS_IPC_H__
#define __SYS_IPC_H__

#include <sys/types.h>

#define IPC_CREAT  0x00001000
#define IPC_EXCL   0x00002000
#define IPC_NOWAIT 0x00004000

#define IPC_RMID   0
#define IPC_SET    1
#define IPC_STAT   2
#if (_GNU_SOURCE)
#define IPC_INFO   3
#endif

#define IPC_PRIVATE ((key_t)0)
struct ipc_perm {
	key_t          key;
	uid_t          uid;
	gid_t          gid;
	uid_t          cuid;
	gid_t          cgid;
	unsigned short mode;
	unsigned short res1;
	unsigned short _seq;
	unsigned short res2;
	unsigned long  res3[2];
};

#if (!__KERNEL__)
extern key_t ftok(const char *path, int proj);
#endif

#endif /* __SYS_IPC_H__ */

