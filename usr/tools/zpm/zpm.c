#include <stdlib.h>
#include <zpm/zpm.h>
#include <zpm/op.h>

#if defined(__GNUC__)
#define opadr(x) &&zpmop##x
#define OPTAB_T  void *
#else
#define opadr(x) &zpm##x
typedef zpmureg  zpmopfunc(struct zpm *vm, uint8_t *ptr, zpmureg pc);
#define OPTAB_T  zpmopfunc *
#endif

#define opjmp(op) goto *jmptab[((struct zpmop *)&vm->mem[pc])->code]
#define opset(unit, inst, func, tab)                                    \
    do {                                                                \
        ((OPTAB_T *)tab)[zpmmkopid(unit, inst)] = (func);               \
    } while (0)

#define zpminitops(tab)                                                 \
    do {                                                                \
        opset(ZPM_NO_UNIT, ZPM_NO_INST, opadr(nop), tab);               \
        opset(ZPM_LOGIC, ZPM_NOT, opadr(not), tab);                     \
        opset(ZPM_LOGIC, ZPM_AND, opadr(and), tab);                     \
        opset(ZPM_LOGIC, ZPM_OR, opadr(or), tab);                       \
        opset(ZPM_LOGIC, ZPM_XOR, opadr(xor), tab);                     \
        opset(ZPM_SHIFTER, ZPM_SHL, opadr(shl), tab);                   \
        opset(ZPM_SHIFTER, ZPM_SHR, opadr(shr), tab);                   \
        opset(ZPM_SHIFTER, ZPM_SAR, opadr(sar), tab);                   \
        opset(ZPM_SHIFTER, ZPM_ROL, opadr(rol), tab);                   \
        opset(ZPM_SHIFTER, ZPM_ROR, opadr(ror), tab);                   \
        opset(ZPM_ARITH, ZPM_INC, opadr(inc), tab);                     \
        opset(ZPM_ARITH, ZPM_DEC, opadr(dec), tab);                     \
        opset(ZPM_ARITH, ZPM_ADD, opadr(add), tab);                     \
        opset(ZPM_ARITH, ZPM_ADC, opadr(adc), tab);                     \
        opset(ZPM_ARITH, ZPM_ADI, opadr(adi), tab);                     \
        opset(ZPM_ARITH, ZPM_SUB, opadr(sub), tab);                     \
        opset(ZPM_ARITH, ZPM_SBC, opadr(sbc), tab);                     \
        opset(ZPM_ARITH, ZPM_CMP, opadr(cmp), tab);                     \
        opset(ZPM_MULTIPLIER, ZPM_MUL, opadr(mul), tab);                \
        opset(ZPM_DIVIDER, ZPM_DIV, opadr(div), tab);                   \
        opset(ZPM_DIVIDER, ZPM_REM, opadr(rem), tab);                   \
        opset(ZPM_LOAD_STORE, ZPM_LDA, opadr(lda), tab);                \
        opset(ZPM_LOAD_STORE, ZPM_STA, opadr(sta), tab);                \
        opset(ZPM_STACK, ZPM_PSH, opadr(psh), tab);                     \
        opset(ZPM_STACK, ZPM_PSHA, opadr(psha), tab);                   \
        opset(ZPM_STACK, ZPM_POP, opadr(pop), tab);                     \
        opset(ZPM_STACK, ZPM_POPA, opadr(popa), tab);                   \
        opset(ZPM_IO, ZPM_IN, opadr(in), tab);                          \
        opset(ZPM_IO, ZPM_OUT, opadr(out), tab);                        \
        opset(ZPM_FLOW, ZPM_JMP, opadr(jmp), tab);                      \
        opset(ZPM_FLOW, ZPM_BZ, opadr(bz), tab);                        \
        opset(ZPM_FLOW, ZPM_BNZ, opadr(bnz), tab);                      \
        opset(ZPM_FLOW, ZPM_BLT, opadr(blt), tab);                      \
        opset(ZPM_FLOW, ZPM_BLE, opadr(ble), tab);                      \
        opset(ZPM_FLOW, ZPM_BGT, opadr(bgt), tab);                      \
        opset(ZPM_FLOW, ZPM_BGE, opadr(bge), tab);                      \
        opset(ZPM_FLOW, ZPM_BO, opadr(bo), tab);                        \
        opset(ZPM_FLOW, ZPM_BNO, opadr(bno), tab);                      \
        opset(ZPM_FLOW, ZPM_BC, opadr(bc), tab);                        \
        opset(ZPM_FLOW, ZPM_BNC, opadr(bnc), tab);                      \
        opset(ZPM_FLOW, ZPM_CALL, opadr(call), tab);                    \
        opset(ZPM_FLOW, ZPM_THR, opadr(thr), tab);                      \
        opset(ZPM_FLOW, ZPM_ENTER, opadr(enter), tab);                  \
        opset(ZPM_FLOW, ZPM_LEAVE, opadr(leave), tab);                  \
        opset(ZPM_FLOW, ZPM_RET, opadr(ret), tab);                      \
        opset(ZPM_SYS, ZPM_LDR, opadr(ldr), tab);                       \
        opset(ZPM_SYS, ZPM_STR, opadr(str), tab);                       \
        opset(ZPM_SYS, ZPM_RST, opadr(rst), tab);                       \
        opset(ZPM_SYS, ZPM_HLT, opadr(hlt), tab);                       \
    } while (0)

int
zpmloop(struct zpm *vm, zpmureg pc)
{
    static OPTAB_T  jmptab[ZPM_NALU_MAX];
#if defined(__GNUC__)
    uint8_t        *text = &vm->mem[pc];
    uint8_t        *op = text;
#endif

    zpminitops(jmptab);

#if !defined(__GNUC__)

    while ((pc) && pc != ZPM_PC_INVAL) {
        uint8_t   *text = &vm->mem[pc];
        uint8_t    code = inst->code;
        zpmopfunc *func = jmptab[code];
        
        pc = func(vm, text, pc);
    }

#else /* defined(__GNUC__) */

    opjmp(pc);
    while ((pc) && pc != ZPM_PC_INVAL) {
        zpmopnop:
            pc = zpmnop(vm, op, pc);

            opjmp(pc);
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
        zpmopadc:
            pc = zpmadc(vm, op, pc);
            
            opjmp(pc);
        zpmopadi:
            pc = zpmadi(vm, op, pc);
            
            opjmp(pc);
        zpmopsub:
            zpmsub(vm, op, pc);
            
            opjmp(pc);
        zpmopsbc:
            zpmsbc(vm, op, pc);
            
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
        zpmoprem:
            pc = zpmrem(vm, op, pc);
            
            opjmp(pc);
        zpmoplda:
            pc = zpmlda(vm, op, pc);
            
            opjmp(pc);
        zpmopsta:
            pc = zpmsta(vm, op, pc);
            
            opjmp(pc);
        zpmoppsh:
            pc = zpmpsh(vm, op, pc);
            
            opjmp(pc);
        zpmoppsha:
            pc = zpmpsha(vm, op, pc);
            
            opjmp(pc);
        zpmoppop:
            pc = zpmpop(vm, op, pc);
            
            opjmp(pc);
        zpmoppopa:
            pc = zpmpopa(vm, op, pc);
            
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
        zpmopcall:
            pc = zpmcall(vm, op, pc);
            
            opjmp(pc);
        zpmopthr:
            pc = zpmthr(vm, op, pc);
            
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

