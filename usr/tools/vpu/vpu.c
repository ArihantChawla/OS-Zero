#include <stdlib.h>
#include <vpu/vpu.h>
#include <vpu/op.h>

#if defined(__GNUC__)
#define opadr(x) &&vpuop##x
#define OPTAB_T  void *
#else
#define opadr(x) &vpu##x
typedef vpuureg  vpuopfunc(struct vpu *vm, uint8_t *ptr, vpuureg pc);
#define OPTAB_T  vpuopfunc *
#endif

#define opjmp(op) goto *jmptab[((struct vpuop *)&vm->mem[pc])->code]
#define opset(unit, inst, func, tab)                                    \
    do {                                                                \
        ((OPTAB_T *)tab)[vpumkopid(unit, inst)] = (func);               \
    } while (0)

#define vpuinitops(tab)                                                 \
    do {                                                                \
        opset(VPU_NO_UNIT, VPU_NO_INST, opadr(nop), tab);               \
        opset(VPU_LOGIC, VPU_NOT, opadr(not), tab);                     \
        opset(VPU_LOGIC, VPU_AND, opadr(and), tab);                     \
        opset(VPU_LOGIC, VPU_OR, opadr(or), tab);                       \
        opset(VPU_LOGIC, VPU_XOR, opadr(xor), tab);                     \
        opset(VPU_SHIFTER, VPU_SHL, opadr(shl), tab);                   \
        opset(VPU_SHIFTER, VPU_SHR, opadr(shr), tab);                   \
        opset(VPU_SHIFTER, VPU_SAR, opadr(sar), tab);                   \
        opset(VPU_SHIFTER, VPU_ROL, opadr(rol), tab);                   \
        opset(VPU_SHIFTER, VPU_ROR, opadr(ror), tab);                   \
        opset(VPU_ARITH, VPU_INC, opadr(inc), tab);                     \
        opset(VPU_ARITH, VPU_DEC, opadr(dec), tab);                     \
        opset(VPU_ARITH, VPU_ADD, opadr(add), tab);                     \
        opset(VPU_ARITH, VPU_ADC, opadr(adc), tab);                     \
        opset(VPU_ARITH, VPU_ADI, opadr(adi), tab);                     \
        opset(VPU_ARITH, VPU_SUB, opadr(sub), tab);                     \
        opset(VPU_ARITH, VPU_SBC, opadr(sbc), tab);                     \
        opset(VPU_ARITH, VPU_CMP, opadr(cmp), tab);                     \
        opset(VPU_MULTIPLIER, VPU_MUL, opadr(mul), tab);                \
        opset(VPU_DIVIDER, VPU_DIV, opadr(div), tab);                   \
        opset(VPU_DIVIDER, VPU_REM, opadr(rem), tab);                   \
        opset(VPU_LOAD_STORE, VPU_LDA, opadr(lda), tab);                \
        opset(VPU_LOAD_STORE, VPU_STA, opadr(sta), tab);                \
        opset(VPU_STACK, VPU_PSH, opadr(psh), tab);                     \
        opset(VPU_STACK, VPU_PSHA, opadr(psha), tab);                   \
        opset(VPU_STACK, VPU_POP, opadr(pop), tab);                     \
        opset(VPU_STACK, VPU_POPA, opadr(popa), tab);                   \
        opset(VPU_IO, VPU_IN, opadr(in), tab);                          \
        opset(VPU_IO, VPU_OUT, opadr(out), tab);                        \
        opset(VPU_FLOW, VPU_JMP, opadr(jmp), tab);                      \
        opset(VPU_FLOW, VPU_BZ, opadr(bz), tab);                        \
        opset(VPU_FLOW, VPU_BNZ, opadr(bnz), tab);                      \
        opset(VPU_FLOW, VPU_BLT, opadr(blt), tab);                      \
        opset(VPU_FLOW, VPU_BLE, opadr(ble), tab);                      \
        opset(VPU_FLOW, VPU_BGT, opadr(bgt), tab);                      \
        opset(VPU_FLOW, VPU_BGE, opadr(bge), tab);                      \
        opset(VPU_FLOW, VPU_BO, opadr(bo), tab);                        \
        opset(VPU_FLOW, VPU_BNO, opadr(bno), tab);                      \
        opset(VPU_FLOW, VPU_BC, opadr(bc), tab);                        \
        opset(VPU_FLOW, VPU_BNC, opadr(bnc), tab);                      \
        opset(VPU_FLOW, VPU_CALL, opadr(call), tab);                    \
        opset(VPU_FLOW, VPU_THR, opadr(thr), tab);                      \
        opset(VPU_FLOW, VPU_ENTER, opadr(enter), tab);                  \
        opset(VPU_FLOW, VPU_LEAVE, opadr(leave), tab);                  \
        opset(VPU_FLOW, VPU_RET, opadr(ret), tab);                      \
        opset(VPU_SYS, VPU_LDR, opadr(ldr), tab);                       \
        opset(VPU_SYS, VPU_STR, opadr(str), tab);                       \
        opset(VPU_SYS, VPU_RST, opadr(rst), tab);                       \
        opset(VPU_SYS, VPU_HLT, opadr(hlt), tab);                       \
    } while (0)

int
vpuloop(struct vpu *vm, vpuureg pc)
{
    static OPTAB_T  jmptab[VPU_NALU_MAX];
#if defined(__GNUC__)
    uint8_t        *text = &vm->mem[pc];
    uint8_t        *op = text;
#endif

    vpuinitops(jmptab);

#if !defined(__GNUC__)

    while ((pc) && pc != VPU_PC_INVAL) {
        uint8_t   *text = &vm->mem[pc];
        uint8_t    code = inst->code;
        vpuopfunc *func = jmptab[code];
        
        pc = func(vm, text, pc);
    }

#else /* defined(__GNUC__) */

    opjmp(pc);
    while ((pc) && pc != VPU_PC_INVAL) {
        vpuopnop:
            pc = vpunop(vm, op, pc);

            opjmp(pc);
        vpuopnot:
            pc = vpunot(vm, op, pc);

            opjmp(pc);
        vpuopand:
            pc = vpuand(vm, op, pc);

            opjmp(pc);
        vpuopor:
            pc = vpuor(vm, op, pc);

            opjmp(pc);
        vpuopxor:
            pc = vpuxor(vm, op, pc);

            opjmp(pc);
        vpuopshl:
            pc = vpushl(vm, op, pc);
            
            opjmp(pc);
        vpuopshr:
            pc = vpushr(vm, op, pc);
            
            opjmp(pc);
        vpuopsar:
            pc = vpusar(vm, op, pc);
            
            opjmp(pc);
        vpuoprol:
            pc = vpurol(vm, op, pc);
            
            opjmp(pc);
        vpuopror:
            pc = vpuror(vm, op, pc);
            
            opjmp(pc);
        vpuopinc:
            pc = vpuinc(vm, op, pc);
            
            opjmp(pc);
        vpuopdec:
            pc = vpudec(vm, op, pc);
            
            opjmp(pc);
        vpuopadd:
            pc = vpuadd(vm, op, pc);
            
            opjmp(pc);
        vpuopadc:
            pc = vpuadc(vm, op, pc);
            
            opjmp(pc);
        vpuopadi:
            pc = vpuadi(vm, op, pc);
            
            opjmp(pc);
        vpuopsub:
            vpusub(vm, op, pc);
            
            opjmp(pc);
        vpuopsbc:
            vpusbc(vm, op, pc);
            
            opjmp(pc);
        vpuopcmp:
            pc = vpucmp(vm, op, pc);
            
            opjmp(pc);
        vpuopmul:
            pc = vpumul(vm, op, pc);
            
            opjmp(pc);
        vpuopdiv:
            pc = vpudiv(vm, op, pc);
            
            opjmp(pc);
        vpuoprem:
            pc = vpurem(vm, op, pc);
            
            opjmp(pc);
        vpuoplda:
            pc = vpulda(vm, op, pc);
            
            opjmp(pc);
        vpuopsta:
            pc = vpusta(vm, op, pc);
            
            opjmp(pc);
        vpuoppsh:
            pc = vpupsh(vm, op, pc);
            
            opjmp(pc);
        vpuoppsha:
            pc = vpupsha(vm, op, pc);
            
            opjmp(pc);
        vpuoppop:
            pc = vpupop(vm, op, pc);
            
            opjmp(pc);
        vpuoppopa:
            pc = vpupopa(vm, op, pc);
            
            opjmp(pc);
        vpuopjmp:
            pc = vpujmp(vm, op, pc);
            
            opjmp(pc);
        vpuopbz:
            pc = vpubz(vm, op, pc);
            
            opjmp(pc);
        vpuopbnz:
            pc = vpubnz(vm, op, pc);
            
            opjmp(pc);
        vpuopblt:
            pc = vpublt(vm, op, pc);
            
            opjmp(pc);
        vpuopble:
            pc = vpuble(vm, op, pc);
            
            opjmp(pc);
        vpuopbgt:
            pc = vpubgt(vm, op, pc);
            
            opjmp(pc);
        vpuopbge:
            pc = vpubge(vm, op, pc);
            
            opjmp(pc);
        vpuopbo:
            pc = vpubo(vm, op, pc);
            
            opjmp(pc);
        vpuopbno:
            pc = vpubno(vm, op, pc);
            
            opjmp(pc);
        vpuopbc:
            pc = vpubc(vm, op, pc);
            
            opjmp(pc);
        vpuopbnc:
            pc = vpubnc(vm, op, pc);
            
            opjmp(pc);
        vpuopcall:
            pc = vpucall(vm, op, pc);
            
            opjmp(pc);
        vpuopthr:
            pc = vputhr(vm, op, pc);
            
            opjmp(pc);
        vpuopenter:
            pc = vpuenter(vm, op, pc);
            
            opjmp(pc);
        vpuopleave:
            pc = vpuleave(vm, op, pc);
            
            opjmp(pc);
        vpuopret:
            pc = vpuret(vm, op, pc);
            
            opjmp(pc);
        vpuopldr:
            pc = vpuldr(vm, op, pc);
            
            opjmp(pc);
        vpuopstr:
            pc = vpustr(vm, op, pc);
            
            opjmp(pc);
        vpuoprst:
            pc = vpurst(vm, op, pc);
            
            opjmp(pc);
        vpuophlt:
            pc = vpuhlt(vm, op, pc);
            
            opjmp(pc);
        vpuopin:
            pc = vpuin(vm, op, pc);
            
            opjmp(pc);
        vpuopout:
            pc = vpuout(vm, op, pc);
            
            opjmp(pc);
    }

#endif
    
    if (pc == VPU_PC_OK) {

        return 0;
    } else {

        return 1;
    }

    /* NOTREACHED */
}

struct vpu *
vpurun(int argc, char *argv[])
{

    return NULL;
}

/* FIXME */
int
main(int argc, char *argv[])
{
    struct vpu *vm = vpurun(argc, argv);
    vpuureg     pc = VPU_TEXT_ADR;
    int         ret;

    if (vm) {
        ret = vpuloop(vm, pc);
    }

    exit(ret);
}

