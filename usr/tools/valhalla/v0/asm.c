/* zero assembler [virtual] machine interface */

#include <vas/conf.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <stdint.h>
#include <zero/trix.h>
#include <vas/vas.h>
#include <v0/mach.h>
#include <v0/vm.h>

extern struct v0 *v0vm;

struct vasop    vasoptab[256];
struct vasop   *vasoptree[256];
static long     v0valbits[V0_NINST_MAX / (sizeof(long) * CHAR_BIT)];
static long     v0arginit;

#define vasopisnop(op)    (*((uint32_t *)(op)) == V0_NOP)
#define vasophasval(code) (bitset(v0valbits, code))

#define vassetnop(adr)    (*((uint32_t *)(op)) = V0_NOP)
#define vassetop(adr, str, op, n, slen)                                 \
    do {                                                                \
        struct vasop *_ptr = (struct vasop *)(adr);                     \
                                                                        \
        _ptr->name = strndup(str, len);                                 \
        _ptr->code = (op);                                              \
        _ptr->narg = (n);                                               \
        _ptr->len = (slen);                                             \
    } while (0)

/*
 * operation info structure addresses are stored in a multilevel table
 * - the top level table is indexed with the first byte of mnemonic and so on
 * TODO: more compact encoding of 6-bit characters + some flag bits in int32_t
 */
static long
vasaddop(const char *str, uint8_t code, uint8_t narg)
{
    char    *cptr = (char *)str;
    void    *ptr;
    long     key;
    uint8_t  len;

    while (*cptr && isalpha(cptr[1])) {
        key = *cptr++;
        ptr = ((struct vasop *)ptr)->tab[key];
        if (!ptr) {
            ptr = calloc(256, sizeof(struct vasop *));
            if (!ptr) {
                fprintf(stderr, "failed to allocate operation table\n");

                return 0;
            }
            ((struct vasop *)ptr)->tab[key] = ptr;
        }
    }
    key = *cptr;
    if (key) {
        len = cptr - str;
        ((struct vasop *)ptr)->tab[key] = &vasoptab[code];
        ptr = &vasoptab[code];
        vassetop(ptr, str, code, narg, len);
    }

    return key;
}

static struct vasop *
vasfindop(char *str)
{
    char         *cptr = (char *)str;
    struct vasop *ptr = NULL;
    long          key;

    while (*cptr && isalpha(*cptr)) {
        key = *cptr++;
        ptr = ptr->tab[key];
        if (!ptr) {

            return NULL;
        }
    }

    return ptr;
}

struct vasop *
vasgetop(char *str, char **retptr)
{
    struct vasop *op = NULL;
    uint8_t       narg;

    op = vasfindop(str);
#if (VASDEBUG)
    fprintf(stderr, "getop: %s\n", str);
#endif
    if (op) {
        str += op->len;
        *retptr = str;
    }

    return op;
}

vasuword
vasgetreg(char *str, vasword *retsize, char **retptr)
{
    vasuword reg = ~0;
    vasword  size = 0;

#if (VASDEBUG)
    fprintf(stderr, "getreg: %s\n", str);
#endif
    if (*str == 'r') {
        str++;
        size = 4;
    } else if (*str == 'b') {
        str++;
        size = 1;
    } else if (*str == 'w') {
        str++;
        size = 2;
    } else if (*str == 'l') {
        str++;
        size = 4;
#if (!V032BIT)
    } else if (*str == 'q') {
        str++;
        size = 8;
#endif
    }
    if (size) {
        while ((*str) && isdigit(*str)) {
            reg *= 10;
            reg += *str - '0';
            str++;
        }
        while (*str == ')' || *str == ',') {
            str++;
        }
        *retsize = size;
        *retptr = str;
    } else {
        fprintf(stderr, "invalid register name %s\n", str);

        exit(1);
    }

    return reg;
}

struct vastoken *
vasprocop(struct vastoken *token, v0memadr adr,
          v0memadr *retadr)
{
    struct v0op      *op = (struct v0op *)&v0vm->mem[adr];
    v0memadr          opadr = rounduppow2(adr, 4);
    struct vastoken  *token1 = NULL;
    struct vastoken  *token2 = NULL;
    struct vastoken  *retval = NULL;
    struct vassymrec *sym;
    long              havearg = 0;
    vasword           val;
    uint8_t           narg = token->data.inst.narg;
//    uint8_t           len = token->data.op.op == V0NOP ? 1 : 4;
    uint8_t           len = 1;

    if (!v0arginit) {
        setbit(v0valbits, v0mkopid(V0_SHIFT, V0_SHL));
        setbit(v0valbits, v0mkopid(V0_SHIFT, V0_SHR));
        setbit(v0valbits, v0mkopid(V0_SHIFT, V0_SAR));
        setbit(v0valbits, v0mkopid(V0_IO, V0_IOR));
        setbit(v0valbits, v0mkopid(V0_IO, V0_IOW));
#if 0
        setbit(v0rngbits, v0mkopid(V0_STACK, V0_PSHM));
        setbit(v0rngbits, v0mkopid(V0_STACK, V0_POPM));
#endif
    }
    while (adr < opadr) {
#if (V032BIT)
        *(uint32_t *)op = UINT32_C(~0);
#else
        *(uint64_t *)op = UINT32_C(~0);
#endif
        adr += sizeof(struct v0op);
        op++;
    }
//    adr = opadr;
#if (VASDB)
    vasaddline(adr, token->data.inst.data, token->file, token->line);
#endif
    op->code = token->data.inst.code;
    if (vasopisnop(op)) {
        retval = token->next;
        vassetnop(adr);
        adr += sizeof(struct v0op);
    } else if (!narg) {
        /* FIXME: failure? */
        op->reg1 = 0;
        op->reg2 = 0;
        op->adr = 0;
        op->parm = 0;
        op->val = 0;
        retval = token->next;
    } else {
        token1 = token->next;
        vasfreetoken(token);
        if (token1) {
            switch(token1->type) {
                case VASTOKENVALUE:
                    val = token1->data.value.val;
                    op->adr = V0_DIR_ADR;
                    havearg = 1;
                    switch (token->data.value.size) {
                        case 1:
                            op->arg[0].i8 = val;

                            break;
                        case 2:
                            op->arg[0].i16 = val;

                            break;
                        case 3:
                            op->arg[0].i32 = val;

                            break;
                        default:
                            fprintf(stderr,
                                    "invalid-size immediate value\n");

                            exit(1);
                    }
                    len++;
                    op->parm = len;

                    break;
                case VASTOKENREG:
                    op->adr = V0_REG_ADR;
                    op->reg1 = token1->data.reg;

                    break;
                case VASTOKENSYM:
                    op->adr = V0_DIR_ADR;
                    sym = malloc(sizeof(struct vassymrec));
                    sym->name = strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->arg[0].adr;
                    vasqueuesym(sym);
                    len++;

                    break;
                case VASTOKENINDIR:
                    token1 = token1->next;
                    if (token1->type == VASTOKENREG) {
                        op->adr = V0_REG_ADR;
                        op->reg1 = token1->data.reg;
                    } else {
                        fprintf(stderr,
                                "indirect addressing requires a register\n");

                        exit(1);
                    }

                    break;
                case VASTOKENIMMED:
                    val = token1->data.value.val;
                    op->adr = V0_DIR_ADR;
                    havearg = 1;
                    op->val = val;
                    switch (token->data.value.size) {
                        case 1:
                            op->arg[0].i8 = token1->data.value.val;

                            break;
                        case 2:
                            op->arg[0].i16 = token1->data.value.val;

                            break;
                        case 3:
                            op->arg[0].i32 = token1->data.value.val;

                            break;
                        default:
                            fprintf(stderr,
                                    "invalid-size immediate value\n");

                            exit(1);
                    }
                    len++;

                    break;
                case VASTOKENADR:
                    op->adr = V0_DIR_ADR;
                    sym = malloc(sizeof(struct vassymrec));
                    sym->name = strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->arg[0].adr;
                    vasqueuesym(sym);
                    len++;

                    break;
                case VASTOKENINDEX:
                    op->adr = V0_NDX_ADR;
                    op->reg1 = token1->data.ndx.reg;
                    op->arg[0].ndx = token1->data.ndx.val;
                    len++;

                    break;
                default:
                    fprintf(stderr,
                            "invalid argument 1 of type %lx\n", token1->type);
                    vasprinttoken(token1);

                    exit(1);

                    break;
            }
            token2 = token1->next;
            vasfreetoken(token1);
            retval = token2;
        }
        if (narg == 2 && (token2)) {
            if (token2->type == VASTOKENREG) {
                op->reg2 = token2->data.reg;
            } else if (havearg) {
                fprintf(stderr,
                        "too many non-register arguments\n");

                exit(1);
            } else {
                switch(token2->type) {
                    case VASTOKENVALUE:
                        op->adr = V0_DIR_ADR;
                        op->arg[0].adr = token2->data.value.val;
                        len++;

                        break;
                    case VASTOKENSYM:
                        op->adr = V0_DIR_ADR;
                        sym = malloc(sizeof(struct vassymrec));
                        sym->name = strdup((char *)token2->data.sym.name);
                        sym->adr = (uintptr_t)&op->arg[0].adr;
                        vasqueuesym(sym);
                        len++;

                        break;
                    case VASTOKENINDIR:
                        token2 = token2->next;
                        if (token2->type == VASTOKENREG) {
                            op->adr = V0_REG_ADR;
                            op->reg2 = token2->data.reg;
                        } else {
                            fprintf(stderr,
                                    "indirect addressing requires a register\n");

                            exit(1);
                        }

                        break;
                    case VASTOKENIMMED:
                        op->adr = V0_DIR_ADR;
                        op->arg[0].adr = token2->val;
                        len++;

                        break;
                    case VASTOKENADR:
                        op->adr = V0_DIR_ADR;
                        sym = malloc(sizeof(struct vassymrec));
                        sym->name = strdup((char *)token2->data.sym.name);
                        sym->adr = (uintptr_t)&op->arg[0].adr;
                        vasqueuesym(sym);
                        len++;

                        break;
                    case VASTOKENINDEX:
                        op->adr = V0_NDX_ADR;
                        op->reg2 = token2->data.ndx.reg;
                        op->arg[0].ndx = token2->data.ndx.val;
                        len++;

                        break;
                    default:
                        fprintf(stderr,
                                "invalid argument 2 of type %lx\n", token2->type);
                        vasprinttoken(token2);

                        exit(1);

                        break;
                }
                retval = token2->next;
                vasfreetoken(token2);
            }
        }
    }
    op->parm = len;
#if (V032BIT)
    *retadr = adr + (len << 2);
#else
    *retadr = adr + (len << 3);
#endif

    return retval;
}

static struct vastoken *
vasprocinst(struct vastoken *token, vasmemadr adr,
            vasmemadr *retadr)
{
    struct v0op      *op = NULL;
    //    vasmemadr         opadr = rounduppow2(adr, 4);
    struct vastoken  *token1 = NULL;
    struct vastoken  *token2 = NULL;
    struct vastoken  *retval = NULL;
    struct vassymrec *sym;
    uint8_t           narg = token->data.inst.narg;
    //    uint8_t           len = token->data.inst.op == VASNOP ? 1 : 4;
    uint8_t           len = 4;

    //    adr = opadr;
#if (VASDB)
    vasaddline(adr, token->data.inst.data, token->file, token->line);
#endif
    op = (struct zpuop *)&v0.physmem[adr];
    op->inst = token->data.inst.op;
    if (op->inst == VAS_NOP) {
        vassetnop(op);
        retval = token->next;
    } else if (!narg) {
        op->reg1 = 0;
        op->reg2 = 0;
        op->parm = 0;
        retval = token->next;
    } else {
        token1 = token->next;
        vasfreetoken(token);
        if (token1) {
            switch(token1->type) {
                case VASTOKENVALUE:
                    op->arg1t = ARGIMMED;
                    op->args[0] = token1->data.value.val;
                    len += sizeof(vasword_t);

                    break;
                case VASTOKENREG:
                    op->arg1t = ARGREG;
                    op->reg1 = token1->data.reg;

                    break;
                case VASTOKENSYM:
                    op->arg1t = ARGADR;
                    sym = malloc(sizeof(struct vassymrec));
                    sym->name = (uint8_t *)strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->args[0];
                    vasqueuesym(sym);
                    len += sizeof(uintptr_t);

                    break;
                case VASTOKENINDIR:
                    token1 = token1->next;
                    if (token1->type == VASTOKENREG) {
                        op->arg1t = ARGREG;
                        op->reg1 = token1->data.reg;
                    } else {
                        fprintf(stderr, "indirect addressing requires a register\n");

                        exit(1);
                    }

                    break;
                case VASTOKENIMMED:
                    op->arg1t = ARGIMMED;
                    op->args[0] = token1->val;
                    len += sizeof(vasword_t);

                    break;
                case VASTOKENADR:
                    op->arg1t = ARGIMMED;
                    sym = malloc(sizeof(struct vassymrec));
                    sym->name = (uint8_t *)strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->args[0];
                    vasqueuesym(sym);
                    len += sizeof(uintptr_t);

                    break;
                case VASTOKENINDEX:
                    op->arg1t = ARGREG;
                    op->reg1 = token1->data.ndx.reg;
                    op->args[0] = token1->data.ndx.val;
                    len += sizeof(vasword_t);

                    break;
                default:
                    fprintf(stderr, "invalid argument 1 of type %lx\n", token1->type);
                    printtoken(token1);

                    exit(1);

                    break;
            }
            token2 = token1->next;
            vasfreetoken(token1);
            retval = token2;
        }
        if (narg == 1) {
            op->arg2t = ARGNONE;
        } else if (narg == 2 && (token2)) {
            switch(token2->type) {
#if (WPMVEC)
                case VASTOKENVAREG:
                    op->arg2t = ARGVAREG;
                    op->reg2 = token2->data.reg & 0xff;

                    break;
                case VASTOKENVLREG:
                    op->arg2t = ARGVLREG;
                    op->reg2 = token2->data.reg & 0xff;

                    break;
#endif
                case VASTOKENVALUE:
                    op->arg2t = ARGIMMED;
                    if (op->arg1t == ARGREG) {
                        op->args[0] = token2->data.value.val;
                    } else {
                        op->args[1] = token2->data.value.val;
                    }
                    len += sizeof(vasword_t);

                    break;
                case VASTOKENREG:
                    op->arg2t = ARGREG;
                    op->reg2 = token2->data.reg;

                    break;
                case VASTOKENSYM:
                    op->arg2t = ARGADR;
                    sym = malloc(sizeof(struct vassymrec));
                    sym->name = (uint8_t *)strdup((char *)token2->data.sym.name);
                    if (op->arg1t == ARGREG) {
                        sym->adr = (uintptr_t)&op->args[0];
                    } else {
                        sym->adr = (uintptr_t)&op->args[1];
                    }
                    vasqueuesym(sym);
                    len += sizeof(uintptr_t);

                    break;
                case VASTOKENINDIR:
                    token2 = token2->next;
                    if (token2->type == VASTOKENREG) {
                        op->arg2t = ARGREG;
                        op->reg2 = token2->data.reg;
                    } else {
                        fprintf(stderr, "indirect addressing requires a register\n");

                        exit(1);
                    }

                    break;
                case VASTOKENIMMED:
                    op->arg2t = ARGIMMED;
                    if (op->arg1t == ARGREG) {
                        op->args[0] = token2->val;
                    } else {
                        op->args[1] = token2->val;
                    }
                    len += sizeof(vasword_t);

                    break;
                case VASTOKENADR:
                    op->arg2t = ARGIMMED;
                    sym = malloc(sizeof(struct vassymrec));
                    sym->name = (uint8_t *)strdup((char *)token2->data.sym.name);
                    if (op->arg1t == ARGREG) {
                        sym->adr = (uintptr_t)&op->args[0];
                    } else {
                        sym->adr = (uintptr_t)&op->args[1];
                    }
                    vasqueuesym(sym);
                    len += sizeof(uintptr_t);

                    break;
                case VASTOKENINDEX:
                    op->arg2t = ARGREG;
                    op->reg2 = token2->data.ndx.reg;
                    if (op->arg1t == ARGREG) {
                        op->args[0] = token2->data.ndx.val;
                    } else {
                        op->args[1] = token2->data.ndx.val;
                    }
                    len += sizeof(vasword_t);

                    break;
                default:
                    fprintf(stderr, "invalid argument 2 of type %lx\n", token2->type);
                    printtoken(token2);

                    exit(1);

                    break;
            }
            retval = token2->next;
            vasfreetoken(token2);
        }
        op->size = len >> 2;
    }
    *retadr = adr + len;

    return retval;
}

#endif

