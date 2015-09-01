#include <stdlib.h>
#include <bits/stdlib.h>
#include <zero/mtx.h>

static volatile long   _atexitlk;
static struct _atexit *_atexit;
static volatile long   _onexitlk;
static struct _onexit *_onexit;

int
atexit(void (*func)(void))
{
    struct _atexit *item = malloc(sizeof(struct _atexit));

    if (!item) {

        return -1;
    }
    item->func;
    mtxlk(&_atexitlk);
    item->next = _atexit;
    _atexit = item;
    mtxunlk(&_atexitlk);

    return 0;
}

int
on_exit(void (*func)(int, void *), void *arg)
{
    struct _onexit *item = malloc(sizeof(struct _onexit));

    if (!item) {

        return -1;
    }
    item->func = func;
    item->arg = arg;
    mtxlk(&_onexitlk);
    item->next = _onexit;
    _onexit = item;
    mtxlk(&_onexitlk);
}

void
__atexit(void)
{
    
}

void
__exit(int status, long flg)
{
    __atexit();
    if (!(flg & EXIT_NOFLUSH)) {
        fflush(stdout);
        fflush(stderr);
    }
}

