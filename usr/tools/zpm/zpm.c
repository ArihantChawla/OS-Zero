#include <zpm/zpm.h>
#include <zpm/op.h>

#define opjmp(op) goto *jmptab[((struct zpmop *)&vm->mem[pc])->code]

#if defined(__GNUC__)

/* this version of zpmloop uses GCC computed gotos to dispatch VM operations */

int
zpmloop(struct zpm *vm, zpmureg pc)
{
    static void *jmptab[] = {
        &&donot,
        &&doand,
        &&door,
        &&doxor,
        &&doshl,
        &&doshr,
        &&dosar,
        &&dorol,
        &&doror,
        &&doinc,
        &&dodec,
        &&doadd,
        &&dosub,
        &&docmp,
        &&domul,
        &&dodiv,
        &&dojmp,
        &&dobz,
        &&dobnz,
        &&doblt,
        &&doble,
        &&dobgt,
        &&dobge,
        &&dobo,
        &&dobno,
        &&dobc,
        &&dobnc,
        &&dopop,
        &&dopush,
        &&dopusha,
        &&dolda,
        &&dosta,
        &&docall,
        &&doenter,
        &&doleave,
        &&doret,
        &&dothr,
        &&doltb,
        &&doldr,
        &&dostr,
        &&dorst,
        &&dohlt,
        &&doin,
        &&doout
    };
    uint8_t *text = &vm->mem[pc];
    uint8_t *op = text;

    opjmp(pc);
    while (pc != ~(zpmureg)0) {
        donot:
            pc = zpmnot(vm, op, pc);

            opjmp(pc);
        doand:
            pc = zpmand(vm, op, pc);

            opjmp(pc);
        door:
            pc = zpmor(vm, op, pc);

            opjmp(pc);
        doxor:
            pc = zpmxor(vm, op, pc);

            opjmp(pc);
        doshl:
            pc = zpmshl(vm, op, pc);
            
            opjmp(pc);
        doshr:
            pc = zpmshr(vm, op, pc);
            
            opjmp(pc);
        dosar:
            pc = zpmsar(vm, op, pc);
            
            opjmp(pc);
        dorol:
            pc = zpmrol(vm, op, pc);
            
            opjmp(pc);
        doror:
            pc = zpmror(vm, op, pc);
            
            opjmp(pc);
        doinc:
            pc = zpminc(vm, op, pc);
            
            opjmp(pc);
        dodec:
            pc = zpmdec(vm, op, pc);
            
            opjmp(pc);
        doadd:
            pc = zpmadd(vm, op, pc);
            
            opjmp(pc);
        dosub:
            zpmsub(vm, op, pc);
            
            opjmp(pc);
        docmp:
            pc = zpmcmp(vm, op, pc);
            
            opjmp(pc);
        domul:
            pc = zpmmul(vm, op, pc);
            
            opjmp(pc);
        dodiv:
            pc = zpmdiv(vm, op, pc);
            
            opjmp(pc);
        dojmp:
            pc = zpmjmp(vm, op, pc);
            
            opjmp(pc);
        dobz:
            pc = zpmbz(vm, op, pc);
            
            opjmp(pc);
        dobnz:
            pc = zpmbnz(vm, op, pc);
            
            opjmp(pc);
        doblt:
            pc = zpmblt(vm, op, pc);
            
            opjmp(pc);
        doble:
            pc = zpmble(vm, op, pc);
            
            opjmp(pc);
        dobgt:
            pc = zpmbgt(vm, op, pc);
            
            opjmp(pc);
        dobge:
            pc = zpmbge(vm, op, pc);
            
            opjmp(pc);
        dobo:
            pc = zpmbo(vm, op, pc);
            
            opjmp(pc);
        dobno:
            pc = zpmbno(vm, op, pc);
            
            opjmp(pc);
        dobc:
            pc = zpmbc(vm, op, pc);
            
            opjmp(pc);
        dobnc:
            pc = zpmbnc(vm, op, pc);
            
            opjmp(pc);
        dopop:
            pc = zpmpop(vm, op, pc);
            
            opjmp(pc);
        dopopa:
            pc = zpmpopa(vm, op, pc);
            
            opjmp(pc);
        dopush:
            pc = zpmpush(vm, op, pc);
            
            opjmp(pc);
        dopusha:
            pc = zpmpusha(vm, op, pc);
            
            opjmp(pc);
        dolda:
            pc = zpmlda(vm, op, pc);
            
            opjmp(pc);
        dosta:
            pc = zpmsta(vm, op, pc);
            
            opjmp(pc);
        docall:
            pc = zpmcall(vm, op, pc);
            
            opjmp(pc);
        doenter:
            pc = zpmenter(vm, op, pc);
            
            opjmp(pc);
        doleave:
            pc = zpmleave(vm, op, pc);
            
            opjmp(pc);
        doret:
            pc = zpmret(vm, op, pc);
            
            opjmp(pc);
        dothr:
            pc = zpmthr(vm, op, pc);
            
            opjmp(pc);
        doltb:
            pc = zpmltb(vm, op, pc);
            
            opjmp(pc);
        doldr:
            pc = zpmldr(vm, op, pc);
            
            opjmp(pc);
        dostr:
            pc = zpmstr(vm, op, pc);
            
            opjmp(pc);
        dorst:
            pc = zpmrst(vm, op, pc);
            
            opjmp(pc);
        dohlt:
            pc = zpmhlt(vm, op, pc);
            
            opjmp(pc);
        doin:
            pc = zpmin(vm, op, pc);
            
            opjmp(pc);
        doout:
            pc = zpmout(vm, op, pc);
            
            opjmp(pc);
    }

    /* NOTREACHED */
}

#endif /* __GNUC__ */

