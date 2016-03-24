#ifndef __ZVM_OP_H__
#define __ZVM_OP_H__

#define zpmgetjmparg(vm, op)                                            \
    zpmgetsrcarg(vm, op)
#define zpmgetsrcarg(vm, op)                                            \
    (((op)->adr & ZPM_REG_VAL)                                          \
     ? (((vm)->regs[(op)->reg1])                                        \
        ? (((op)->adr & ZPM_IMM8_VAL)                                   \
           ? ((op)->imm8)                                               \
           : ((op)->args[0]))))
#define zpmgetdestptr(vm, op)                                           \
    (((op)->adr & ZPM_REG_VAL)                                          \
     ? (&(vm)->regs[(op)->reg2])                                        \
     : (&(vm)->physmem[(op)->args[0]]))
#define zpmgetldrsrcptr(vm, op)                                         \
    (((op)->adr & ZPM_REG_ADR)                                          \
     ? (&(vm)->physmem[(op)->reg1])                                     \
     : (((op)->adr & ZPM_REG_NDX)                                       \
        ? (&(vm)->physmem[(op)->reg1 + (op)->args[0]])                  \
        : (((op)->adr & ZPM_IMM_ADR)                                    \
           ? (((op)->adr & ZPM_REG_NDX)                                 \
              ? (&(vm)->physmem[(op)->reg1 + (op)->args[0]])            \
              : (&(vm)->physmem[(op)->args[0]])))))
#define zpmgetstrdestptr(vm, op)                                        \
    (((op)->adr & ZPM_REG_ADR)                                          \
     ? (&(vm)->physmem[(op)->reg2])                                     \
     : (((op)->adr & ZPM_IMM_ADR)                                       \
        ? (((op)->adr & ZPM_REG_NDX)                                    \
           ? (&(vm)->physmem[(op)->reg2 + (op)->args[0]])               \
           : (&(vm)->physmem[(op)->args[0]]))))
#define zpmsetzf(val)                                                   \
    (!(val) \
     ? (zpm.msw_reg |= ZPM_MSW_ZF) \
     : (zpm.msw_reg &= ~ZPM_MSW_ZF))

#endif /* __ZPM_OP_H__ */

