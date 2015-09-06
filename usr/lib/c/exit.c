#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <zero/mtx.h>
#include <sys/zero/syscall.h>

struct exitcmd {
    void           *func;
    void           *arg;
    struct exitcmd *next;
};

struct exit {
    zeromtx         lk;
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

    mtxlk(&g_exit.lk);
    item = malloc(sizeof(struct exitcmd));
    if (!item) {
        fprintf(stderr, "ATEXIT: failed to allocate item\n");

        exit(1);
    }
    item->func = func;
    item->arg = NULL;
    item->next = g_exit.atexitq;
    g_exit.atexitq = item;
    mtxunlk(&g_exit.lk);

    return 0;
}

int
on_exit(void (*func)(int, void *), void *arg)
{
    struct exitcmd *item;

    mtxlk(&g_exit.lk);
    item = malloc(sizeof(struct exitcmd));
    if (!item) {
        fprintf(stderr, "ON_EXIT: failed to allocate item\n");

        exit(1);
    }
    item->func = func;
    item->arg = arg;
    item->next = g_exit.on_exitq;
    g_exit.on_exitq = item;
    mtxunlk(&g_exit.lk);

    return 0;
}

void
__on_exit(int status)
{
    struct exitcmd  *item;
    void           (*func)(int, void *);

    mtxlk(&g_exit.lk);
    item = g_exit.on_exitq;
    while (item) {
        func = item->func;
        func(status, item->arg);
        item = item->next;
    }
    mtxunlk(&g_exit.lk);
}

void
__atexit(void)
{
    struct exitcmd  *item;
    void           (*func)(void);

    mtxlk(&g_exit.lk);
    item = g_exit.atexitq;
    while (item) {
        func = item->func;
        func();
        item = item->next;
    }
    mtxunlk(&g_exit.lk);
}

void
__exit(int status, long flg)
{
    __atexit();
    if (!(flg & EXIT_NOFLUSH)) {
        fflush(stdout);
        fflush(stderr);
    }
    /*
     * NOTE: descriptors are closed, children inherited by init,
     * parent sent SIGCHLD
     */
    _syscall(SYS_EXIT, SYS_NOARG, status, SYS_NOARG);

    /* NOTREACHED */
}

