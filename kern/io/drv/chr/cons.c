/* TODO: console scrollback buffer */

#include <zero/trix.h>
#include <zero/gfx/rgb.h>
#include <kern/conf.h>
#include <kern/mem.h>
#include <kern/io/drv/chr/cons.h>
#include <kern/io/drv/pc/vga.h>
#if (VBE)
#include <kern/io/drv/pc/vbe.h>
#endif

struct cons constab[NCONS] ALIGNED(PAGESIZE);
long        conscur;

void
consinit(int w, int h)
{
//    struct cons *cons;
//    long         bufsz;
//    long         l;

#if (VBE)
    vbeinitcons(w, h);
#else
    vgainitcons(w, h);
#endif
#if 0
    cons = &constab[0];
    bufsz = CONSNTEXTROW * (w + 1) * sizeof(conschar_t);
    for (l = 0 ; l < NCONS ; l++) {
        cons->textbuf = kmalloc(bufsz);
        if (!cons->textbuf) {
            kprintf("CONS failed to allocate text buffer\n");
            
            return;
        }
        kbzero(cons->textbuf, bufsz);
        cons++;
    }
#endif
}

void
consscroll(struct cons *cons, long nrow)
{
    long row;

    if (nrow) {
        row = cons->row;
        row += nrow;
        if (nrow > 0) {
            row = min(row, CONSNTEXTROW - 1);
        } else {
            row = max(row, CONSNTEXTROW - 1);
        }
        cons->row = row;
    }
}

