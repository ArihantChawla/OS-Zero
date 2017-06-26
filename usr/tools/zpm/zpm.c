#include <stdlib.h>
#include <zpm/zpm.h>
#include <zpm/op.h>

#if defined(__GNUC__)
#define opadr(x) &&x
#define OPTAB_T  static void *
#else
#define opadr(x) &x
typedef zpmureg  zpmopfunc(struct zpm *vm, uint8_t *ptr, zpmureg pc);
#define OPTAB_T  zpmopfunc *
#endif

#define opjmp(op) goto *jmptab[((struct zpmop *)&vm->mem[pc])->code]

int
zpmloop(struct zpm *vm, zpmureg pc)
{
    OPTAB_T jmptab[] = {
        opadr(zpmopnop),
        opadr(zpmopnot),
        opadr(zpmopand),
        opadr(zpmopor),
        opadr(zpmopxor),
        opadr(zpmopshl),
        opadr(zpmopshr),
        opadr(zpmopsar),
        opadr(zpmoprol),
        opadr(zpmopror),
        opadr(zpmopinc),
        opadr(zpmopdec),
        opadr(zpmopadd),
        opadr(zpmopsub),
        opadr(zpmopcmp),
        opadr(zpmopmul),
        opadr(zpmopdiv),
        opadr(zpmopjmp),
        opadr(zpmopbz),
        opadr(zpmopbnz),
        opadr(zpmopblt),
        opadr(zpmopble),
        opadr(zpmopbgt),
        opadr(zpmopbge),
        opadr(zpmopbo),
        opadr(zpmopbno),
        opadr(zpmopbc),
        opadr(zpmopbnc),
        opadr(zpmoppop),
        opadr(zpmoppopa),
        opadr(zpmoppush),
        opadr(zpmoppusha),
        opadr(zpmoplda),
        opadr(zpmopsta),
        opadr(zpmopcall),
        opadr(zpmopenter),
        opadr(zpmopleave),
        opadr(zpmopret),
        opadr(zpmopthr),
        opadr(zpmopltb),
        opadr(zpmopldr),
        opadr(zpmopstr),
        opadr(zpmoprst),
        opadr(zpmophlt),
        opadr(zpmopin),
        opadr(zpmopout)
    };

#if !defined(__GNUC__)

    while ((pc) && pc != ZPM_PC_INVAL) {
        uint8_t   *text = &vm->mem[pc];
        uint8_t    code = inst->code;
        zpmopfunc *func = jmtab[code];
        
        pc = func(vm, text, pc);
    }

#else /* !defined(__GNUC__) */

    uint8_t *text = &vm->mem[pc];
    uint8_t *op = text;

    opjmp(pc);
    while ((pc) && pc != ZPM_PC_INVAL) {
        zpmopnot:
            pc = zpmnot(vm, op, pc);

            opjmp(pc);
        zpmopand:
            pc = zpmand(vm, op, pc);

            opjmp(pc);
        zpmopor:
            pc = zpmor(vm, op, pc);

            opjmp(pc);
        zpmopxor:
            pc = zpmxor(vm, op, pc);

            opjmp(pc);
        zpmopshl:
            pc = zpmshl(vm, op, pc);
            
            opjmp(pc);
        zpmopshr:
            pc = zpmshr(vm, op, pc);
            
            opjmp(pc);
        zpmopsar:
            pc = zpmsar(vm, op, pc);
            
            opjmp(pc);
        zpmoprol:
            pc = zpmrol(vm, op, pc);
            
            opjmp(pc);
        zpmopror:
            pc = zpmror(vm, op, pc);
            
            opjmp(pc);
        zpmopinc:
            pc = zpminc(vm, op, pc);
            
            opjmp(pc);
        zpmopdec:
            pc = zpmdec(vm, op, pc);
            
            opjmp(pc);
        zpmopadd:
            pc = zpmadd(vm, op, pc);
            
            opjmp(pc);
        zpmopsub:
            zpmsub(vm, op, pc);
            
            opjmp(pc);
        zpmopcmp:
            pc = zpmcmp(vm, op, pc);
            
            opjmp(pc);
        zpmopmul:
            pc = zpmmul(vm, op, pc);
            
            opjmp(pc);
        zpmopdiv:
            pc = zpmdiv(vm, op, pc);
            
            opjmp(pc);
        zpmopjmp:
            pc = zpmjmp(vm, op, pc);
            
            opjmp(pc);
        zpmopbz:
            pc = zpmbz(vm, op, pc);
            
            opjmp(pc);
        zpmopbnz:
            pc = zpmbnz(vm, op, pc);
            
            opjmp(pc);
        zpmopblt:
            pc = zpmblt(vm, op, pc);
            
            opjmp(pc);
        zpmopble:
            pc = zpmble(vm, op, pc);
            
            opjmp(pc);
        zpmopbgt:
            pc = zpmbgt(vm, op, pc);
            
            opjmp(pc);
        zpmopbge:
            pc = zpmbge(vm, op, pc);
            
            opjmp(pc);
        zpmopbo:
            pc = zpmbo(vm, op, pc);
            
            opjmp(pc);
        zpmopbno:
            pc = zpmbno(vm, op, pc);
            
            opjmp(pc);
        zpmopbc:
            pc = zpmbc(vm, op, pc);
            
            opjmp(pc);
        zpmopbnc:
            pc = zpmbnc(vm, op, pc);
            
            opjmp(pc);
        zpmoppop:
            pc = zpmpop(vm, op, pc);
            
            opjmp(pc);
        zpmoppopa:
            pc = zpmpopa(vm, op, pc);
            
            opjmp(pc);
        zpmoppush:
            pc = zpmpush(vm, op, pc);
            
            opjmp(pc);
        zpmoppusha:
            pc = zpmpusha(vm, op, pc);
            
            opjmp(pc);
        zpmoplda:
            pc = zpmlda(vm, op, pc);
            
            opjmp(pc);
        zpmopsta:
            pc = zpmsta(vm, op, pc);
            
            opjmp(pc);
        zpmopcall:
            pc = zpmcall(vm, op, pc);
            
            opjmp(pc);
        zpmopenter:
            pc = zpmenter(vm, op, pc);
            
            opjmp(pc);
        zpmopleave:
            pc = zpmleave(vm, op, pc);
            
            opjmp(pc);
        zpmopret:
            pc = zpmret(vm, op, pc);
            
            opjmp(pc);
        zpmopthr:
            pc = zpmthr(vm, op, pc);
            
            opjmp(pc);
        zpmopltb:
            pc = zpmltb(vm, op, pc);
            
            opjmp(pc);
        zpmopldr:
            pc = zpmldr(vm, op, pc);
            
            opjmp(pc);
        zpmopstr:
            pc = zpmstr(vm, op, pc);
            
            opjmp(pc);
        zpmoprst:
            pc = zpmrst(vm, op, pc);
            
            opjmp(pc);
        zpmophlt:
            pc = zpmhlt(vm, op, pc);
            
            opjmp(pc);
        zpmopin:
            pc = zpmin(vm, op, pc);
            
            opjmp(pc);
        zpmopout:
            pc = zpmout(vm, op, pc);
            
            opjmp(pc);
    }

#endif
    
    if (pc == ZPM_PC_OK) {

        return 0;
    } else {

        return 1;
    }

    /* NOTREACHED */
}

struct zpm *
zpmrun(int argc, char *argv[])
{

    return NULL;
}

/* FIXME */
int
main(int argc, char *argv[])
{
    struct zpm *vm = zpmrun(argc, argv);
    zpmureg     pc = ZPM_TEXT_ADR;
    int         ret;

    if (vm) {
        ret = zpmloop(vm, pc);
    }

    exit(ret);
}

