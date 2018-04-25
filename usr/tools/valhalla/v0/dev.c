#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <v0/vm.h>

void
v0readkbd(struct v0 *vm, uint16_t port, uint8_t reg)
{
    v0reg *dest = v0regtoptr(vm, reg);
    v0reg  key = getchar();

    if (key == EOF) {
        key = 0;
    }
    *dest = key;

    return;
}

void
v0writetty(struct v0 *vm, uint16_t port, uint8_t reg)
{
    v0reg *src = v0regtoptr(vm, reg);
    int    ch = *src;

    printf("%c", ch);

    return;
}

void
v0readrtc(struct v0 *vm, uint16_t port, uint8_t reg)
{
    v0reg  *dest = v0regtoptr(vm, reg);
    time_t  tm = time(NULL);
    v0reg   val = (v0reg)tm;

    *dest = val;

    return;
}

void
v0readtmr(struct v0 *vm, uint16_t port, uint8_t reg)
{
    struct timeval  tv = { 0 };
    v0reg          *dest = v0regtoptr(vm, reg);
    v0reg           val;

    gettimeofday(&tv, NULL);
    val = tv.tv_sec * 1000000;
    val += tv.tv_usec;
    *dest = val;

    return;
}

void
v0writevtd(struct v0 *vm, uint16_t port, uint8_t reg)
{
    int   ch = vm->regs[reg] & 0xff;
    FILE *fp = vm->vtdfp;
    int   ret;

    ret = fputc(ch, fp) == EOF;
    if (ret != EOF) {

        return;
    }
    fprintf(stderr, "V0: failed to print to VTD-file %s\n",
            vm->vtdpath);

    exit(1);
}

