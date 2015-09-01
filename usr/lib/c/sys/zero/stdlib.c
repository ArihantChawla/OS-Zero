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

struct stdio {
    volatile long   exitlk;
    size_t          natexit;
    struct exitcmd *atexitq;
    size_t          non_exit;
    struct exitcmd *on_exitq;
};

static struct stdio g_stdio;

int
atexit(void (*func)(void))
{
    struct exitcmd *item;

    mtxlk(&g_stdio.exitlk);
    item = malloc(sizeof(struct exitcmd));
    if (!item) {
        fprintf(stderr, "ATEXIT: failed to allocate item\n");

        exit(1);
    }
    item->func = func;
    item->arg = NULL;
    item->next = g_stdio.atexitq;
    g_stdio.atexitq = item;
    mtxunlk(&g_stdio.exitlk);

    return 0;
}

int
on_exit(void (*func)(int, void *), void *arg)
{
    struct exitcmd *item;

    mtxlk(&g_stdio.exitlk);
    item = malloc(sizeof(struct exitcmd));
    if (!item) {
        fprintf(stderr, "ON_EXIT: failed to allocate item\n");

        exit(1);
    }
    item->func = func;
    item->arg = arg;
    item->next = g_stdio.on_exitq;
    g_stdio.on_exitq = item;
    mtxunlk(&g_stdio.exitlk);

    return 0;
}

void
exit(int status)
{
    fflush(stdout);
    fflush(stderr);
    /*
     * NOTE: descriptors are closed, children inherited by init,
     * parent sent SIGCHLD
     */
    _syscall(SYS_EXIT, SYS_NOARG, status, SYS_NOARG);

    /* NOTREACHED */
}

