#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zero/trix.h>
#include <zcc/zcc.h>

extern struct zccmach zccmach;
extern long           typesztab[32];

#if (ZCCPRINT)
static void zppprinttoken(struct zpptoken *token);
#endif
struct zccstruct * zppgetstruct(struct zpptoken *token,
                                struct zpptoken **nextret,
                                size_t *sizeret);
struct zccunion * zppgetunion(struct zpptoken *token,
                              struct zpptoken **nextret,
                              size_t *sizeret);
static struct zcctoken * zppgetfunc(struct zpptoken *token,
                                    struct zpptoken **nextret,
                                    struct zcctoken **lastret);
void zccqueuetoken(struct zcctoken *token, struct zcctoken *last,
                   struct zcctoken **queue, struct zcctoken **tail);

typedef struct zcctoken * zpptokenfunc_t(struct zpptoken *,
                                         struct zpptoken **,
                                         struct zcctoken **);

struct zcctoken *zppprocif(struct zpptoken *, struct zpptoken **,
                           struct zcctoken **);
struct zcctoken *zppprocifdef(struct zpptoken *, struct zpptoken **,
                              struct zcctoken **);
struct zcctoken *zppprocifndef(struct zpptoken *, struct zpptoken **,
                               struct zcctoken **);
struct zcctoken *zppprocdefine(struct zpptoken *, struct zpptoken **,
                               struct zcctoken **);

#if (ZCCPRINT) || (ZPPDEBUG)
static char *toktypetab[256] =
{
    NULL,
    "ZPP_TYPE_TOKEN",
    "ZPP_TYPEDEF_TOKEN",
    "ZPP_VAR_TOKEN",
    "ZPP_STRUCT_TOKEN",
    "ZPP_UNION_TOKEN",
    "ZPP_OPER_TOKEN",
    "ZPP_DOT_TOKEN",
    "ZPP_INDIR_TOKEN",
    "ZPP_ASTERISK_TOKEN",
    "ZPP_COMMA_TOKEN",
    "ZPP_SEMICOLON_TOKEN",
    "ZPP_COLON_TOKEN",
    "ZPP_EXCLAMATION_TOKEN",
    "ZPP_LEFT_PAREN_TOKEN",
    "ZPP_RIGHT_PAREN_TOKEN",
    "ZPP_INDEX_TOKEN",
    "ZPP_END_INDEX_TOKEN",
    "ZPP_BLOCK_TOKEN",
    "ZPP_END_BLOCK_TOKEN",
    "ZPP_QUOTE_TOKEN",
    "ZPP_DOUBLE_QUOTE_TOKEN",
    "ZPP_BACKSLASH_TOKEN",
    "ZPP_NEWLINE_TOKEN",
    "ZPP_VALUE_TOKEN",
    "ZPP_STRING_TOKEN",
    "ZPP_LITERAL_TOKEN",
    "ZPP_QUAL_TOKEN",
    "ZPP_ATR_TOKEN",
    "ZPP_FUNC_TOKEN",
    "ZPP_LABEL_TOKEN",
    "ZPP_ADR_TOKEN",
    "ZPP_MACRO_TOKEN",
    "ZPP_PREPROC_TOKEN",
    "ZPP_CONCAT_TOKEN",
    "ZPP_STRINGIFY_TOKEN"
#if 0
    "ZPP_LATIN1_TOKEN",
    "ZPP_UTF8_TOKEN",
    "ZPP_UCS16_TOKEN",
    "ZPP_UCS32_TOKEN"
#endif
};
static char *tokparmtab[256] =
{
    "NONE",
    "ZCC_EXTERN_QUAL",
    "ZCC_STATIC_QUAL",
    "ZCC_CONST_QUAL",
    "ZCC_VOLATILE_QUAL",
    "ZPP_IF_DIR",
    "ZPP_ELIF_DIR",
    "ZPP_ELSE_DIR",
    "ZPP_ENDIF_DIR",
    "ZPP_IFDEF_DIR",
    "ZPP_IFNDEF_DIR",
    "ZPP_DEFINE_DIR"
};
static char *typetab[32] =
{
    "NONE",
    "ZCC_CHAR",
    "ZCC_UCHAR",
    "ZCC_SHORT",
    "ZCC_USHORT",
    "ZCC_INT",
    "ZCC_UINT",
    "ZCC_LONG",
    "ZCC_ULONG",
    "ZCC_LONGLONG",
    "ZCC_ULONGLONG",
    "ZCC_FLOAT",
    "ZCC_DOUBLE",
    "ZCC_LDOUBLE",
    "ZCC_VOID"
};
#endif /* ZCCPRINT */
static zpptokenfunc_t *dirfunctab[16] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    zppprocif,          // ZPP_IF_DIR
    NULL,               // ZPP_ELIF_DIR
    NULL,               // ZPP_ELSE_DIR
    NULL,               // ZPP_ENDIF_DIR
    zppprocifdef,       // ZPP_IFDEF_DIR
    zppprocifndef,      // ZPP_IFNDEF_DIR
    zppprocdefine       // ZPP_DEFINE_DIR
};

static long qualflgtab[16] = {
    0,
    ZCC_EXTERN,         // ZCC_EXTERN_QUAL
    ZCC_STATIC,         // ZCC_STATIC_QUAL
    ZCC_CONST,          // ZCC_CONST_QUAL
    ZCC_VOLATILE        // ZCC_VOLATILE_QUAL
};

#if (ZCCPRINT) || (ZPPDEBUG)

void
zppprintval(struct zccval *val)
{
    switch (val->type) {
        case ZCC_CHAR:
            fprintf(stderr, "CHAR: %x\n", val->ival.c);

            break;
        case ZCC_UCHAR:
            fprintf(stderr, "UCHAR: %x\n", val->ival.uc);

            break;
        case ZCC_SHORT:
            fprintf(stderr, "SHORT: %x\n", val->ival.s);

            break;
        case ZCC_USHORT:
            fprintf(stderr, "USHORT: %x\n", val->ival.us);

            break;
        case ZCC_INT:
            fprintf(stderr, "INT: %x\n", val->ival.i);

            break;
        case ZCC_UINT:
            fprintf(stderr, "UINT: %x\n", val->ival.ui);

            break;
        case ZCC_LONG:
            fprintf(stderr, "LONG: %lx\n", val->ival.l);

            break;
        case ZCC_ULONG:
            fprintf(stderr, "ULONG: %lx\n", val->ival.ul);

            break;
        case ZCC_LONGLONG:
            fprintf(stderr, "LONGLONG: %llx\n", val->ival.ll);


        case ZCC_ULONGLONG:
            fprintf(stderr, "ULONGLONG: %llx\n", val->ival.ull);

            break;

        default:

            break;
    }
    fprintf(stderr, "SZ: %ld\n", val->sz);

    return;
}

static void
zppprinttoken(struct zpptoken *token)
{
    fprintf(stderr, "TYPE %s\n", toktypetab[token->type]);
    if (token->type == ZPP_TYPE_TOKEN) {
        fprintf(stderr, "PARM: %s\n", typetab[token->parm]);
    } else {
        fprintf(stderr, "PARM: %s\n", tokparmtab[token->parm]);
    }
    fprintf(stderr, "STR: %s\n", token->str);
    if (token->type == ZPP_VALUE_TOKEN) {
        fprintf(stderr, "VALUE\n");
        fprintf(stderr, "-----\n");
        zppprintval((void *)token->data);
    } else {
        fprintf(stderr, "DATA: %lx\n", (unsigned long)token->data);
    }
}

void
zppprintqueue(struct zpptokenq *queue)
{
    struct zpptoken *token = queue->head;

    while (token) {
        zppprinttoken(token);
        token = token->next;
    }
}

#endif /* ZCCPRINT */

#if 0
struct zcctoken *
zppprocif(struct zpptoken *token, struct zpptoken **nextret,
          struct zcctoken **tailret)
{
    struct zpptoken *head = NULL;
    struct zpptoken *tail = NULL;
    long             block = 0;
    long             skip = 0;
    long             loop = 1;

    token = token->next;
    skip = !zppeval(token, &token);
    while (loop) {
        while (token) {
            if (!skip) {
                zppqueuetoken(token, &head, &tail);
            }
            token = token->next;
            if (token->type == ZPP_ELSE_TOKEN) {
                if (skip) {
                    skip = !skip;
                    token = token->next;
                } else {
                    while (token->type != ZPP_ENDIF_TOKEN) {
                        token = token->next;
                    }
                    loop = 0;
                }
            } else if (token->type == ZPP_ELIF_TOKEN) {
                if (skip) {
                    skip = !zppeval(token, &token);
                } else {
                    while (token->type != ZPP_ENDIF_TOKEN) {
                        token = token->next;
                    }
                    loop = 0;
                }
            } else if (token->type == ZPP_ENDIF_TOKEN) {
                loop = 0;
            }
        }
    }
    if (head) {
        *nextret = token;
    }

    return head;
}
#endif
struct zcctoken *
zppprocif(struct zpptoken *token, struct zpptoken **nextret,
          struct zcctoken **tailret)
{
    return NULL;
}

struct zcctoken *
zppprocifdef(struct zpptoken *token, struct zpptoken **nextret,
             struct zcctoken **tailret)
{
    return NULL;
}

struct zcctoken *
zppprocifndef(struct zpptoken *token, struct zpptoken **nextret,
              struct zcctoken **tailret)
{
    return NULL;
}

struct zcctoken *
zppprocdefine(struct zpptoken *token, struct zpptoken **nextret,
              struct zcctoken **tailret)
{
    return NULL;
}

static struct zcctoken *
zppmktype(struct zpptoken *token, struct zpptoken **nextret,
          struct zcctoken **lastret)
{
    struct zcctoken *head = NULL;
    struct zcctoken *tail = NULL;
    struct zcctoken *last = NULL;
    struct zcctoken *tok = malloc(sizeof(struct zcctoken));;
    long             parm;
    long             def = 0;

    if (token->type == ZPP_TYPEDEF_TOKEN) {
        token = token->next;
        def = 1;
    }
    if (!token) {
        if (def) {
            fprintf(stderr, "invalid typedef\n");

            exit(1);
        }
    }
    if (token->type == ZPP_ATR_TOKEN) {
        token = token->next;
        while (token->type == ZPP_LEFT_PAREN_TOKEN) {
            token = token->next;
        }
        
    }
    tok->type = ZCC_NONE;
    tok->parm = ZCC_NONE;
    tok->str = NULL;
    tok->data = 0;
    tok->datasz = 0;
    if (token->type == ZPP_QUAL_TOKEN) {
        parm = token->parm;
        tok->flg |= qualflgtab[parm];
        token = token->next;
        while (token->type == ZPP_QUAL_TOKEN) {
            parm = token->parm;
            tok->flg |= qualflgtab[parm];
            token = token->next;
        }
    }
    if (token->type == ZPP_STRUCT_TOKEN) {
        tok->type = ZCC_STRUCT_TOKEN;
        token = token->next;
        if (token->type == ZCC_VAR_TOKEN) {
            tok->str = strdup(token->str);
            tok->data = (uintptr_t)zppgetstruct(token, &token, &tok->datasz);
        } else {
            fprintf(stderr, "invalid structure %s\n", token->str);
            free(tok);
            tok = NULL;
        }
    } else if (token->type == ZPP_UNION_TOKEN) {
        tok->type = ZCC_UNION_TOKEN;
        token = token->next;
        if (token->type == ZCC_VAR_TOKEN) {
            tok->str = strdup(token->str);
            tok->data = (uintptr_t)zppgetunion(token, &token, &tok->datasz);
        } else {
            fprintf(stderr, "invalid union %s\n", token->str);
            free(tok);
            tok = NULL;
        }
    } else if (token->type == ZPP_TYPE_TOKEN) {
        tok->type = ZCC_TYPE_TOKEN;
        parm = zccgettype(token);
        tok->parm = parm;
        parm = zccvarsz(parm);
        if (parm) {
            tok->datasz = parm;
            tok->str = strdup(token->str);
        } else {
            fprintf(stderr, "invalid type %s\n", token->str);
            free(tok);
            tok = NULL;
        }
    }
    if  (tok) {
        zccqueuetoken(tok, last, &head, &tail);
        *nextret = token;
        *lastret = NULL;
    }

    return tok;
}

struct zccstruct *
zppgetstruct(struct zpptoken *token, struct zpptoken **nextret, size_t *sizeret)
{
    struct zccstruct *newstruct = NULL;
    struct zcctoken  *tok;
    struct zcctoken  *last;
    long              parm;
    size_t            msz;
    size_t            sz = 0;
    size_t            nmemb = 8;
    size_t            n = 0;

    if (token->type == ZPP_BLOCK_TOKEN) {
        newstruct = malloc(sizeof(struct zccstruct));
        newstruct->mtab = malloc(nmemb * sizeof(struct zcctoken *));
        newstruct->ofstab = malloc(nmemb * sizeof(size_t));
        token = token->next;
        while ((token) && token->type != ZPP_END_BLOCK_TOKEN) {
            if (token->type == ZPP_TYPE_TOKEN) {
                tok = zppmktype(token, &token, &last);
                if (token->type == ZPP_VAR_TOKEN) {
                    if (n == nmemb) {
                        nmemb <<= 1;
                        newstruct->mtab = realloc(newstruct->mtab,
                                                  nmemb * sizeof(struct zcctoken *));
                        newstruct->ofstab = realloc(newstruct->ofstab,
                                                    nmemb * sizeof(size_t));
                    }
                    tok = malloc(sizeof(struct zcctoken));
                    if (tok) {
                        parm = tok->parm;
                        tok->type = ZCC_VAR_TOKEN;
//                        msz = zccvarsz(parm);
                        if (tok->flg & ZCC_PACKED) {
                            msz = zccvarsz(parm);
                        } else {
                            msz = max(zccvarsz(parm), zccmach.ialn);
                        }
                        tok->datasz = msz;
                        token = token->next;
                        if (token->type == ZPP_SEMICOLON_TOKEN) {
                            token = token->next;
                            newstruct->mtab[n] = tok;
                            newstruct->ofstab[n] = sz;
                            sz += msz;
                            n++;
                        } else {
                            fprintf(stderr, "unexpected token in struct: %s\n",
                                    token->str);
                            free(tok);
                        }
                    } else {
                        fprintf(stderr, "cannot allocate token\n");

                        exit(1);
                    }
                }
            }
        }
        if (token->type == ZPP_SEMICOLON_TOKEN) {
            token = token->next;
        }
    }
    if (newstruct) {
        newstruct->nmemb = n;
        *nextret = token;
        *sizeret = sz;
    }

    return newstruct;
}

struct zccunion *
zppgetunion(struct zpptoken *token, struct zpptoken **nextret, size_t *sizeret)
{
    return NULL;
}

static struct zcctoken *
zppgetblock(struct zpptoken *token, struct zpptoken **nextret,
            struct zcctoken **lastret)
{
    return NULL;
}

static struct zcctoken *
zppgetfunc(struct zpptoken *token, struct zpptoken **nextret,
           struct zcctoken **lastret)
{
    return NULL;
}

void                                 
zccqueuetoken(struct zcctoken *token, struct zcctoken *last,
              struct zcctoken **queue, struct zcctoken **tail)
{
    if (token) {
        token->next = NULL;
        if (!*queue) {
            token->prev = NULL;
            *queue = token;
            *tail = last;
        } else if (*tail) {
            token->prev = *tail;
            (*tail)->next = token;
            if (last) {
                last->next = NULL;
                *tail = last;
            } else {
                *tail = token;
            }
        } else {
            token->prev = *queue;
            (*queue)->next = token;
            if (last) {
                *tail = last;
            } else {
                *tail = token;
            }
        }
    }

    return;
}

struct zcctoken *
zpppreproc(struct zpptoken *token, struct zcctoken **tailret)
{
    struct zcctoken *head = NULL;
    struct zcctoken *tail = NULL;
    struct zcctoken *tok = NULL;
    struct zcctoken *last = NULL;
    long             parm;
    zpptokenfunc_t  *func;

    while (token) {
        if (token->type == ZPP_TYPEDEF_TOKEN
            || token->type == ZPP_TYPE_TOKEN) {
            tok = zppmktype(token, &token, &last);
            if (tok) {
                zccqueuetoken(tok, last, &head, &tail);
            }
            if (token->type == ZCC_FUNC_TOKEN) {
                tok = zppgetfunc(token, &token, &last);
            }
        } else if (token->type == ZPP_PREPROC_TOKEN) {
            parm = token->parm;
            func = dirfunctab[parm];
            if (func) {
                tok = func(token, &token, &last);
                if (!tok) {
                    fprintf(stderr, "invalid token %s\n", token->str);
                }
            }
        }
        if (tok) {
            zccqueuetoken(tok, last, &head, &tail);
        } else {
            fprintf(stderr, "unexpected token\n");
#if (ZPPDEBUG)
            zppprinttoken(token);
#endif

            exit(1);
        }
    }
    if (head) {
        *tailret = tail;
    }

    return head;
}

