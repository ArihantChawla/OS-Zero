#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/trix.h>
#include <zas/zas.h>
#include <zvm/zvm.h>
#include <zvm/op.h>

extern struct zastoken *zastokenqueue;
extern unsigned long    zasinputread;
extern zasmemadr_t      _startadr;

zvmopfunc_t  *zvmfunctab[ZVMNOP] ALIGNED(PAGESIZE);
struct zvm    zvm;

void *
zvminitmem(long memsize)
{
    size_t len = ZASMEMSIZE;
    void *ptr = malloc(ZASMEMSIZE);

    while (!ptr) {
        len >>= 1;
        ptr = malloc(len);
    }
    zvm.physmem = ptr;
    zvm.memsize = len;

    return ptr;
}

void
zvminitasmop(uint8_t unit, uint8_t inst, uint8_t *str, uint8_t narg,
             zvmopfunc_t *func)
{
    long          id = (unit << 4) | inst;
    struct zasop *op = &zvminsttab[id];
    
    op->name = str;
    op->code = inst;
    op->narg = narg;
    if (!zasaddop(str, op)) {
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
    zvminitasmop(ZVMOPLOGIC, ZVMOPNOT, (uint8_t *)"not", 1, zvmopnot);
    zvminitasmop(ZVMOPLOGIC, ZVMOPAND, (uint8_t *)"and", 2, zvmopand);
    zvminitasmop(ZVMOPLOGIC, ZVMOPOR, (uint8_t *)"or", 2, zvmopor);
    zvminitasmop(ZVMOPLOGIC, ZVMOPXOR, (uint8_t *)"xor", 2, zvmopxor);
    /* shift operations */
    zvminitasmop(ZVMOPSHIFT, ZVMOPSHR, (uint8_t *)"shr", 2, zvmopshr);
    zvminitasmop(ZVMOPSHIFT, ZVMOPSAR, (uint8_t *)"sar", 2, zvmopsar);
    zvminitasmop(ZVMOPSHIFT, ZVMOPSHL, (uint8_t *)"shl", 2, zvmopshl);
    zvminitasmop(ZVMOPSHIFT, ZVMOPROR, (uint8_t *)"ror", 2, zvmopror);
    zvminitasmop(ZVMOPSHIFT, ZVMOPROL, (uint8_t *)"rol", 2, zvmoprol);
    /* arithmetic operations */
    zvminitasmop(ZVMOPARITH, ZVMOPINC, (uint8_t *)"inc", 1, zvmopinc);
    zvminitasmop(ZVMOPARITH, ZVMOPDEC, (uint8_t *)"dec", 1, zvmopdec);
    zvminitasmop(ZVMOPARITH, ZVMOPADD, (uint8_t *)"add", 2, zvmopadd);
    zvminitasmop(ZVMOPARITH, ZVMOPSUB, (uint8_t *)"sub", 2, zvmopsub);
    zvminitasmop(ZVMOPARITH, ZVMOPCMP, (uint8_t *)"cmp", 2, zvmopcmp);
    zvminitasmop(ZVMOPARITH, ZVMOPMUL, (uint8_t *)"mul", 2, zvmopmul);
    zvminitasmop(ZVMOPARITH, ZVMOPDIV, (uint8_t *)"div", 2, zvmopdiv);
    zvminitasmop(ZVMOPARITH, ZVMOPMOD, (uint8_t *)"mod", 2, zvmopmod);
    /* branch operations */
    zvminitasmop(ZVMOPBRANCH, ZVMOPJMP, (uint8_t *)"jmp", 1, zvmopjmp);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBZ, (uint8_t *)"bz", 1, zvmopbz);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBNZ, (uint8_t *)"bnz", 1, zvmopbnz);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBLT, (uint8_t *)"blt", 1, zvmopblt);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBLE, (uint8_t *)"ble", 1, zvmopble);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBGT, (uint8_t *)"bgt", 1, zvmopbgt);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBGE, (uint8_t *)"bge", 1, zvmopbge);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBO, (uint8_t *)"bo", 1, zvmopbo);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBNO, (uint8_t *)"bno", 1, zvmopbno);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBC, (uint8_t *)"bc", 1, zvmopbc);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBNC, (uint8_t *)"bnc", 1, zvmopbnc);
    /* stack operations */
    zvminitasmop(ZVMOPSTACK, ZVMOPPOP, (uint8_t *)"pop", 1, zvmoppop);
    zvminitasmop(ZVMOPSTACK, ZVMOPPUSH, (uint8_t *)"push", 1, zvmoppush);
    zvminitasmop(ZVMOPSTACK, ZVMOPPUSHA, (uint8_t *)"pusha", 0, zvmoppush);
    /* load/store */
    zvminitasmop(ZVMOPMOV, ZVMOPMOVL, (uint8_t *)"mov", 2, zvmopmovl);
    zvminitasmop(ZVMOPMOV, ZVMOPMOVL, (uint8_t *)"movl", 2, zvmopmovl);
    zvminitasmop(ZVMOPMOV, ZVMOPMOVB, (uint8_t *)"movb", 2, zvmopmovb);
    zvminitasmop(ZVMOPMOV, ZVMOPMOVW, (uint8_t *)"movw", 2, zvmopmovw);
#if (!ZAS32BIT)
    zvminitasmop(ZVMOPMOV, ZVMOPMOVQ, (uint8_t *)"movq", 2, zvmopmovq);
#endif
    /* function calls */
    zvminitasmop(ZVMOPFUNC, ZVMOPCALL, (uint8_t *)"call", 1, zvmopcall);
    zvminitasmop(ZVMOPFUNC, ZVMOPENTER, (uint8_t *)"enter", 1, zvmopenter);
    zvminitasmop(ZVMOPFUNC, ZVMOPLEAVE, (uint8_t *)"leave", 1, zvmopleave);
    zvminitasmop(ZVMOPFUNC, ZVMOPRET, (uint8_t *)"ret", 1, zvmopret);
    /* machine status word */
    zvminitasmop(ZVMOPMSW, ZVMOPLMSW, (uint8_t *)"lmsw", 1, zvmoplmsw);
    zvminitasmop(ZVMOPMSW, ZVMOPSMSW, (uint8_t *)"smsw", 1, zvmopsmsw);
    /* machine state */
    zvminitasmop(ZVMOPMACH, ZVMOPRESET, (uint8_t *)"reset", 0, zvmopreset);
    zvminitasmop(ZVMOPMACH, ZVMOPHLT, (uint8_t *)"hlt", 0, zvmophlt);
#if 0
    /* I/O operations */
    zvminitasmop(ZVMOPIO, ZVMINB, (uint8_t *)"inb", 2, zvmopinb);
    zvminitasmop(ZVMOPIO, ZVMINW, (uint8_t *)"inw", 2, zvmopinw);
    zvminitasmop(ZVMOPIO, ZVMINL, (uint8_t *)"inl", 2, zvmopinl);
#if (!ZAS32BIT)
    zvminitasmop(ZVMOPIO, ZVMINQ, (uint8_t *)"inq", 2, zvmopinq);
#endif
    zvminitasmop(ZVMOPIO, ZVMOUTB, (uint8_t *)"outb", 2, zvmopoutb);
    zvminitasmop(ZVMOPIO, ZVMOUTW, (uint8_t *)"outw", 2, zvmopoutw);
    zvminitasmop(ZVMOPIO, ZVMOUTL, (uint8_t *)"outl", 2, zvmopoutl);
#if (!ZAS32BIT)
    zvminitasmop(ZVMOPIO, ZVMOUTQ, (uint8_t *)"outq", 2, zvmopoutq);
#endif
#endif
    
    return;
};

void
zvminit(size_t memsize)
{
    zvminitmem(memsize);
    zvminitasm();
    zvm.sp = memsize;
    zvm.pc = ZASTEXTBASE;
}

void *
zvmloop(zasmemadr_t _startadr)
{
    zvmopfunc_t      *func;
    struct zvmopcode *op;
#if (ZVMTRACE)
    int               i;
#endif
#if (ZVMDB)
    struct zasline *line;
#endif

#if (ZVMTRACE) && 0
    fprintf(stderr, "memory\n");
    fprintf(stderr, "------\n");
    for (i = ZASTEXTBASE ; i < ZASTEXTBASE + 256 ; i++) {
        fprintf(stderr, "%02x ", physmem[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "registers\n");
    fprintf(stderr, "---------\n");
    fprintf(stderr, "---------\n");
    for (i = 0 ; i < NREG ; i++) {
        fprintf(stderr, "r%d:\t%x\n", i, zvm.regs[i]);
    }
#endif
    zvm.shutdown = 0;
//    memcpy(&zvm.cpustat, cpustat, sizeof(struct zvmcpustate));
//    free(cpustat);
    while (!zvm.shutdown) {
        op = (struct zvmopcode *)&zvm.physmem[zvm.pc];
        if (op->inst == ZVMOPNOP) {
            zvm.pc += sizeof(struct zvmopcode);
        } else {
//            zvm.cpustat.pc = rounduppow2(zvm.pc, sizeof(zasword_t));
            op = (struct zvmopcode *)&zvm.physmem[zvm.pc];
            func = zvmfunctab[op->inst];
#if (ZVMTRACE)
            zvmprintop(op);
#endif
            if (func) {
#if (ZVMDB)
                line = zasfindline(zvm.pc);
                if (line) {
                    fprintf(stderr, "%s:%ld:\t%s\n", line->file, line->num, line->data);
                }
#endif
                func(op);
            } else {
                fprintf(stderr, "illegal instruction, PC == %lx\n",
                        (long)zvm.pc);
#if (ZVM)
//                zvmprintop(op);
#endif
                
                exit(1);
            }
        }
    }
#if (ZVMTRACE)
    fprintf(stderr, "memory\n");
    fprintf(stderr, "------\n");
    for (i = ZASTEXTBASE ; i < ZASTEXTBASE + 256 ; i++) {
        fprintf(stderr, "%02x ", physmem[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "registers\n");
    fprintf(stderr, "---------\n");
    for (i = 0 ; i < NREG ; i++) {
        fprintf(stderr, "r%d:\t%x\n", i, zvm.regs[i]);
    }
#endif

    return NULL;
}

int
zvmmain(int argc, char *argv[])
{
    long        l;
    zasmemadr_t adr = ZASTEXTBASE;
#if (ZASPROF)
    PROFDECLCLK(clk);
#endif

    if (argc < 2) {
        fprintf(stderr, "usage: zvm <file1> ...\n");

        exit(1);
    }
    memset(zvm.physmem, 0, ZASTEXTBASE);
#if (ZASPROF)
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
            zasresolve(ZASTEXTBASE);
            zasremovesyms();
#if (ZASPROF)
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
    zvmloop(_startadr);

    /* NOTREACHED */
    exit(0);
}

struct zastoken *
zasprocinst(struct zastoken *token, zasmemadr_t adr, zasmemadr_t *retadr)
{
    struct zvmopcode *op = NULL;
    zasmemadr_t       opadr = rounduppow2(adr, sizeof(struct zvmopcode));
    struct zastoken  *token1 = NULL;
    struct zastoken  *token2 = NULL;
    struct zastoken  *retval = NULL;
    struct zassymrec *sym;
    uint8_t           narg = token->data.inst.narg;
    uint8_t           len = sizeof(struct zvmopcode);

    while (adr < opadr) {
        zvm.physmem[adr] = ZVMOPNOP;
        adr++;
    }
#if (ZASDB)
    zasaddline(adr, token->data.inst.data, token->file, token->line);
#endif
    op = (struct zvmopcode *)&zvm.physmem[adr];
    op->inst = token->data.inst.op;
    if (op->inst == ZVMOPNOP) {
        retval = token->next;
        adr++;
    } else if (!narg) {
        op->reg1 = 0;
        op->reg2 = 0;
        retval = token->next;
    } else {
        token1 = token->next;
        zasfreetoken(token);
        if (token1) {
            switch(token1->type) {
                case ZASTOKENVALUE:
                    op->arg1t = ZVMARGIMMED;
                    op->args[0] = token1->data.value.val;
                    len += sizeof(zasword_t);
                    
                    break;
                case ZASTOKENREG:
                    op->arg1t = ZVMARGREG;
                    op->reg1 = token1->data.reg;
                    
                    break;
                case ZASTOKENSYM:
                    op->arg1t = ZVMARGADR;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = (uint8_t *)strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->args[0];
                    zasqueuesym(sym);
                    len += sizeof(zasmemadr_t);
                    
                    break;
                case ZASTOKENINDIR:
                    token1 = token1->next;
                    if (token1->type == ZASTOKENREG) {
                        op->arg1t = ZVMARGREG;
                        op->reg1 = token1->data.reg;
                    } else {
                        fprintf(stderr, "indirect addressing requires a register\n");
                        
                        exit(1);
                    }
                    
                    break;
                case ZASTOKENIMMED:
                    op->arg1t = ZVMARGIMMED;
                    op->args[0] = token1->val;
                    len += sizeof(zasword_t);
                    
                    break;
                case ZASTOKENADR:
                    op->arg1t = ZVMARGIMMED;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = (uint8_t *)strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->args[0];
                    zasqueuesym(sym);
                    len += sizeof(zasmemadr_t);
                    
                    break;
                case ZASTOKENINDEX:
                    op->arg1t = ZVMARGREG;
                    op->reg1 = token1->data.ndx.reg;
                    op->args[0] = token1->data.ndx.val;
                    len += sizeof(zasword_t);
                    
                    break;
                default:
                    fprintf(stderr, "invalid argument 1 of type %lx\n", token1->type);
                    zasprinttoken(token1);
                    
                    exit(1);
                    
                    break;
            }
            token2 = token1->next;
            zasfreetoken(token1);
            retval = token2;
        }
        if (narg == 1) {
//                    op->arg2t = ZVMARGNONE;
        } else if (narg == 2 && (token2)) {
            switch(token2->type) {
                case ZASTOKENVALUE:
                    op->arg2t = ZVMARGIMMED;
                    if (op->arg1t == ZVMARGREG) {
                        op->args[0] = token2->data.value.val;
                    } else {
                        op->args[1] = token2->data.value.val;
                    }
                    len += sizeof(zasword_t);
                    
                    break;
                case ZASTOKENREG:
                    op->arg2t = ZVMARGREG;
                    op->reg2 = token2->data.reg;
                    
                    break;
                case ZASTOKENSYM:
                    op->arg2t = ZVMARGADR;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = (uint8_t *)strdup((char *)token2->data.sym.name);
                    if (op->arg1t == ZVMARGREG) {
                        sym->adr = (uintptr_t)&op->args[0];
                    } else {
                        sym->adr = (uintptr_t)&op->args[1];
                    }
                    zasqueuesym(sym);
                    len += sizeof(zasmemadr_t);
                    
                    break;
                case ZASTOKENINDIR:
                    token2 = token2->next;
                    if (token2->type == ZASTOKENREG) {
                        op->arg2t = ZVMARGREG;
                        op->reg2 = token2->data.reg;
                    } else {
                        fprintf(stderr, "indirect addressing requires a register\n");
                        
                        exit(1);
                    }
                    
                    break;
                case ZASTOKENIMMED:
                    op->arg2t = ZVMARGIMMED;
                    if (op->arg1t == ZVMARGREG) {
                        op->args[0] = token2->val;
                    } else {
                        op->args[1] = token2->val;
                    }
                    len += sizeof(zasword_t);
                    
                    break;
                case ZASTOKENADR:
                    op->arg2t = ZVMARGIMMED;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = (uint8_t *)strdup((char *)token2->data.sym.name);
                    if (op->arg1t == ZVMARGREG) {
                        sym->adr = (uintptr_t)&op->args[0];
                    } else {
                        sym->adr = (uintptr_t)&op->args[1];
                    }
                    zasqueuesym(sym);
                    len += sizeof(zasmemadr_t);
                    
                    break;
                case ZASTOKENINDEX:
                    op->arg2t = ZVMARGREG;
                    op->reg2 = token2->data.ndx.reg;
                    if (op->arg1t == ZVMARGREG) {
                        op->args[0] = token2->data.ndx.val;
                    } else {
                        op->args[1] = token2->data.ndx.val;
                    }
                    len += sizeof(zasword_t);
                    
                    break;
                default:
                    fprintf(stderr, "invalid argument 2 of type %lx\n", token2->type);
                    zasprinttoken(token2);
                    
                    exit(1);
                    
                    break;
            }
            retval = token2->next;
            zasfreetoken(token2);
        }
    }
    op->size = len >> 2;
    *retadr = adr + len;

    return retval;
}

int
main(int argc, char *argv[])
{
    zasinit(NULL, NULL);
    zvminit(ZASMEMSIZE);

    exit(zvmmain(argc, argv));
}

