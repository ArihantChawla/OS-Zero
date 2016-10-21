#ifndef __AIO_H__
#define __AIO_H__

#include <features.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <zero/param.h>

/* return values for cancelation */
#define AIO_CANCELED    0
#define AIO_NOTCANCELED 1
#define AIO_ALLDONE     2

/* synchronisation modes for lio_listio() */
#define LIO_NOWAIT      0
#define LIO_WAIT        1

/* operation codes for aio_lio_opcode-field */
#define LIO_READ        1
#define LIO_WRITE       2
#define LIO_NOP         3
struct aiocb {
    off_t            aio_offset;        // file offset
    volatile void   *aio_buf;           // buffer location
    size_t           aio_nbytes;        // transfer length in bytes
    struct sigevent  aio_sigevent;      // signal number and value
    int              aio_fildes;        // file descriptor
    int              aio_lio_opcode;    // operation
    int              aio_reqprio;       // request priority offset
    uint8_t          _res[CLSIZE - sizeof(off_t) - sizeof(void *)
                          - sizeof(struct sigevent) - 3 * sizeof(int)];
};
#if 0
struct aiocb {
    int              aio_fildes;        // file descriptor
    off_t            aio_offset;        // file offset
    volatile void   *aio_buf;           // buffer location
    size_t           aio_nbytes;        // transfer length in bytes
    struct sigevent  aio_sigevent;      // signal number and value
    int              aio_lio_opcode;    // operation
    int              aio_reqprio;       // request priority offset
};
#endif

#if (USEGNU)
struct aioinit {
    int aio_threads;		// max # of threads
    int aio_num;		// # of expected simultanious requests
    int aio_locks;		// not used
    int aio_usedba;		// not used
    int aio_debug;		// not used
    int aio_numusers;		// not used
    int aio_idle_time;		// # of idle seconds before termination
    int aio_reserved;
};
#endif

#if !defined(__KERNEL__)

#if (USEGNU)
extern void aioinit(const struct aioinit *init);
#endif

extern int     aio_cancel(int fd, struct aiocb *aiocb);
extern int     aio_error(const struct aiocb *aiocb);
extern int     aio_fsync(int op, struct aiocb *aiocb);
extern ssize_t aio_return(struct aiocb *aiocb);
extern int     aio_suspend(const struct aiocb *const list[], int nent,
                           const struct timespec *__restrict timeout);
extern int     aio_read(struct aiocb *aiocb);
extern int     aio_write(struct aiocb *aiocb);
extern int     lio_listio(int mode, struct aiocb *const list[],
                          int nent, struct sigevent *__restrict sigev);

#endif /* !__KERNEL__ */

#endif /* __AIO_H__ */

