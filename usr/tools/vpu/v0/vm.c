#include <v0/conf.h>
#include <stdlib.h>
#include <string.h>
#include <zero/fastudiv.h>
#include <v0/mach.h>
#include <v0/vm32.h>
#include <v0/op.h>
#include <v0/vm.h>

#if defined(__GNUC__)
#define _v0opadr(x) &&v0op##x
#define _V0OPTAB_T  void *
#else
#define _v0opadr(x) v0##x
typedef v0reg       v0opfunc(struct v0 *vm, uint8_t *ptr, v0ureg pc);
#define _V0OPTAB_T  v0opfunc *
#endif

#if defined(V0_DEBUG_TABS)
static struct    v0opinfo v0opinfotab[V0_NINST_MAX];
#endif

struct v0       *v0vm;

void
v0printop(struct v0op *op)
{
    int val = op->code;

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

#if defined(__GNUC__)
#define opjmp(op)                                                       \
    do {                                                                \
        struct v0op *_etext = vm->seglims[V0_TEXT_SEG];                 \
                                                                        \
        while (v0opisnop(op)) {                                         \
            op++;                                                       \
        }                                                               \
        if (op <= _etext) {                                             \
            goto *jmptab[(op)->code];                                   \
        } else {                                                        \
            v0doxcpt(V0_TEXT_FAULT);                                    \
                                                                        \
            return EXIT_FAILURE;                                        \
        }                                                               \
    } while (0)
#endif /* defined(__GNUC__) */
#define _v0setop(unit, op, func, tab)                                   \
    do {                                                                \
        long _code = v0mkopid(unit, op);                                \
                                                                        \
        opaddtab(unit, op, func);                                       \
        ((_V0OPTAB_T *)tab)[_code] = (func);                            \
    } while (0)

#define v0initops(tab)                                                  \
    do {                                                                \
        _v0setop(V0_BITS, V0_NOP, _v0opadr(nop), tab);                  \
        _v0setop(V0_BITS, V0_NOT, _v0opadr(not), tab);                  \
	_v0setop(V0_BITS, V0_AND, _v0opadr(and), tab);                  \
	_v0setop(V0_BITS, V0_OR, _v0opadr(or), tab);                    \
	_v0setop(V0_BITS, V0_XOR, _v0opadr(xor), tab);                  \
	_v0setop(V0_SHIFT, V0_SHL, _v0opadr(shl), tab);                 \
	_v0setop(V0_SHIFT, V0_SHR, _v0opadr(shr), tab);                 \
	_v0setop(V0_SHIFT, V0_SAR, _v0opadr(sar), tab);                 \
	_v0setop(V0_ARITH, V0_INC, _v0opadr(inc), tab);                 \
	_v0setop(V0_ARITH, V0_DEC, _v0opadr(dec), tab);                 \
	_v0setop(V0_ARITH, V0_ADD, _v0opadr(add), tab);                 \
	_v0setop(V0_ARITH, V0_ADC, _v0opadr(adc), tab);                 \
	_v0setop(V0_ARITH, V0_SUB, _v0opadr(sub), tab);                 \
	_v0setop(V0_ARITH, V0_SBB, _v0opadr(sbb), tab);                 \
	_v0setop(V0_ARITH, V0_CMP, _v0opadr(cmp), tab);                 \
	_v0setop(V0_ARITH, V0_MUL, _v0opadr(mul), tab);                 \
	_v0setop(V0_ARITH, V0_DIV, _v0opadr(div), tab);                 \
	_v0setop(V0_ARITH, V0_REM, _v0opadr(rem), tab);                 \
	_v0setop(V0_FLOW, V0_JMP, _v0opadr(jmp), tab);                  \
	_v0setop(V0_FLOW, V0_CALL, _v0opadr(call), tab);                \
	_v0setop(V0_FLOW, V0_CPL, _v0opadr(cpl), tab);                  \
	_v0setop(V0_FLOW, V0_ENTER, _v0opadr(enter), tab);              \
	_v0setop(V0_FLOW, V0_LEAVE, _v0opadr(leave), tab);              \
	_v0setop(V0_FLOW, V0_RET, _v0opadr(ret), tab);                  \
	_v0setop(V0_FLOW, V0_BZ, _v0opadr(bz), tab);                    \
	_v0setop(V0_FLOW, V0_BNZ, _v0opadr(bnz), tab);                  \
	_v0setop(V0_FLOW, V0_BC, _v0opadr(bc), tab);                    \
	_v0setop(V0_FLOW, V0_BNC, _v0opadr(bnc), tab);                  \
        _v0setop(V0_FLOW, V0_BO, _v0opadr(bo), tab);                    \
	_v0setop(V0_FLOW, V0_BNO, _v0opadr(bno), tab);                  \
	_v0setop(V0_FLOW, V0_BLT, _v0opadr(blt), tab);                  \
	_v0setop(V0_FLOW, V0_BLE, _v0opadr(ble), tab);                  \
	_v0setop(V0_FLOW, V0_BGT, _v0opadr(bgt), tab);                  \
	_v0setop(V0_FLOW, V0_BGE, _v0opadr(bge), tab);                  \
	_v0setop(V0_XFER, V0_LDR, _v0opadr(ldr), tab);                  \
	_v0setop(V0_XFER, V0_STR, _v0opadr(str), tab);                  \
	_v0setop(V0_STACK, V0_PSH, _v0opadr(psh), tab);                 \
	_v0setop(V0_STACK, V0_POP, _v0opadr(pop), tab);                 \
	_v0setop(V0_STACK, V0_PSHA, _v0opadr(psha), tab);               \
	_v0setop(V0_STACK, V0_POPA, _v0opadr(popa), tab);               \
	_v0setop(V0_IO, V0_IOR, _v0opadr(ior), tab);                    \
	_v0setop(V0_IO, V0_IOW, _v0opadr(iow), tab);                    \
    } while (0)

void
v0initio(struct v0 *vm)
{
    struct v0iofuncs *vec = vm->iovec;

    vec[V0_STDIN_PORT].rdfunc = v0readkbd;
    vec[V0_STDOUT_PORT].wrfunc = v0writetty;
    vec[V0_STDERR_PORT].wrfunc = v0writetty;
    vec[V0_RTC_PORT].rdfunc = v0readrtc;
    vec[V0_TMR_PORT].rdfunc = v0readtmr;

    return;
}

struct v0 *
v0init(struct v0 *vm)
{
    void *mem = calloc(1, V0_MEM_SIZE);
    void *ptr;
    long  newvm = 0;

    if (!mem) {

        return NULL;
    }
    ptr = calloc(V0_NIOPORT_MAX, sizeof(struct v0iofuncs));
    if (ptr) {
        if (!vm) {
            vm = malloc(sizeof(struct v0));
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
        v0initio(vm);
        vm->regs.sys[V0_FP_REG] = 0x00000000;
        vm->regs.sys[V0_SP_REG] = V0_MEM_SIZE;
    }
    v0vm = vm;

    return vm;
}

int
v0loop(struct v0 *vm)
{
    static _V0OPTAB_T  jmptab[V0_NINST_MAX];
    v0reg              pc = vm->regs.sys[V0_PC_REG];
    struct v0op       *op = (struct v0op *)&vm->mem[pc];

    v0initops(jmptab);

#if defined(__GNUC__)

    do {
        v0opnop:
            op = v0nop(vm, op);

            opjmp(op);
        v0opnot:
            op = v0not(vm, op);

            opjmp(op);
        v0opand:
            op = v0and(vm, op);

            opjmp(op);
        v0opor:
            op = v0or(vm, op);

            opjmp(op);
        v0opxor:
            op = v0xor(vm, op);

            opjmp(op);
        v0opshl:
            op = v0shl(vm, op);

            opjmp(op);
        v0opshr:
            op = v0shr(vm, op);

            opjmp(op);
        v0opsar:
            op = v0sar(vm, op);

            opjmp(op);
        v0opinc:
            op = v0inc(vm, op);

            opjmp(op);
        v0opdec:
            op = v0dec(vm, op);

            opjmp(op);
        v0opadd:
            op = v0add(vm, op);

            opjmp(op);
        v0opadc:
            op = v0adc(vm, op);

            opjmp(op);
        v0opsub:
            op = v0sub(vm, op);

            opjmp(op);
        v0opsbb:
            v0sbb(vm, op);

            opjmp(op);
        v0opcmp:
            op = v0cmp(vm, op);

            opjmp(op);
        v0opmul:
            op = v0mul(vm, op);

            opjmp(op);
        v0opdiv:
            op = v0div(vm, op);

            opjmp(op);
        v0oprem:
            op = v0rem(vm, op);

            opjmp(op);
#if 0
        v0opcrm:
            op = v0crm(vm, op);

            opjmp(op);
#endif
        v0opjmp:
            op = v0jmp(vm, op);

            opjmp(op);
        v0opcpl:
            op = v0cpl(vm, op);

            opjmp(op);
        v0opcall:
            op = v0call(vm, op);

            opjmp(op);
        v0openter:
            op = v0enter(vm, op);

            opjmp(op);
        v0opleave:
            op = v0leave(vm, op);

            opjmp(op);
        v0opret:
            op = v0ret(vm, op);

            opjmp(op);
        v0opbz:
            op = v0bz(vm, op);

            opjmp(op);
        v0opbnz:
            op = v0bnz(vm, op);

            opjmp(op);
        v0opbc:
            op = v0bc(vm, op);

            opjmp(op);
        v0opbnc:
            op = v0bnc(vm, op);

            opjmp(op);
        v0opbo:
            op = v0bo(vm, op);

            opjmp(op);
        v0opbno:
            op = v0bno(vm, op);

            opjmp(op);
        v0opblt:
            op = v0blt(vm, op);

            opjmp(op);
        v0opble:
            op = v0ble(vm, op);

            opjmp(op);
        v0opbgt:
            op = v0bgt(vm, op);

            opjmp(op);
        v0opbge:
            op = v0bge(vm, op);

            opjmp(op);
        v0opldr:
            op = v0ldr(vm, op);

            opjmp(op);
        v0opstr:
            op = v0str(vm, op);

            opjmp(op);
        v0oppsh:
            op = v0psh(vm, op);

            opjmp(op);
        v0oppop:
            op = v0pop(vm, op);

            opjmp(op);
        v0oppsha:
            op = v0psha(vm, op);

            opjmp(op);
        v0oppopa:
            op = v0popa(vm, op);

            opjmp(op);
        v0opior:
            op = v0ior(vm, op);

            opjmp(op);
        v0opiow:
            op = v0iow(vm, op);

            opjmp(op);
    } while (1);

#else /* !defined(__GNUC__) */

    while (op) {
        static struct v0op *etext = vm->seglims[V0_TEXT_SEG];
        uint8_t             code = op->code;
        v0opfunc           *func = jmptab[code];

        op = func(vm, op);
        if (op > etext) {
            v0doxcpt(V0_TEXT_FAULT);

            return EXIT_FAILURE;
        }
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
    struct v0 *vm = v0init(NULL);
    int        ret = EXIT_FAILURE;

    if (vm) {
        v0getopt(vm, argc, argv);
        if (!vm->regs.sys[V0_PC_REG]) {
            vm->regs.sys[V0_PC_REG] = V0_TEXT_ADR;
        }
        ret = v0loop(vm);
    }

    exit(ret);
}

