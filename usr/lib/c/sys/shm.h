#ifndef __SYS_SHM_H__
#define __SYS_SHM_H__

#include <stddef.h>
#include <sys/time.h>

#define SHM_W 0x00000080
#define SHM_R 0x00000100

struct ipc_perm {
};

struct shmid_ds {
    struct ipc_perm shm_perm;
    size_t          shm_segsz;
    time_t          shm_atime;
};

#endif /* __SYS_SHM_H__ */

