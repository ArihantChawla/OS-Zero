#if 0

/* zero assembler [virtual] machine interface */

#include <vas/conf.h>

#if (!ZVM)

static void
vasaddop(struct vasop *op)
{
    uint8_t       *str = op->name;
    unsigned long  key = 0;
    unsigned long  len = 0;

    while (isalpha(*str)) {
        key += *str++;
        len++;
    }
    op->len = len;
    key &= (VASNHASH - 1);
    op->next = vasophash[key];
    vasophash[key] = op;

    return;
}

struct vasop *
vasfindop(uint8_t *name)
{
    struct vasop  *op = NULL;
    uint8_t       *str = name;
    unsigned long  key = 0;

    while ((*str) && isalpha(*str)) {
        key += *str++;
    }
    key &= (VASNHASH - 1);
    op = vasophash[key];
    while ((op) && strncmp((char *)op->name, (char *)name, op->len)) {
        op = op->next;
    }

    return op;
}

#endif /* !ZVM */

static struct vastoken *
vasprocinst(struct vastoken *token, vasmemadr_t adr,
            vasmemadr_t *retadr)
{
#if (WPM)
    struct wpmopcode *op = NULL;
#elif (ZEN)
    struct zpuop     *op = NULL;
#endif
#if (WPMVEC)
    struct vecopcode *vop = NULL;
#endif
    vasmemadr_t       opadr = rounduppow2(adr, 4);
    struct vastoken  *token1 = NULL;
    struct vastoken  *token2 = NULL;
    struct vastoken  *retval = NULL;
    struct vassymrec *sym;
    uint8_t           narg = token->data.inst.narg;
//    uint8_t           len = token->data.inst.op == VASNOP ? 1 : 4;
    uint8_t           len = 4;

    while (adr < opadr) {
#if (WPM)
        physmem[adr] = VASNOP;
#endif
        adr++;
    }
//    adr = opadr;
#if (VASDB)
    vasaddline(adr, token->data.inst.data, token->file, token->line);
#endif
#if (WPMVEC)
    if (token->unit == UNIT_VEC) {
        vop = (struct vecopcode *)&zvm.physmem[adr];
        vop->inst = token->data.inst.op;
        vop->unit = UNIT_VEC;
        vop->flg = token->opflg;
        token1 = token->next;
        vasfreetoken(token);
        if (token1) {
            switch(token1->type) {
                case VASTOKENVAREG:
                    vop->arg1t = ARGVAREG;
                    vop->reg1 = token1->data.reg & 0xff;

                    break;
                case VASTOKENVLREG:
                    vop->arg1t = ARGVLREG;
                    vop->reg1 = token1->data.reg & 0xff;

                    break;
                case VASTOKENIMMED:
                    vop->arg1t = ARGIMMED;
                    vop->args[0] = token1->val;
                    len += sizeof(vasword_t);

                    break;
                case VASTOKENADR:
                    vop->arg1t = ARGIMMED;
                    sym = malloc(sizeof(struct vassymrec));
                    sym->name = (uint8_t *)strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->args[0];
                    vasqueuesym(sym);
                    len += sizeof(uintptr_t);

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
        vop->narg = len >> 3;
        if (narg == 1) {
            vop->arg2t = ARGNONE;
        } else if (narg == 2 && (token2)) {
            switch(token2->type) {
                case VASTOKENVAREG:
                    vop->arg2t = ARGVAREG;
                    vop->reg2 = token2->data.reg & 0xff;

                    break;
                case VASTOKENVLREG:
                    vop->arg2t = ARGVLREG;
                    vop->reg2 = token2->data.reg & 0xff;

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
    } else
#endif
    {
#if (WPM)
        op = (struct wpmopcode *)&physmem[adr];
#elif (ZEN)
        op = (struct zpuop *)&zvm.physmem[adr];
#endif
        op->inst = token->data.inst.op;
        if (op->inst == VASNOP) {
            retval = token->next;
            adr++;
        } else
            if (!narg) {
                op->arg1t = ARGNONE;
                op->arg2t = ARGNONE;
                op->reg1 = 0;
                op->reg2 = 0;
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
            }
        op->size = len >> 2;
    }
    *retadr = adr + len;

    return retval;
}

#endif

