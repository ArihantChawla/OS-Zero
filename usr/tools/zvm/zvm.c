#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#if !(ZVMVIRTMEM) || (ZASDEBUG)
#include <assert.h>
#endif
#if (ZVMXORG)
#include <unistd.h>
#include <zvm/xorg.h>
#elif (ZVMXCB)
#include <unistd.h>
#include <xcb/xcb.h>
#endif
#include <zero/param.h>
#include <zero/cdecl.h>
//#include <zero/trix.h>
#include <zas/zas.h>
#include <zvm/zvm.h>
#include <zvm/op.h>
#include <zvm/mem.h>
#if (ZVMEFL)
#include <zvm/efl.h>
#endif
#if (ZVMXCB)
#include <zvm/xcb.h>
#endif

extern struct zastoken *zastokenqueue;
extern unsigned long    zasinputread;
extern zasmemadr_t      _startadr;

zvmopfunc_t            *zvmfunctab[ZVMNOP] ALIGNED(PAGESIZE);
struct zvm              zvm;

void
zvminitasmop(uint8_t unit, uint8_t inst, const char *str, uint8_t narg,
             zvmopfunc_t *func)
{
    uint8_t       id = (unit << 4) | inst;
    struct zasop *op = &zvminsttab[id];
    
    op->name = str;
    op->code = id;
    op->narg = narg;
    if (!asmaddop(str, op)) {
        fprintf(stderr, "failed to initialise assembly operation:\n");
        fprintf(stderr, "unit == %d, inst == %d, str == %s, narg = %d\n",
                (int)unit, (int)inst, str, (int)narg);
        
        exit(1);
    }
    zvmfunctab[id] = func;
}

void
zvminitasm(void)
{
    /* logical operations */
    zvminitasmop(ZVMOPLOGIC, ZVMOPNOT, "not", 1, zvmopnot);
    zvminitasmop(ZVMOPLOGIC, ZVMOPAND, "and", 2, zvmopand);
    zvminitasmop(ZVMOPLOGIC, ZVMOPOR, "or", 2, zvmopor);
    zvminitasmop(ZVMOPLOGIC, ZVMOPXOR, "xor", 2, zvmopxor);
    /* shift operations */
    zvminitasmop(ZVMOPSHIFT, ZVMOPSHR, "shr", 2, zvmopshr);
    zvminitasmop(ZVMOPSHIFT, ZVMOPSAR, "sar", 2, zvmopsar);
    zvminitasmop(ZVMOPSHIFT, ZVMOPSHL, "shl", 2, zvmopshl);
    zvminitasmop(ZVMOPSHIFT, ZVMOPROR, "ror", 2, zvmopror);
    zvminitasmop(ZVMOPSHIFT, ZVMOPROL, "rol", 2, zvmoprol);
    /* arithmetic operations */
    zvminitasmop(ZVMOPARITH, ZVMOPINC, "inc", 1, zvmopinc);
    zvminitasmop(ZVMOPARITH, ZVMOPDEC, "dec", 1, zvmopdec);
    zvminitasmop(ZVMOPARITH, ZVMOPADD, "add", 2, zvmopadd);
    zvminitasmop(ZVMOPARITH, ZVMOPSUB, "sub", 2, zvmopsub);
    zvminitasmop(ZVMOPARITH, ZVMOPCMP, "cmp", 2, zvmopcmp);
    zvminitasmop(ZVMOPARITH, ZVMOPMUL, "mul", 2, zvmopmul);
    zvminitasmop(ZVMOPARITH, ZVMOPDIV, "div", 2, zvmopdiv);
    zvminitasmop(ZVMOPARITH, ZVMOPMOD, "mod", 2, zvmopmod);
    /* branch operations */
    zvminitasmop(ZVMOPBRANCH, ZVMOPJMP, "jmp", 1, zvmopjmp);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBZ, "bz", 1, zvmopbz);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBNZ, "bnz", 1, zvmopbnz);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBLT, "blt", 1, zvmopblt);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBLE, "ble", 1, zvmopble);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBGT, "bgt", 1, zvmopbgt);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBGE, "bge", 1, zvmopbge);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBO, "bo", 1, zvmopbo);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBNO, "bno", 1, zvmopbno);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBC, "bc", 1, zvmopbc);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBNC, "bnc", 1, zvmopbnc);
    /* stack operations */
    zvminitasmop(ZVMOPSTACK, ZVMOPPOP, "pop", 1, zvmoppop);
    zvminitasmop(ZVMOPSTACK, ZVMOPPUSH, "push", 1, zvmoppush);
    zvminitasmop(ZVMOPSTACK, ZVMOPPUSHA, "pusha", 0, zvmoppush);
    /* load/store */
    zvminitasmop(ZVMOPMOV, ZVMOPMOVL, "mov", 2, zvmopmovl);
//    zvminitasmop(ZVMOPMOV, ZVMOPMOVL, "movl", 2, zvmopmovl);
    zvminitasmop(ZVMOPMOV, ZVMOPMOVB, "movb", 2, zvmopmovb);
    zvminitasmop(ZVMOPMOV, ZVMOPMOVW, "movw", 2, zvmopmovw);
#if (!ZVM32BIT)
    zvminitasmop(ZVMOPMOV, ZVMOPMOVQ, "movq", 2, zvmopmovq);
#endif
    /* function calls */
    zvminitasmop(ZVMOPFUNC, ZVMOPCALL, "call", 1, zvmopcall);
    zvminitasmop(ZVMOPFUNC, ZVMOPENTER, "enter", 1, zvmopenter);
    zvminitasmop(ZVMOPFUNC, ZVMOPLEAVE, "leave", 1, zvmopleave);
    zvminitasmop(ZVMOPFUNC, ZVMOPRET, "ret", 1, zvmopret);
#if 0
    /* thread interface */
    zvminitasmop(ZVMOPFUNC, ZVMOPTHR, "thr", 1, zvmopthr);
#endif
    /* machine status word */
    zvminitasmop(ZVMOPMSW, ZVMOPLMSW, "lmsw", 1, zvmoplmsw);
    zvminitasmop(ZVMOPMSW, ZVMOPSMSW, "smsw", 1, zvmopsmsw);
    /* machine state */
    zvminitasmop(ZVMOPMACH, ZVMOPRESET, "reset", 0, zvmopreset);
    zvminitasmop(ZVMOPMACH, ZVMOPHLT, "hlt", 0, zvmophlt);
#if 0
    /* I/O operations */
    zvminitasmop(ZVMOPIO, ZVMINB, "inb", 2, zvmopinb);
    zvminitasmop(ZVMOPIO, ZVMINW, "inw", 2, zvmopinw);
    zvminitasmop(ZVMOPIO, ZVMINL, "inl", 2, zvmopinl);
#if (!ZVM32BIT)
    zvminitasmop(ZVMOPIO, ZVMINQ, "inq", 2, zvmopinq);
#endif
    zvminitasmop(ZVMOPIO, ZVMOUTB, "outb", 2, zvmopoutb);
    zvminitasmop(ZVMOPIO, ZVMOUTW, "outw", 2, zvmopoutw);
    zvminitasmop(ZVMOPIO, ZVMOUTL, "outl", 2, zvmopoutl);
#if (!ZVM32BIT)
    zvminitasmop(ZVMOPIO, ZVMOUTQ, "outq", 2, zvmopoutq);
#endif
#endif /* 0 */
    
    return;
};

void
zvminit(void)
{
    size_t memsize;
    
    memsize = zvminitmem();
    if (!memsize) {
        fprintf(stderr, "zvm: FAILED to allocate machine memory\n");

        exit(1);
    }
    zvminitasm();
#if (!ZVMVIRTMEM)
    zvm.cregs[ZVMSPCREG] = memsize;
#endif
    zvm.cregs[ZVMPCCREG] = ZVMTEXTBASE;
    zvminitio();
#if (ZVMEFL) || (ZVMXORG) || (ZVMXCB)
    zvminitui();
#endif

    return;
}

void *
zvmloop(zasmemadr_t _startadr)
{
    zvmopfunc_t         *func;
    struct zvmopcode    *op;
#if (ZVMTRACE)
    int                  i;
#endif
#if (ZVMDB)
    struct zasline      *line;
#endif
#if (ZVMXCB)
    xcb_void_cookie_t    cookie;
#endif

#if (ZVMTRACE)
    fprintf(stderr, "memory\n");
    fprintf(stderr, "------\n");
    for (i = ZVMTEXTBASE ; i < ZVMTEXTBASE + 256 ; i++) {
        fprintf(stderr, "%02x ", (int8_t)(zvmgetmemt(i, int8_t)) & 0xff);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "registers\n");
    fprintf(stderr, "---------\n");
    fprintf(stderr, "---------\n");
    for (i = 0 ; i < ZVMNREG ; i++) {
        fprintf(stderr, "r%d:\t0x%lx\n", i, (long)zvm.regs[i]);
    }
#endif
    zvm.shutdown = 0;
    zvm.cregs[ZVMPCCREG] = _startadr;
//    memcpy(&zvm.cpustat, cpustat, sizeof(struct zvmcpustate));
//    free(cpustat);
    while (!zvm.shutdown) {
#if (ZVMXCB)
        xcbdoevent();
#endif
//        op = (struct zvmopcode *)&zvm.physmem[zvm.cregs[ZVMPCCREG]];
        op = (struct zvmopcode *)&zvm.physmem[zvm.cregs[ZVMPCCREG]];
        if (op->code == ZVMOPNOP) {
            zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);
        } else {
//            zvm.cpustat.pc = rounduppow2(zvm.cregs[ZVMPCCREG], sizeof(zasword_t));
//            op = (struct zvmopcode *)&zvm.physmem[zvm.cregs[ZVMPCCREG]];
//            op = &zvm.physmem[zvm.cregs[ZVMPCCREG]];
            func = zvmfunctab[op->code];
#if (ZVMTRACE)
            asmprintop(op);
#endif
            if (func) {
#if (ZVMDB)
                line = zasfindline(zvm.cregs[ZVMPCCREG]);
                if (line) {
                    fprintf(stderr, "%s:%ld:\t%s\n", line->file, line->num, line->data);
                }
#endif
                func(op);
            } else {
                fprintf(stderr, "illegal instruction, PC == %lx\n",
                        (long)zvm.cregs[ZVMPCCREG]);
#if (ZVM)
//                asmprintop(op);
#endif
                
                exit(1);
            }
        }
    }
#if (ZVMTRACE)
    fprintf(stderr, "memory\n");
    fprintf(stderr, "------\n");
    for (i = ZVMTEXTBASE ; i < ZVMTEXTBASE + 256 ; i++) {
        fprintf(stderr, "%02x ", (int8_t)(zvmgetmemt(i, int8_t)) & 0xff);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "registers\n");
    fprintf(stderr, "---------\n");
    for (i = 0 ; i < ZVMNREG ; i++) {
        fprintf(stderr, "r%d:\t0x%lx\n", i, (long)zvm.regs[i]);
    }
#endif
#if (ZVMXORG) || (ZVMXCB)
    while (1) {
        zvmdouievent();
    }
#endif

    return NULL;
}

int
zvmmain(int argc, char *argv[])
{
    long        l;
    zasmemadr_t adr = ZVMTEXTBASE;
#if (ZVMPROF)
    PROFDECLCLK(clk);
#endif

    if (argc < 2) {
        fprintf(stderr, "usage: zvm <file1> ...\n");

        exit(1);
    }
    zvminit();
    zvminitopt();
    zasinit();
#if (ZVMEFL)
    zvminitui();
#endif
#if (!ZVMVIRTMEM)
    if (ZVMTEXTBASE) {
        fprintf(stderr, "ZVMTEXTBASE == %ld\n", ZVMTEXTBASE);
        memset(zvm.physmem, 0, ZVMTEXTBASE);
    }
#endif
#if (ZVMPROF)
    profstartclk(clk);
#endif
    for (l = 1 ; l < argc ; l++) {
#if (ZASBUF)
        zasreadfile(argv[l], adr, readbufcur);
#else
        zasreadfile(argv[l], adr);
#endif
        if (!zastokenqueue) {
            fprintf(stderr, "WARNING: no input in %s\n", argv[l]);
        } else {
            zasinputread = 1;
            adr = zastranslate(adr);
            zasresolve(ZVMTEXTBASE);
            zasremovesyms();
#if (ZVMPROF)
            profstopclk(clk);
            fprintf(stderr, "%ld microseconds to process %s\n",
                    profclkdiff(clk), argv[l]);
#endif        
        }
    }
    if (!zasinputread) {
        fprintf(stderr, "empty input\n");

        exit(1);
    }
    fprintf(stderr, "START: %lx\n", (long)_startadr);
#if (ZVMEFL)
    ecore_main_loop_begin();
//    sleep(5);
#else
    zvmloop(_startadr);
#endif

    /* NOTREACHED */
    exit(0);
}

#if 0
int
main(int argc, char *argv[])
{
    exit(zvmmain(argc, argv));
}
#endif

