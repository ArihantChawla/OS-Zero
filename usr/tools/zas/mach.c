static struct zastoken *
zasprocinst(struct zastoken *token, zasmemadr_t adr,
            zasmemadr_t *retadr)
{
#if (WPM)
    struct wpmopcode *op = NULL;
#elif (ZEN)
    struct zpuop     *op = NULL;
#endif
#if (WPMVEC)
    struct vecopcode *vop = NULL;
#endif
    zasmemadr_t       opadr = rounduppow2(adr, 4);
    struct zastoken  *token1 = NULL;
    struct zastoken  *token2 = NULL;
    struct zastoken  *retval = NULL;
    struct zassymrec *sym;
    uint8_t           narg = token->data.inst.narg;
//    uint8_t           len = token->data.inst.op == OPNOP ? 1 : 4;
    uint8_t           len = 4;

    while (adr < opadr) {
#if (WPM)
        physmem[adr] = OPNOP;
#endif
        adr++;
    }
//    adr = opadr;
#if (ZASDB)
    zasaddline(adr, token->data.inst.data, token->file, token->line);
#endif
#if (WPMVEC)
    if (token->unit == UNIT_VEC) {
        vop = (struct vecopcode *)&zvm.physmem[adr];
        vop->inst = token->data.inst.op;
        vop->unit = UNIT_VEC;
        vop->flg = token->opflg;
        token1 = token->next;
        zasfreetoken(token);
        if (token1) {
            switch(token1->type) {
                case ZASTOKENVAREG:
                    vop->arg1t = ARGVAREG;
                    vop->reg1 = token1->data.reg & 0xff;
                    
                    break;
                case ZASTOKENVLREG:
                    vop->arg1t = ARGVLREG;
                    vop->reg1 = token1->data.reg & 0xff;
                    
                    break;
                case ZASTOKENIMMED:
                    vop->arg1t = ARGIMMED;
                    vop->args[0] = token1->val;
                    len += sizeof(zasword_t);
                    
                    break;
                case ZASTOKENADR:
                    vop->arg1t = ARGIMMED;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = (uint8_t *)strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->args[0];
                    zasqueuesym(sym);
                    len += sizeof(uintptr_t);
                    
                    break;
                default:
                    fprintf(stderr, "invalid argument 1 of type %lx\n", token1->type);
                    printtoken(token1);
                    
                    exit(1);
                    
                    break;
            }
            token2 = token1->next;
            zasfreetoken(token1);
            retval = token2;
        }
        vop->narg = len >> 3;
        if (narg == 1) {
            vop->arg2t = ARGNONE;
        } else if (narg == 2 && (token2)) {
            switch(token2->type) {
                case ZASTOKENVAREG:
                    vop->arg2t = ARGVAREG;
                    vop->reg2 = token2->data.reg & 0xff;
                    
                    break;
                case ZASTOKENVLREG:
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
            zasfreetoken(token2);
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
        if (op->inst == OPNOP) {
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
                zasfreetoken(token);
                if (token1) {
                    switch(token1->type) {
                        case ZASTOKENVALUE:
                            op->arg1t = ARGIMMED;
                            op->args[0] = token1->data.value.val;
                            len += sizeof(zasword_t);
                            
                            break;
                        case ZASTOKENREG:
                            op->arg1t = ARGREG;
                            op->reg1 = token1->data.reg;
                            
                            break;
                        case ZASTOKENSYM:
                            op->arg1t = ARGADR;
                            sym = malloc(sizeof(struct zassymrec));
                            sym->name = (uint8_t *)strdup((char *)token1->data.sym.name);
                            sym->adr = (uintptr_t)&op->args[0];
                            zasqueuesym(sym);
                            len += sizeof(uintptr_t);
                            
                            break;
                        case ZASTOKENINDIR:
                            token1 = token1->next;
                            if (token1->type == ZASTOKENREG) {
                                op->arg1t = ARGREG;
                                op->reg1 = token1->data.reg;
                            } else {
                                fprintf(stderr, "indirect addressing requires a register\n");
                                
                                exit(1);
                            }
                            
                            break;
                        case ZASTOKENIMMED:
                            op->arg1t = ARGIMMED;
                            op->args[0] = token1->val;
                            len += sizeof(zasword_t);
                            
                            break;
                        case ZASTOKENADR:
                            op->arg1t = ARGIMMED;
                            sym = malloc(sizeof(struct zassymrec));
                            sym->name = (uint8_t *)strdup((char *)token1->data.sym.name);
                            sym->adr = (uintptr_t)&op->args[0];
                            zasqueuesym(sym);
                            len += sizeof(uintptr_t);

                            break;
                        case ZASTOKENINDEX:
                            op->arg1t = ARGREG;
                            op->reg1 = token1->data.ndx.reg;
                            op->args[0] = token1->data.ndx.val;
                            len += sizeof(zasword_t);
                            
                            break;
                        default:
                            fprintf(stderr, "invalid argument 1 of type %lx\n", token1->type);
                            printtoken(token1);
                            
                            exit(1);
                            
                            break;
                    }
                    token2 = token1->next;
                    zasfreetoken(token1);
                    retval = token2;
                }
                if (narg == 1) {
                    op->arg2t = ARGNONE;
                } else if (narg == 2 && (token2)) {
                    switch(token2->type) {
#if (WPMVEC)
                        case ZASTOKENVAREG:
                            op->arg2t = ARGVAREG;
                            op->reg2 = token2->data.reg & 0xff;
                            
                            break;
                        case ZASTOKENVLREG:
                            op->arg2t = ARGVLREG;
                            op->reg2 = token2->data.reg & 0xff;
                            
                            break;
#endif
                        case ZASTOKENVALUE:
                            op->arg2t = ARGIMMED;
                            if (op->arg1t == ARGREG) {
                                op->args[0] = token2->data.value.val;
                            } else {
                                op->args[1] = token2->data.value.val;
                            }
                            len += sizeof(zasword_t);
                            
                            break;
                        case ZASTOKENREG:
                            op->arg2t = ARGREG;
                            op->reg2 = token2->data.reg;
                            
                            break;
                        case ZASTOKENSYM:
                            op->arg2t = ARGADR;
                            sym = malloc(sizeof(struct zassymrec));
                            sym->name = (uint8_t *)strdup((char *)token2->data.sym.name);
                            if (op->arg1t == ARGREG) {
                                sym->adr = (uintptr_t)&op->args[0];
                            } else {
                                sym->adr = (uintptr_t)&op->args[1];
                            }
                            zasqueuesym(sym);
                            len += sizeof(uintptr_t);
                            
                            break;
                        case ZASTOKENINDIR:
                            token2 = token2->next;
                            if (token2->type == ZASTOKENREG) {
                                op->arg2t = ARGREG;
                                op->reg2 = token2->data.reg;
                            } else {
                                fprintf(stderr, "indirect addressing requires a register\n");
                                
                                exit(1);
                            }
                            
                            break;
                        case ZASTOKENIMMED:
                            op->arg2t = ARGIMMED;
                            if (op->arg1t == ARGREG) {
                                op->args[0] = token2->val;
                            } else {
                                op->args[1] = token2->val;
                            }
                            len += sizeof(zasword_t);
                            
                            break;
                        case ZASTOKENADR:
                            op->arg2t = ARGIMMED;
                            sym = malloc(sizeof(struct zassymrec));
                            sym->name = (uint8_t *)strdup((char *)token2->data.sym.name);
                            if (op->arg1t == ARGREG) {
                                sym->adr = (uintptr_t)&op->args[0];
                            } else {
                                sym->adr = (uintptr_t)&op->args[1];
                            }
                            zasqueuesym(sym);
                            len += sizeof(uintptr_t);
                            
                            break;
                        case ZASTOKENINDEX:
                            op->arg2t = ARGREG;
                            op->reg2 = token2->data.ndx.reg;
                            if (op->arg1t == ARGREG) {
                                op->args[0] = token2->data.ndx.val;
                            } else {
                                op->args[1] = token2->data.ndx.val;
                            }
                            len += sizeof(zasword_t);
                            
                            break;
                        default:
                            fprintf(stderr, "invalid argument 2 of type %lx\n", token2->type);
                            printtoken(token2);
                            
                            exit(1);
                            
                            break;
                    }
                    retval = token2->next;
                    zasfreetoken(token2);
                }
            }
        op->size = len >> 2;
    }
    *retadr = adr + len;

    return retval;
}

