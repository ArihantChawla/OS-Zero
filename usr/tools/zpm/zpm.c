#define opjmp(op) goto *jmptab[(struct zpmop *)op->code]


#if defined(__GNUC__)

/* this version of zpmloop uses GCC computed gotos to dispatch VM operations */

int
zpmloop(struct zpm *vm)
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
    uint8_t *text = &vm->mem[vm->segs[ZPM_TEXT]];
    uint8_t *op = text;

    opjmp(op);
    while (op) {
        donot:
            op = zpmnot(vm, op, pc);

            opjmp(op);
        doand:
            op = zpmand(vm, op, pc);

            opjmp(op);
        door:
            op = zpmor(vm, op, pc);

            opjmp(op);
        doxor:
            op = zpmxor(vm, op, pc);

            opjmp(op);
        doshl:
            op = zpmshl(vm, op, pc);
            
            opjmp(op);
        doshr:
            op = zpmshr(vm, op, pc);
            
            opjmp(op);
        dosar:
            op = zpmsar(vm, op, pc);
            
            opjmp(op);
        dorol:
            op = zpmrol(vm, op, pc);
            
            opjmp(op);
        doror:
            op = zpmror(vm, op, pc);
            
            opjmp(op);
        doinc:
            op = zpminc(vm, op, pc);
            
            opjmp(op);
        dodec:
            op = zpmdec(vm, op, pc);
            
            opjmp(op);
        doadd:
            op = zpmadd(vm, op, pc);
            
            opjmp(op);
        dosub:
            zpsub(vm, op, pc);
            
            opjmp(op);
        docmp:
            op = zpmcmp(vm, op, pc);
            
            opjmp(op);
        domul:
            op = zpmmul(vm, op, pc);
            
            opjmp(op);
        dodiv:
            op = zpmdiv(vm, op, pc);
            
            opjmp(op);
        dojmp:
            op = zpmjmp(vm, op, pc);
            
            opjmp(op);
        dobz:
            op = zpmbz(vm, op, pc);
            
            opjmp(op);
        dobnz:
            op = zpmbnz(vm, op, pc);
            
            opjmp(op);
        doblt:
            op = zpmblt(vm, op, pc);
            
            opjmp(op);
        doble:
            op = zpmble(vm, op, pc);
            
            opjmp(op);
        dobgt:
            op = zpmbgt(vm, op, pc);
            
            opjmp(op);
        dobge:
            op = zpmbge(vm, op, pc);
            
            opjmp(op);
        dobo:
            op = zpmbo(vm, op, pc);
            
            opjmp(op);
        dobno:
            op = zpmbno(vm, op, pc);
            
            opjmp(op);
        dobc:
            op = zpmbc(vm, op, pc);
            
            opjmp(op);
        dobnc:
            op = zpmbnc(vm, op, pc);
            
            opjmp(op);
        dopop:
            op = zpmpop(vm, op, pc);
            
            opjmp(op);
        dopush:
            op = zpmpush(vm, op, pc);
            
            opjmp(op);
        dopusha:
            op = zpmpusha(vm, op, pc);
            
            opjmp(op);
        dolda:
            op = zpmlda(vm, op, pc);
            
            opjmp(op);
        dosta:
            op = zpmsta(vm, op, pc);
            
            opjmp(op);
        docall:
            op = zpmcall(vm, op, pc);
            
            opjmp(op);
        doenter:
            op = zpmenter(vm, op, pc);
            
            opjmp(op);
        doleave:
            op = zpmleave(vm, op, pc);
            
            opjmp(op);
        doret:
            op = zpmret(vm, op, pc);
            
            opjmp(op);
        dothr:
            op = zpmthr(vm, op, pc);
            
            opjmp(op);
        doltb:
            op = zpmltb(vm, op, pc);
            
            opjmp(op);
        doldr:
            op = zpmldr(vm, op, pc);
            
            opjmp(op);
        dostr:
            op = zpmstr(vm, op, pc);
            
            opjmp(op);
        dorst:
            op = zpmrst(vm, op, pc);
            
            opjmp(op);
        dohlt:
            op = zpmhlt(vm, op, pc);
            
            opjmp(op);
        doin:
            op = zpmin(vm, op, pc);
            
            opjmp(op);
        doout:
            op = zpmout(vm, op, pc);
            
            opjmp(op);
    }

    /* NOTREACHED */
}

#endif /* __GNUC__ */

