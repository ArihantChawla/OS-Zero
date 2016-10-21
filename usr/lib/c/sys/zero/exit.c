#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <zero/cdefs.h>
#include <zero/mtx.h>
#if defined(__ZEROKERNEL__)
#include <sys/zero/syscall.h>
#endif

#define EXIT_NOFUNC  (1 << 0)
#define EXIT_NOFLUSH (1 << 1)
#define EXIT_IMMED   (EXIT_NOFUNC | EXIT_NOFLUSH)

struct exitcmd {
    void           *func;
    void           *arg;
    struct exitcmd *next;
};

struct exit {
    zerofmtx        lk;
    size_t          natexit;
    struct exitcmd *atexitq;
    size_t          non_exit;
    struct exitcmd *on_exitq;
};

static struct exit g_exit;

int
atexit(void (*func)(void))
{
    struct exitcmd *item;

    fmtxlk(&g_exit.lk);
    item = malloc(sizeof(struct exitcmd));
    if (!item) {
        fprintf(stderr, "ATEXIT: failed to allocate item\n");

        exit(1);
    }
    item->func = func;
    item->arg = NULL;
    item->next = g_exit.atexitq;
    g_exit.atexitq = item;
    fmtxunlk(&g_exit.lk);

    return 0;
}

int
on_exit(void (*func)(int, void *), void *arg)
{
    struct exitcmd *item;

    fmtxlk(&g_exit.lk);
    item = malloc(sizeof(struct exitcmd));
    if (!item) {
        fprintf(stderr, "ON_EXIT: failed to allocate item\n");

        exit(1);
    }
    item->func = func;
    item->arg = arg;
    item->next = g_exit.on_exitq;
    g_exit.on_exitq = item;
    fmtxunlk(&g_exit.lk);

    return 0;
}

void
__on_exit(int status)
{
    struct exitcmd  *item;
    void           (*func)(int, void *);

    fmtxlk(&g_exit.lk);
    item = g_exit.on_exitq;
    while (item) {
        g_exit.on_exitq = item->next;
        fmtxunlk(&g_exit.lk);
        func = item->func;
        func(status, item->arg);
        fmtxlk(&g_exit.lk);
        item = g_exit.on_exitq;
    }
    fmtxunlk(&g_exit.lk);
}

void
__atexit(void)
{
    struct exitcmd  *item;
    void           (*func)(void);

    fmtxlk(&g_exit.lk);
    item = g_exit.atexitq;
    while (item) {
        g_exit.atexitq = item->next;
        fmtxunlk(&g_exit.lk);
        func = item->func;
        func();
        fmtxlk(&g_exit.lk);
        item = g_exit.on_exitq;
    }
    fmtxunlk(&g_exit.lk);
}

#if defined(__ZEROKERNEL__)

NORETURN
void
__exit(int status, long flg)
{
    if (!(flg & EXIT_NOFUNC)) {
        __atexit();
        __on_exit(status);
    }
    if (!(flg & EXIT_NOFLUSH)) {
        fflush(stdout);
        fflush(stderr);
    }
    /*
     * NOTE: descriptors are closed, children inherited by init,
     * parent sent SIGCHLD
     */
    _syscall(SYS_EXIT, SYS_NOARG, status, SYS_NOARG);
    for ( ; ; ) { ; }

    /* NOTREACHED */
}

NORETURN
void
_exit(int status)
{
    __exit(status, EXIT_IMMED);
    for ( ; ; ) { ; }

    /* NOTREACHED */
}

NORETURN
void
_Exit(int status)
{
    __exit(status, EXIT_IMMED);
    for ( ; ; ) { ; }

    /* NOTREACHED */
}

#endif /* defined(__ZEROKERNEL__) */

