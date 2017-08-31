#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <v0/vm32.h>

v0reg
v0readkbd(struct v0 *vm, uint16_t port, uint16_t reg)
{
    v0reg *dest = &vm->genregs[reg];
    v0reg  key = getchar();

    if (key == EOF) {
        key = 0;
    }
    *dest = key;

    return key;
}

v0reg
v0writetty(struct v0 *vm, uint16_t port, uint16_t reg)
{
    v0reg *src = &vm->genregs[reg];
    int    ch = *src;

    printf("%c", ch);

    return ch;
}

v0reg
v0readrtc(struct v0 *vm, uint16_t port, uint16_t reg)
{
    v0reg  *dest = &vm->genregs[reg];
    time_t  tm = time(NULL);
    v0reg   val = (v0reg)tm;

    *dest = val;

    return val;
}

v0reg
v0readtmr(struct v0 *vm, uint16_t port, uint16_t reg)
{
    struct timeval  tv = { 0 };
    v0reg          *dest = &vm->genregs[reg];
    v0reg           val;

    gettimeofday(&tv, NULL);
    val = tv.tv_sec * 1000000;
    val += tv.tv_usec;
    *dest = val;

    return val;
}

void
vminitio(struct v0 *vm)
{
    struct v0iofuncs *vec = vm->iovec;

    vec[V0_STDIN_PORT].rdfunc = v0readkbd;
    vec[V0_STDOUT_PORT].wrfunc = v0writetty;
    vec[V0_STDERR_PORT].wrfunc = v0writetty;
    vec[V0_RTC_PORT].rdfunc = v0readrtc;
    vec[V0_TMR_PORT].rdfunc = v0readtmr;

    return;
}

