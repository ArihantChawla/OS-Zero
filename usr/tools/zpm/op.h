#ifndef __ZPM_OP_H__
#define __ZPM_OP_H__

#define zpmget1arggenreg(vm, op, dptr)                                  \
    ((dptr) = &vm->genregs[(op)->reg1], *(dptr))
#define zpmget2argsgenreg(vm, op, dptr, src, dest)                      \
    (!((op)->argt & ZPM_IMM_VAL)                                        \
     ? ((dptr) = &vm->genregs[(op)->reg2],                              \
        (src) = vm->genregs[(op)->reg1],                                \
        (dest) = *(dptr))                                               \
     : ((dptr) = &vm->gengregs[(op)->reg2],                             \
        (src) = op->imm,                                                \
        (dest) = *(dptr)))

#endif /* __ZPM_OP_H__ */

