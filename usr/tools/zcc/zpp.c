#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zcc/zcc.h>

extern long typesztab[32];

#if (ZCCPRINT)
static void zppprinttoken(struct zpptoken *token);

#endif

typedef struct zcctoken * zpptokenfunc_t(struct zpptoken *,
                                         struct zpptoken **);

static struct zcctoken *zppprocif(struct zpptoken *, struct zpptoken **);
static struct zcctoken *zppprocifdef(struct zpptoken *, struct zpptoken **);
static struct zcctoken *zppprocifndef(struct zpptoken *, struct zpptoken **);
static struct zcctoken *zppprocdefine(struct zpptoken *, struct zpptoken **);

#if (ZCCPRINT)
static char *toktypetab[256] =
{
    NULL,
    "ZPP_TYPE_TOKEN",
    "ZPP_TYPEDEF_TOKEN",
    "ZPP_VAR_TOKEN",
    "ZPP_CHAR_TOKEN",
    "ZPP_SHORT_TOKEN",
    "ZPP_INT_TOKEN",
    "ZPP_LONG_TOKEN",
    "ZPP_LONG_LONG_TOKEN",
#if (ZCC_C99_TYPES)
    "ZPP_INT8_TOKEN",
    "ZPP_INT16_TOKEN",
    "ZPP_INT32_TOKEN",
    "ZPP_INT64_TOKEN",
#endif
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
    "ZPP_VALUE_TOKEN",
    "ZPP_QUAL_TOKEN",
    "ZPP_ATR_TOKEN",
    "ZPP_FUNC_TOKEN",
    "ZPP_LABEL_TOKEN",
    "ZPP_ADR_TOKEN",
    "ZPP_MACRO_TOKEN",
    "ZPP_PREPROC_TOKEN",
    "ZPP_CONCAT_TOKEN",
    "ZPP_STRINGIFY_TOKEN",
    "ZPP_LATIN1_TOKEN",
    "ZPP_UTF8_TOKEN",
    "ZPP_UCS16_TOKEN",
    "ZPP_UCS32_TOKEN"
};
static char *tokparmtab[256] =
{
    "NONE",
    "ZCC_EXTERN_QUAL",
    "ZCC_STATIC_QUAL",
    "ZCC_CONST_QUAL",
    "ZCC_VOLATILE_QUAL",
    "ZCC_IF_DIR",
    "ZCC_ELIF_DIR",
    "ZCC_ELSE_DIR",
    "ZCC_ENDIF_DIR",
    "ZCC_IFDEF_DIR",
    "ZCC_IFNDEF_DIR",
    "ZCC_DEFINE_DIR"
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
    "ZCC_LDOUBLE"
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

static struct zcctoken *
zppprocif(struct zpptoken *token, struct zpptoken **nextret)
{
    return NULL;
}

static struct zcctoken *
zppprocifdef(struct zpptoken *token, struct zpptoken **nextret)
{
    return NULL;
}

static struct zcctoken *
zppprocifndef(struct zpptoken *token, struct zpptoken **nextret)
{
    return NULL;
}

static struct zcctoken *
zppprocdefine(struct zpptoken *token, struct zpptoken **nextret)
{
    return NULL;
}

struct zccstruct *
zppgetstruct(struct zpptoken *token, struct zpptoken **nextret, size_t *sizeret)
{
    struct zccstruct *newstruct = malloc(sizeof(struct zccstruct));
    struct zpptoken  *next;

    if (token->type == ZPP_BLOCK_TOKEN) {
        token = token->next;
        while ((token) && token->type != ZPP_END_BLOCK_TOKEN) {
            ;
        }
    }

    return newstruct;
}

struct zccsunion *
zppgetunion(struct zpptoken *token, struct zpptoken **nextret, size_t *sizeret)
{
    return NULL;
}

static struct zcctoken *
zppmktype(struct zpptoken *token, struct zpptoken **nextret)
{
    struct zcctoken *tok = malloc(sizeof(struct zcctoken));;
    long             parm;

    if (token->type == ZPP_ATR_TOKEN) {
        token = token->next;
        while (token->type == ZPP_LEFT_PAREN_TOKEN) {
            token = token->next;
        }
        /* TODO */
    }
    tok->type = ZCC_NONE;
    tok->parm = ZCC_NONE;
    tok->str = NULL;
    tok->data = NULL;
    tok->itemsz = 0;
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
        tok->data = zppgetstruct(token, &token, &tok->itemsz);
    } else if (token->type == ZPP_UNION_TOKEN) {
        tok->type = ZCC_UNION_TOKEN;
        token = token->next;
        tok->data = zppgetunion(token, &token, &tok->itemsz);
    } else if (token->type == ZPP_TYPE_TOKEN) {
        tok->type = ZCC_TYPE_TOKEN;
        parm = zccgettype(token);
        tok->parm = parm;
        parm = zccvalsz(parm);
        if (parm) {
            tok->itemsz = parm;
        }
    }
    if  (tok) {
        *nextret = token;
    }

    return tok;
}

void                                 
zppqueuetoken(struct zcctoken *token,
              struct zcctoken **queue, struct zcctoken **tail)
{
    if (token) {
        token->next = NULL;
        if (!*queue) {
            token->prev = NULL;
            *queue = token;
        } else if (*tail) {
            token->prev = *tail;
            (*tail)->next = token;
            *tail = token;
        } else {
            token->prev = *queue;
            (*queue)->next = token;
            *tail = token;
        }
    }

    return;
}

struct zcctoken *
zpppreproc(struct zpptoken *token)
{
    struct zcctoken *head = NULL;
    struct zcctoken *tail = NULL;
    struct zcctoken *tok;
    long             parm;
    zpptokenfunc_t  *func;

    while (token) {
        if (token->type == ZPP_TYPEDEF_TOKEN) {
            token = token->next;
            tok = zppmktype(token, &token);
            if (!tok) {
                fprintf(stderr, "invalid typedef\n");
                
                exit(1);
            }
            tok->str = strdup(token->str);
        } else if (token->type == ZPP_PREPROC_TOKEN) {
            parm = token->parm;
            func = dirfunctab[parm];
            if (func) {
                tok = func(token, &token);
            }
        } else if (token->type == ZPP_TYPE_TOKEN) {
            parm = token->parm;
            if (!parm) {
                tok->type = zccgettype(token);
            } else {
                tok->type = token->parm;
            }
        } else if (token->type == ZPP_FUNC_TOKEN) {
            token = token->next;
//            tok = zccgetfunc(token);
        } else if (token->type == ZPP_STRUCT_TOKEN) {
        } else if (token->type == ZPP_UNION_TOKEN) {
        }
        if (tok) {
            zppqueuetoken(tok, &head, &tail);
        }
    }

    return head;
}

#if (ZCCPRINT)

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
        printval(token->adr);
    } else {
        fprintf(stderr, "ADR: %p\n", token->adr);
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

