#include <v0/conf.h>
#include <stdlib.h>
#include <string.h>
#include <zero/fastudiv.h>
#include <v0/vm32.h>
#include <v0/mach.h>
#include <v0/op.h>

extern void vminitio(struct v0 *vm);

#if defined(__GNUC__)
#define opadr(x) &&v0op##x
#define OPTAB_T  void *
#else
#define opadr(x) &v0##x
typedef v0reg    v0opfunc(struct v0 *vm, uint8_t *ptr, v0ureg pc);
#define OPTAB_T  v0opfunc *
#endif

#if defined(V0_DEBUG_TABS)
static struct v0opinfo v0opinfotab[V0_NINST_MAX];
#endif

void
v0printop(struct v0op *op)
{
    long val = op->code;

    fprintf(stderr, "code\t%x - unit == %x, inst == %x\n",
            val, v0getunit(val), v0getinst(val));
}

#if defined(V0_DEBUG_TABS)
#define opaddtab(proc, inst, handler)                                   \
    do {                                                                \
        long _code = v0mkopid(proc, inst);                              \
                                                                        \
        v0opinfotab[_code].unit = strdup(#proc);                        \
        v0opinfotab[_code].op = strdup(#inst);                          \
        v0opinfotab[_code].func = strdup(#handler);                     \
    } while (0)
#else
#define opaddtab(unit, op)
#endif

#define opjmp(op)                                                       \
    do {                                                                \
        if (pc <= vm->seglims[V0_TEXT_SEG]) {                           \
            goto *jmptab[((struct v0op *)&vm->mem[pc])->code];          \
        } else {                                                        \
            v0doxcpt(V0_TEXT_FAULT);                                    \
        }                                                               \
    } while (0)
#define opset(unit, op, func, tab)                                      \
    do {                                                                \
        long _code = v0mkopid(unit, op);                                \
                                                                        \
        opaddtab(unit, op, func);                                       \
        ((OPTAB_T *)tab)[_code] = (func);                               \
    } while (0)

#if !defined(__GNUC__)

#define v0initops(tab)                                                  \
    do {                                                                \
        opset(V0_BITS, V0_NOT, opadr(not), tab);                        \
	opset(V0_BITS, V0_AND, opadr(and), tab);                        \
	opset(V0_BITS, V0_OR, opadr(or), tab);                          \
	opset(V0_BITS, V0_XOR, opadr(xor), tab);                        \
	opset(V0_SHIFT, V0_SHL, opadr(shl), tab);                       \
	opset(V0_SHIFT, V0_SHR, opadr(shr), tab);                       \
	opset(V0_SHIFT, V0_SAR, opadr(sar), tab);                       \
	opset(V0_ARITH, V0_INC, opadr(inc), tab);                       \
	opset(V0_ARITH, V0_DEC, opadr(dec), tab);                       \
	opset(V0_ARITH, V0_ADD, opadr(add), tab);                       \
	opset(V0_ARITH, V0_ADC, opadr(adc), tab);                       \
	opset(V0_ARITH, V0_SUB, opadr(sub), tab);                       \
	opset(V0_ARITH, V0_SBB, opadr(sbb), tab);                       \
	opset(V0_ARITH, V0_CMP, opadr(cmp), tab);                       \
	opset(V0_ARITH, V0_MUL, opadr(mul), tab);                       \
	opset(V0_ARITH, V0_DIV, opadr(div), tab);                       \
	opset(V0_ARITH, V0_REM, opadr(rem), tab);                       \
	opset(V0_FLOW, V0_JMP, opadr(jmp), tab);                        \
	opset(V0_FLOW, V0_CALL, opadr(call), tab);                      \
	opset(V0_FLOW, V0_RET, opadr(ret), tab);                        \
	opset(V0_FLOW, V0_BZ, opadr(bz), tab);                          \
	opset(V0_FLOW, V0_BNZ, opadr(bnz), tab);                        \
	opset(V0_FLOW, V0_BC, opadr(bc), tab);                          \
	opset(V0_FLOW, V0_BNC, opadr(bnc), tab);                        \
	opset(V0_FLOW, V0_BO, opadr(bo), tab);                          \
	opset(V0_FLOW, V0_BNO, opadr(bno), tab);                        \
	opset(V0_FLOW, V0_BLT, opadr(blt), tab);                        \
	opset(V0_FLOW, V0_BLE, opadr(ble), tab);                        \
	opset(V0_FLOW, V0_BGT, opadr(bgt), tab);                        \
	opset(V0_FLOW, V0_BGE, opadr(bge), tab);                        \
	opset(V0_XFER, V0_LDR, opadr(ldr), tab);                        \
	opset(V0_XFER, V0_STR, opadr(str), tab);                        \
	opset(V0_STACK, V0_PSH, opadr(psh), tab);                       \
	opset(V0_STACK, V0_POP, opadr(pop), tab);                       \
	opset(V0_STACK, V0_PSHA, opadr(psha), tab);                     \
	opset(V0_STACK, V0_POPA, opadr(popa), tab);                     \
	opset(V0_STACK, V0_PSHM, opadr(pshm), tab);                     \
	opset(V0_STACK, V0_POPM, opadr(popm), tab);                     \
	opset(V0_IO, V0_IOR, opadr(ior), tab);                          \
	opset(V0_IO, V0_IOW, opadr(iow), tab);                          \
    } while (0)
#endif

struct v0 *
v0init(struct v0 *vm, v0ureg text)
{
    void *mem = malloc(V0_MEM_SIZE);
    void *ptr;
    long  newvm = 0;

    if (!mem) {

        return NULL;
    }
    ptr = calloc(V0_NIOPORT_MAX, sizeof(struct v0iofuncs));
    if (ptr) {
        if (!vm) {
            vm = calloc(1, sizeof(struct v0));
            if (!vm) {
                free(mem);
                free(ptr);

                return NULL;
            }
            newvm = 1;
        }
        memset(vm, 0, sizeof(struct v0));
        vm->iovec = ptr;
        ptr = calloc(65536, sizeof(struct divuf16));
        if (!ptr) {
            free(mem);
            free(vm->iovec);
            if (newvm) {
                free(vm);
            }

            return NULL;
        }
        fastuf16divuf16gentab(ptr, 0xffff);
        vm->divu16tab = ptr;
        vminitio(vm);
        vm->sysregs[V0_PC_REG] = text;
    }

    return vm;
}

int
v0loop(struct v0 *vm)
{
    static OPTAB_T  jmptab[V0_NINST_MAX];
    v0ureg          pc = vm->sysregs[V0_PC_REG];
#if defined(__GNUC__)
    static long     init = 0;
    uint8_t        *text = &vm->mem[pc];
    uint8_t        *op = text;
#endif

#if defined(__GNUC__)

    do {
        if (init) {
            while ((pc) && pc != V0_PC_INVAL) {
                opjmp(pc);
            }
        } else {
            goto v0opinit;

            v0opnot:
                pc = v0not(vm, op, pc);

                opjmp(pc);
            v0opand:
                pc = v0and(vm, op, pc);

                opjmp(pc);
            v0opor:
                pc = v0or(vm, op, pc);

                opjmp(pc);
            v0opxor:
                pc = v0xor(vm, op, pc);

                opjmp(pc);
            v0opshl:
                pc = v0shl(vm, op, pc);

                opjmp(pc);
            v0opshr:
                pc = v0shr(vm, op, pc);

                opjmp(pc);
            v0opsar:
                pc = v0sar(vm, op, pc);

                opjmp(pc);
            v0opinc:
                pc = v0inc(vm, op, pc);

                opjmp(pc);
            v0opdec:
                pc = v0dec(vm, op, pc);

                opjmp(pc);
            v0opadd:
                pc = v0add(vm, op, pc);

                opjmp(pc);
            v0opadc:
                pc = v0adc(vm, op, pc);

                opjmp(pc);
            v0opsub:
                pc = v0sub(vm, op, pc);

                opjmp(pc);
            v0opsbb:
                v0sbb(vm, op, pc);

                opjmp(pc);
            v0opcmp:
                pc = v0cmp(vm, op, pc);

                opjmp(pc);
            v0opmul:
                pc = v0mul(vm, op, pc);

                opjmp(pc);
            v0opdiv:
                pc = v0div(vm, op, pc);

                opjmp(pc);
            v0oprem:
                pc = v0rem(vm, op, pc);

                opjmp(pc);
#if 0
            v0opcrm:
                pc = v0crm(vm, op, pc);

                opjmp(pc);
#endif
            v0opjmp:
                pc = v0jmp(vm, op, pc);

                opjmp(pc);
            v0opcall:
                pc = v0call(vm, op, pc);

                opjmp(pc);
            v0opret:
                pc = v0ret(vm, op, pc);

                opjmp(pc);
            v0opbz:
                pc = v0bz(vm, op, pc);

                opjmp(pc);
            v0opbnz:
                pc = v0bnz(vm, op, pc);

                opjmp(pc);
            v0opbc:
                pc = v0bc(vm, op, pc);

                opjmp(pc);
            v0opbnc:
                pc = v0bnc(vm, op, pc);

                opjmp(pc);
            v0opbo:
                pc = v0bo(vm, op, pc);

                opjmp(pc);
            v0opbno:
                pc = v0bno(vm, op, pc);

                opjmp(pc);
            v0opblt:
                pc = v0blt(vm, op, pc);

                opjmp(pc);
            v0opble:
                pc = v0ble(vm, op, pc);

                opjmp(pc);
            v0opbgt:
                pc = v0bgt(vm, op, pc);

                opjmp(pc);
            v0opbge:
                pc = v0bge(vm, op, pc);

                opjmp(pc);
            v0opldr:
                pc = v0ldr(vm, op, pc);

                opjmp(pc);
            v0opstr:
                pc = v0str(vm, op, pc);

                opjmp(pc);
            v0oppsh:
                pc = v0psh(vm, op, pc);

                opjmp(pc);
            v0oppop:
                pc = v0pop(vm, op, pc);

                opjmp(pc);
            v0oppsha:
                pc = v0psha(vm, op, pc);

                opjmp(pc);
            v0oppopa:
                pc = v0popa(vm, op, pc);

                opjmp(pc);
            v0oppshm:
                pc = v0pshm(vm, op, pc);

                opjmp(pc);
            v0oppopm:
                pc = v0popm(vm, op, pc);

                opjmp(pc);
            v0opior:
                pc = v0ior(vm, op, pc);

                opjmp(pc);
            v0opiow:
                pc = v0iow(vm, op, pc);

                opjmp(pc);
            v0opinit:
                opset(V0_BITS, V0_NOT, opadr(not), jmptab);
                opset(V0_BITS, V0_AND, opadr(and), jmptab);
                opset(V0_BITS, V0_OR, opadr(or), jmptab);
                opset(V0_BITS, V0_XOR, opadr(xor), jmptab);
                opset(V0_SHIFT, V0_SHL, opadr(shl), jmptab);
                opset(V0_SHIFT, V0_SHR, opadr(shr), jmptab);
                opset(V0_SHIFT, V0_SAR, opadr(sar), jmptab);
                opset(V0_ARITH, V0_INC, opadr(inc), jmptab);
                opset(V0_ARITH, V0_DEC, opadr(dec), jmptab);
                opset(V0_ARITH, V0_ADD, opadr(add), jmptab);
                opset(V0_ARITH, V0_ADC, opadr(adc), jmptab);
                opset(V0_ARITH, V0_SUB, opadr(sub), jmptab);
                opset(V0_ARITH, V0_SBB, opadr(sbb), jmptab);
                opset(V0_ARITH, V0_CMP, opadr(cmp), jmptab);
                opset(V0_ARITH, V0_MUL, opadr(mul), jmptab);
                opset(V0_ARITH, V0_DIV, opadr(div), jmptab);
                opset(V0_ARITH, V0_REM, opadr(rem), jmptab);
                //                opset(V0_ARITH, V0_CRM, opadr(crm), jmptab);
                opset(V0_FLOW, V0_JMP, opadr(jmp), jmptab);
                opset(V0_FLOW, V0_CALL, opadr(call), jmptab);
                opset(V0_FLOW, V0_RET, opadr(ret), jmptab);
                opset(V0_FLOW, V0_BZ, opadr(bz), jmptab);
                opset(V0_FLOW, V0_BNZ, opadr(bnz), jmptab);
                opset(V0_FLOW, V0_BO, opadr(bc), jmptab);
                opset(V0_FLOW, V0_BNO, opadr(bnc), jmptab);
                opset(V0_FLOW, V0_BO, opadr(bo), jmptab);
                opset(V0_FLOW, V0_BNO, opadr(bno), jmptab);
                opset(V0_FLOW, V0_BLT, opadr(blt), jmptab);
                opset(V0_FLOW, V0_BLE, opadr(ble), jmptab);
                opset(V0_FLOW, V0_BGT, opadr(bgt), jmptab);
                opset(V0_FLOW, V0_BGE, opadr(bge), jmptab);
                opset(V0_XFER, V0_LDR, opadr(ldr), jmptab);
                opset(V0_XFER, V0_STR, opadr(str), jmptab);
                opset(V0_STACK, V0_PSH, opadr(psh), jmptab);
                opset(V0_STACK, V0_POP, opadr(pop), jmptab);
                opset(V0_STACK, V0_PSHA, opadr(psha), jmptab);
                opset(V0_STACK, V0_POPA, opadr(popa), jmptab);
                opset(V0_STACK, V0_PSHM, opadr(pshm), jmptab);
                opset(V0_STACK, V0_POPM, opadr(popm), jmptab);
                //                opset(V0_IO, V0_IOC, opadr(ioc), jmptab);
                opset(V0_IO, V0_IOR, opadr(ior), jmptab);
                opset(V0_IO, V0_IOW, opadr(iow), jmptab);
                init = 1;

                break;
        }
    } while (1);

#else /* !defined(__GNUC__) */

    while ((pc) && pc != V0_PC_INVAL) {
        uint8_t   *text = &vm->mem[pc];
        uint8_t    code = op->code;
        v0opfunc *func = jmptab[code];

        pc = func(vm, text, pc);
    }

#endif

    return EXIT_SUCCESS;
}

void
v0getopt(struct v0 *vm, int argc, char *argv[])
{
    return;
}

int
main(int argc, char *argv[])
{
    struct v0 *vm = v0init(NULL, V0_TEXT_ADR);
    int        ret;

    if (vm) {
        v0getopt(vm, argc, argv);
        ret = v0loop(vm);
    }

    exit(ret);
}

