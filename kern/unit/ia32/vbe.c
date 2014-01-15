#include <kern/conf.h>

#if (GERRY)

#include <stdint.h>

#include <kern/util.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/real.h>
#include <kern/io/drv/pc/vbe2.h>

extern void gdtinit(void);
extern void realint10(void);

struct realregs {
    int16_t di;
    int16_t si;
    int16_t bp;
    int16_t sp;
    int16_t bx;
    int16_t dx;
    int16_t cx;
    int16_t ax;
} PACK();

#define VBEPTR(x) ((((uint32_t)(x) & 0xffffffff) >> 12) | ((uint32_t)(x) & 0xffff))

struct vbeinfo vbectlinfo;

void
vbeint10(struct realregs *regs)
{
    static int first = 1;

    if (first) {
        kmemcpy((void *)KERNREALBASE,
                &realstart,
                (unsigned long)&realend - (unsigned long)&realstart);
        first = 0;
    }
    kmemcpy((void *)(KERNREALSTK - sizeof(struct realregs)),
            regs,
            sizeof(struct realregs));
    realint10();
    gdtinit();
}

void
vbegetinfo(void)
{
    struct realregs  regs;
    uint16_t        *modeptr;

    regs.ax = 0x4f00;
    regs.di = 0xa000;
    vbeint10(&regs);
    kmemcpy(&vbectlinfo, (void *)0xa000, sizeof(struct vbeinfo));
    modeptr = (uint16_t *)VBEPTR(vbectlinfo.modelst);
}

#endif

